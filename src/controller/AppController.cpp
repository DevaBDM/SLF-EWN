#include "AppController.h"

#include "core/PacketParser.h"

#include <QFile>
#include <QFontDatabase>
#include <QSerialPortInfo>
#include <QSet>
#include <QSettings>
#include <QTextStream>
#include <QUrl>

namespace Slf {

AppController::AppController(QObject *parent)
    : QObject(parent)
{
    detectFonts();
    nodes_ = new NodeListModel(&manager_, this);
    alerts_ = new AlertListModel(&manager_, this);

    QSettings st;
    simulationEnabled_ = st.value(QStringLiteral("sim"), true).toBool();
    portName_ = st.value(QStringLiteral("port")).toString();

    refreshPorts();
    if (portName_.isEmpty() && !ports_.isEmpty())
        portName_ = ports_.first();

    connect(&sim_, &SimulatedTelemetrySource::packetReceived, &manager_, &NodeManager::ingest);
    connect(&serial_, &SerialPortReader::packetReceived, &manager_, &NodeManager::ingest);

    connect(&manager_, &NodeManager::nodeUpdated, this, [this](quint16 id, const NodeData &) {
        nodes_->handleNodeUpdated(id);
        emit countsChanged();
        emit mapNodesChanged();
        if (detailOpen_ && selectedId_ == int(id)) {
            emit selectedChanged();
            emit chartNeedsReload();
        }
    });
    connect(&manager_, &NodeManager::alertRaised, this, [this](const AlertRecord &a) {
        alerts_->handleAlertRaised(a);
        emit countsChanged();
    });
    connect(&manager_, &NodeManager::alertsChanged, this, [this]() {
        alerts_->reload();
        nodes_->refreshAll();
        emit countsChanged();
        if (detailOpen_)
            emit selectedChanged();
    });
    connect(&manager_, &NodeManager::profilesChanged, this, [this]() {
        nodes_->refreshAll();
        emit mapNodesChanged();
        if (detailOpen_)
            emit selectedChanged();
    });
    connect(&manager_, &NodeManager::thresholdsChanged, this, [this](const Thresholds &) {
        emit thresholdsChanged();
        nodes_->refreshAll();
        emit mapNodesChanged();
    });
    connect(&manager_, &NodeManager::emergencyTriggered, this, [this](quint16 id) {
        Q_UNUSED(id);
        emit countsChanged();
    });

    connect(&serial_, &SerialPortReader::errorOccurred, this, [this](const QString &msg) {
        lastError_ = msg;
        emit connectionChanged();
    });

    clockTimer_.setInterval(1000);
    connect(&clockTimer_, &QTimer::timeout, this, [this]() {
        const QDateTime now = QDateTime::currentDateTime();
        clock_ = now.toString(QStringLiteral("yyyy-MM-dd   HH:mm:ss"));
        nodes_->setNow(now);
        emit clockChanged();
    });
    clockTimer_.start();
    clock_ = QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd   HH:mm:ss"));

    seedHistory();
    if (simulationEnabled_)
        startSimulation();
    else
        applyConnectionState(ConnectionState::Disconnected);
}

void AppController::setPage(int p)
{
    if (page_ == p)
        return;
    page_ = qBound(0, p, 3);
    emit pageChanged();
}

void AppController::setPortName(const QString &p)
{
    if (portName_ == p)
        return;
    portName_ = p;
    QSettings().setValue(QStringLiteral("port"), p);
    emit connectionChanged();
}

void AppController::setSimulationEnabled(bool on)
{
    if (simulationEnabled_ == on)
        return;
    simulationEnabled_ = on;
    QSettings().setValue(QStringLiteral("sim"), on);
    if (on && connection_ != ConnectionState::Connected)
        startSimulation();
    else if (!on && connection_ == ConnectionState::Simulated)
        stopSimulation();
    emit simulationChanged();
}

int AppController::nodeCount() const
{
    return manager_.nodeCount();
}

int AppController::alertCount() const
{
    return manager_.unacknowledgedCount();
}

QVariantMap AppController::selectedNode() const
{
    QVariantMap m;
    if (!detailOpen_ || selectedId_ <= 0)
        return m;
    const NodeState n = manager_.node(static_cast<quint16>(selectedId_));
    m["deviceId"] = int(n.profile.deviceId);
    m["idString"] = deviceIdString(n.profile.deviceId);
    m["zone"] = n.profile.zoneLabel;
    m["status"] = int(n.status);
    m["acknowledged"] = n.acknowledged;
    m["moisture"] = n.latest.moisturePercent();
    m["tiltX"] = n.latest.tiltXDeg();
    m["tiltY"] = n.latest.tiltYDeg();
    m["tiltZ"] = n.latest.tiltZDeg();
    m["tiltMag"] = n.latest.tiltMagnitudeDeg();
    m["relativeTime"] = relativeTime(n.latest.timestamp, QDateTime::currentDateTime());
    m["everSeen"] = n.everSeen;
    return m;
}

QVariantList AppController::mapNodes() const
{
    QVariantList out;
    for (quint16 id : manager_.nodeIds()) {
        const NodeState n = manager_.node(id);
        QVariantMap m;
        m["deviceId"] = int(id);
        m["idString"] = deviceIdString(id);
        m["zone"] = n.profile.zoneLabel;
        m["status"] = int(n.everSeen ? n.status : NodeStatus::Unknown);
        m["acknowledged"] = n.acknowledged;
        m["x"] = n.profile.mapPos.x();
        m["y"] = n.profile.mapPos.y();
        m["everSeen"] = n.everSeen;
        out.append(m);
    }
    return out;
}

QVariantList AppController::zones() const
{
    QVariantList out;
    for (const ZoneDef &z : manager_.zones()) {
        QVariantMap m;
        m["id"] = z.id;
        m["name"] = z.name;
        m["x"] = z.bounds.x();
        m["y"] = z.bounds.y();
        m["w"] = z.bounds.width();
        m["h"] = z.bounds.height();
        out.append(m);
    }
    return out;
}

QStringList AppController::zoneNames() const
{
    QStringList names;
    names.append(QString());
    QSet<QString> seen;
    for (quint16 id : manager_.nodeIds()) {
        const QString z = manager_.node(id).profile.zoneLabel;
        if (!z.isEmpty() && !seen.contains(z)) {
            seen.insert(z);
            names.append(z);
        }
    }
    return names;
}

float AppController::moistureWarning() const { return manager_.thresholds().moistureWarning; }
float AppController::moistureCritical() const { return manager_.thresholds().moistureCritical; }
float AppController::tiltWarning() const { return manager_.thresholds().tiltWarningDeg; }
float AppController::tiltCritical() const { return manager_.thresholds().tiltCriticalDeg; }

void AppController::setMoistureWarning(float v)
{
    Thresholds t = manager_.thresholds();
    t.moistureWarning = v;
    manager_.setThresholds(t);
}
void AppController::setMoistureCritical(float v)
{
    Thresholds t = manager_.thresholds();
    t.moistureCritical = v;
    manager_.setThresholds(t);
}
void AppController::setTiltWarning(float v)
{
    Thresholds t = manager_.thresholds();
    t.tiltWarningDeg = v;
    manager_.setThresholds(t);
}
void AppController::setTiltCritical(float v)
{
    Thresholds t = manager_.thresholds();
    t.tiltCriticalDeg = v;
    manager_.setThresholds(t);
}

void AppController::refreshPorts()
{
    ports_.clear();
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        ports_.append(info.portName());
    emit portsChanged();
}

void AppController::connectSerial()
{
    if (portName_.isEmpty()) {
        lastError_ = QStringLiteral("No port selected");
        applyConnectionState(ConnectionState::Failed);
        emit serialShake();
        return;
    }

    applyConnectionState(ConnectionState::Connecting);
    settleTimer_.stop();
    settleTimer_.disconnect();
    connect(&settleTimer_, &QTimer::timeout, this, [this]() {
        settleTimer_.stop();
        stopSimulation();
        if (serial_.open(portName_)) {
            lastError_.clear();
            applyConnectionState(ConnectionState::Connected);
        } else {
            lastError_ = serial_.lastError();
            applyConnectionState(ConnectionState::Failed);
            emit serialShake();
            if (simulationEnabled_)
                startSimulation();
        }
    });
    settleTimer_.start(600);
}

void AppController::disconnectSerial()
{
    serial_.close();
    if (simulationEnabled_)
        startSimulation();
    else
        applyConnectionState(ConnectionState::Disconnected);
}

void AppController::openDetail(int deviceId)
{
    selectedId_ = deviceId;
    detailOpen_ = true;
    emit selectedChanged();
    emit chartNeedsReload();
}

void AppController::closeDetail()
{
    detailOpen_ = false;
    emit selectedChanged();
}

void AppController::acknowledgeSelected()
{
    if (selectedId_ > 0)
        manager_.acknowledgeNode(static_cast<quint16>(selectedId_));
}

void AppController::acknowledgeAlert(int alertId)
{
    manager_.acknowledgeAlert(alertId);
}

void AppController::acknowledgeNode(int deviceId)
{
    manager_.acknowledgeNode(static_cast<quint16>(deviceId));
}

void AppController::clearAlerts()
{
    manager_.clearAlerts();
}

bool AppController::exportAlerts(const QUrl &url)
{
    const QString path = url.isLocalFile() ? url.toLocalFile() : url.toString();
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream ts(&f);
    ts << "Timestamp,DeviceID,Zone,EventType,Value,Acknowledged\n";
    for (const AlertRecord &a : manager_.alerts()) {
        QString zone = a.zone;
        zone.replace(QLatin1Char('"'), QStringLiteral("''"));
        ts << a.timestamp.toString(Qt::ISODate) << ','
           << deviceIdString(a.deviceId) << ','
           << '"' << zone << '"' << ','
           << eventTypeLabel(a.type) << ','
           << '"' << a.triggerValue << '"' << ','
           << (a.acknowledged ? "yes" : "no") << '\n';
    }
    return true;
}

void AppController::setZoneLabel(int deviceId, const QString &label)
{
    manager_.setZoneLabel(static_cast<quint16>(deviceId), label);
}

QVariantList AppController::moistureSeries(int range) const
{
    return seriesOf(static_cast<quint16>(selectedId_), range, 0);
}

QVariantList AppController::tiltSeries(int range) const
{
    return seriesOf(static_cast<quint16>(selectedId_), range, 1);
}

QString AppController::statusLabel(int status) const
{
    return Slf::statusLabel(static_cast<NodeStatus>(status));
}

QString AppController::statusColor(int status) const
{
    switch (static_cast<NodeStatus>(status)) {
    case NodeStatus::Safe: return QStringLiteral("#3DDC97");
    case NodeStatus::Warning: return QStringLiteral("#F0A63A");
    case NodeStatus::Critical: return QStringLiteral("#E4483C");
    default: return QStringLiteral("#5C6573");
    }
}

int AppController::probeStatus(float moisture, float tilt) const
{
    const Thresholds t = manager_.thresholds();
    if (moisture >= t.moistureCritical || tilt >= t.tiltCriticalDeg)
        return int(NodeStatus::Critical);
    if (moisture >= t.moistureWarning || tilt >= t.tiltWarningDeg)
        return int(NodeStatus::Warning);
    return int(NodeStatus::Safe);
}

void AppController::applyConnectionState(ConnectionState s)
{
    connection_ = s;
    emit connectionChanged();
}

void AppController::startSimulation()
{
    sim_.start();
    applyConnectionState(ConnectionState::Simulated);
}

void AppController::stopSimulation()
{
    sim_.stop();
}

void AppController::seedHistory()
{
    const QDateTime now = QDateTime::currentDateTime();
    for (quint16 id : manager_.nodeIds())
        manager_.seedHistory(id, sim_.makeHistory(id, now, 60 * 24));
}

TimeRange AppController::toRange(int range) const
{
    if (range == 1)
        return TimeRange::Days7;
    if (range == 2)
        return TimeRange::Days30;
    return TimeRange::Hours24;
}

QVariantList AppController::seriesOf(quint16 id, int range, int field) const
{
    QVariantList out;
    if (id == 0)
        return out;
    const QVector<Sample> samples = manager_.samplesInRange(id, toRange(range));
    const int step = qMax(1, samples.size() / 700);
    out.reserve(samples.size() / step + 4);
    for (int i = 0; i < samples.size(); i += step) {
        const Sample &s = samples.at(i);
        QVariantMap p;
        p["t"] = s.timestamp.toMSecsSinceEpoch();
        if (field == 0) {
            p["v"] = s.moisture;
        } else {
            p["x"] = s.tiltXDeg;
            p["y"] = s.tiltYDeg;
            p["z"] = s.tiltZDeg;
            p["m"] = s.tiltMagDeg;
        }
        out.append(p);
    }
    return out;
}

void AppController::detectFonts()
{
    const QStringList families = QFontDatabase::families();
    auto pick = [&](const QStringList &wanted, const QString &fallback) {
        for (const QString &w : wanted) {
            for (const QString &s : families) {
                if (s.compare(w, Qt::CaseInsensitive) == 0 || s.startsWith(w, Qt::CaseInsensitive))
                    return s;
            }
        }
        return fallback;
    };
    uiFont_ = pick({QStringLiteral("IBM Plex Sans"), QStringLiteral("Inter"), QStringLiteral("Segoe UI")},
                   QStringLiteral("Segoe UI"));
    monoFont_ = pick({QStringLiteral("IBM Plex Mono"), QStringLiteral("JetBrains Mono"),
                      QStringLiteral("Cascadia Mono"), QStringLiteral("Consolas")},
                     QStringLiteral("Consolas"));
}

} // namespace Slf
