#include "NodeManager.h"

#include <algorithm>

namespace Slf {

static NodeProfile makeProfile(quint16 id, const QString &zoneId, const QString &label, QPointF pos)
{
    NodeProfile p;
    p.deviceId = id;
    p.zoneId = zoneId;
    p.zoneLabel = label;
    p.mapPos = pos;
    return p;
}

NodeManager::NodeManager(QObject *parent)
    : QObject(parent)
{
    ensureDefaults();
    loadSettings();
}

void NodeManager::ensureDefaults()
{
    zones_ = {
        {QStringLiteral("north"), QStringLiteral("North Slope"), QRectF(0.06, 0.08, 0.42, 0.36)},
        {QStringLiteral("south"), QStringLiteral("South Berm"),  QRectF(0.08, 0.52, 0.36, 0.38)},
        {QStringLiteral("east"),  QStringLiteral("East Face"),   QRectF(0.54, 0.10, 0.40, 0.40)},
        {QStringLiteral("west"),  QStringLiteral("West Cut"),    QRectF(0.50, 0.56, 0.28, 0.32)},
        {QStringLiteral("road"),  QStringLiteral("Access Road"), QRectF(0.80, 0.58, 0.16, 0.30)},
    };

    const QVector<NodeProfile> defaults = {
        makeProfile(0xA104, QStringLiteral("north"), QStringLiteral("North Slope — Mine Area A"), QPointF(0.18, 0.20)),
        makeProfile(0xA108, QStringLiteral("north"), QStringLiteral("North Slope — Mine Area A"), QPointF(0.34, 0.28)),
        makeProfile(0xB201, QStringLiteral("south"), QStringLiteral("South Berm — Drainage B"),   QPointF(0.16, 0.64)),
        makeProfile(0xB207, QStringLiteral("south"), QStringLiteral("South Berm — Drainage B"),   QPointF(0.30, 0.74)),
        makeProfile(0xC312, QStringLiteral("east"),  QStringLiteral("East Face — Tunnel Adit C"), QPointF(0.64, 0.22)),
        makeProfile(0xC318, QStringLiteral("east"),  QStringLiteral("East Face — Tunnel Adit C"), QPointF(0.78, 0.34)),
        makeProfile(0xD401, QStringLiteral("west"),  QStringLiteral("West Cut — Stockpile D"),    QPointF(0.58, 0.70)),
        makeProfile(0xE510, QStringLiteral("road"),  QStringLiteral("Access Road — Culvert E"),   QPointF(0.88, 0.72)),
    };

    for (const NodeProfile &p : defaults) {
        NodeState s;
        s.profile = p;
        nodes_.insert(p.deviceId, s);
    }
}

void NodeManager::ingest(const NodeData &data)
{
    const bool isNew = !nodes_.contains(data.deviceId);
    NodeState &n = nodes_[data.deviceId];
    if (isNew) {
        n.profile.deviceId = data.deviceId;
        if (n.profile.zoneLabel.isEmpty()) {
            n.profile.zoneLabel = QStringLiteral("Unassigned");
            n.profile.zoneId = QStringLiteral("none");
            n.profile.mapPos = QPointF(0.5, 0.5);
        }
    }

    const NodeStatus previous = n.status;
    n.latest = data;
    n.everSeen = true;

    Sample s;
    s.timestamp = data.timestamp;
    s.moisture = data.moisturePercent();
    s.tiltXDeg = data.tiltXDeg();
    s.tiltYDeg = data.tiltYDeg();
    s.tiltZDeg = data.tiltZDeg();
    s.tiltMagDeg = data.tiltMagnitudeDeg();
    n.history.append(s);
    trimHistory(n);

    const NodeStatus next = evaluate(data);
    const bool worse = static_cast<int>(next) > static_cast<int>(previous)
                       && next != NodeStatus::Unknown;
    if (next != previous) {
        n.status = next;
        if (worse)
            n.acknowledged = false;
        emit nodeStateChanged(data.deviceId, next);
    } else {
        n.status = next;
    }

    if (data.hardwareEmergency()) {
        n.status = NodeStatus::Critical;
        n.acknowledged = false;
        raiseAlert(n, EventType::EmergencyInterrupt,
                   QStringLiteral("HW INT 0x%1").arg(data.hardwareStatus, 2, 16, QChar('0')));
        emit emergencyTriggered(data.deviceId);
        emit nodeStateChanged(data.deviceId, NodeStatus::Critical);
    } else if (worse) {
        if (next == NodeStatus::Critical) {
            raiseAlert(n, EventType::Critical,
                       QStringLiteral("M %1%  /  T %2°")
                           .arg(data.moisturePercent(), 0, 'f', 1)
                           .arg(data.tiltMagnitudeDeg(), 0, 'f', 1));
        } else if (next == NodeStatus::Warning) {
            raiseAlert(n, EventType::Warning,
                       QStringLiteral("M %1%  /  T %2°")
                           .arg(data.moisturePercent(), 0, 'f', 1)
                           .arg(data.tiltMagnitudeDeg(), 0, 'f', 1));
        }
    }

    if (isNew)
        emit nodeAdded(data.deviceId);
    emit nodeUpdated(data.deviceId, data);
}

void NodeManager::seedHistory(quint16 id, const QVector<Sample> &samples)
{
    if (!nodes_.contains(id))
        return;
    NodeState &n = nodes_[id];
    n.history = samples;
    trimHistory(n);
    if (!samples.isEmpty() && !n.everSeen) {
        n.latest.deviceId = id;
        n.latest.moistureRaw = static_cast<quint16>(qBound(0, int(samples.last().moisture * 100), 10000));
        n.latest.timestamp = samples.last().timestamp;
        n.status = evaluate(n.latest);
    }
}

QList<quint16> NodeManager::nodeIds() const
{
    QList<quint16> ids = nodes_.keys();
    std::sort(ids.begin(), ids.end());
    return ids;
}

NodeState NodeManager::node(quint16 id) const
{
    return nodes_.value(id);
}

bool NodeManager::hasNode(quint16 id) const
{
    return nodes_.contains(id);
}

void NodeManager::setThresholds(const Thresholds &t)
{
    thresholds_ = t;
    saveSettings();
    emit thresholdsChanged(t);

    for (auto it = nodes_.begin(); it != nodes_.end(); ++it) {
        if (!it->everSeen)
            continue;
        const NodeStatus next = evaluate(it->latest);
        if (next != it->status) {
            it->status = next;
            emit nodeStateChanged(it.key(), next);
            emit nodeUpdated(it.key(), it->latest);
        }
    }
}

void NodeManager::setZoneLabel(quint16 id, const QString &label)
{
    if (!nodes_.contains(id))
        return;
    nodes_[id].profile.zoneLabel = label;
    saveSettings();
    emit profilesChanged();
}

void NodeManager::setMapPos(quint16 id, const QPointF &pos)
{
    if (!nodes_.contains(id))
        return;
    nodes_[id].profile.mapPos = pos;
    saveSettings();
    emit profilesChanged();
}

int NodeManager::unacknowledgedCount() const
{
    int n = 0;
    for (const AlertRecord &a : alerts_) {
        if (!a.acknowledged)
            ++n;
    }
    return n;
}

void NodeManager::acknowledgeAlert(qint64 alertId)
{
    for (AlertRecord &a : alerts_) {
        if (a.id == alertId) {
            a.acknowledged = true;
            if (nodes_.contains(a.deviceId)) {
                nodes_[a.deviceId].acknowledged = true;
                emit nodeUpdated(a.deviceId, nodes_[a.deviceId].latest);
            }
            emit alertsChanged();
            return;
        }
    }
}

void NodeManager::acknowledgeNode(quint16 id)
{
    if (!nodes_.contains(id))
        return;
    nodes_[id].acknowledged = true;
    for (AlertRecord &a : alerts_) {
        if (a.deviceId == id)
            a.acknowledged = true;
    }
    emit nodeUpdated(id, nodes_[id].latest);
    emit alertsChanged();
}

void NodeManager::clearAlerts()
{
    alerts_.clear();
    emit alertsChanged();
}

QVector<Sample> NodeManager::samplesInRange(quint16 id, TimeRange range) const
{
    const NodeState n = nodes_.value(id);
    const qint64 spanMs =
        range == TimeRange::Hours24 ? 24LL * 3600 * 1000
        : range == TimeRange::Days7  ? 7LL * 24 * 3600 * 1000
                                     : 30LL * 24 * 3600 * 1000;
    const QDateTime cutoff = QDateTime::currentDateTime().addMSecs(-spanMs);
    QVector<Sample> out;
    out.reserve(n.history.size());
    for (const Sample &s : n.history) {
        if (s.timestamp >= cutoff)
            out.append(s);
    }
    return out;
}

void NodeManager::loadSettings()
{
    QSettings st;
    thresholds_.moistureWarning = st.value(QStringLiteral("th/mWarn"), thresholds_.moistureWarning).toFloat();
    thresholds_.moistureCritical = st.value(QStringLiteral("th/mCrit"), thresholds_.moistureCritical).toFloat();
    thresholds_.tiltWarningDeg = st.value(QStringLiteral("th/tWarn"), thresholds_.tiltWarningDeg).toFloat();
    thresholds_.tiltCriticalDeg = st.value(QStringLiteral("th/tCrit"), thresholds_.tiltCriticalDeg).toFloat();

    st.beginGroup(QStringLiteral("nodes"));
    const QStringList keys = st.childGroups();
    for (const QString &key : keys) {
        bool ok = false;
        const quint16 id = key.toUShort(&ok, 16);
        if (!ok)
            continue;
        st.beginGroup(key);
        NodeState &n = nodes_[id];
        n.profile.deviceId = id;
        n.profile.zoneLabel = st.value(QStringLiteral("zone"), n.profile.zoneLabel).toString();
        n.profile.zoneId = st.value(QStringLiteral("zoneId"), n.profile.zoneId).toString();
        n.profile.mapPos = QPointF(st.value(QStringLiteral("x"), n.profile.mapPos.x()).toDouble(),
                                   st.value(QStringLiteral("y"), n.profile.mapPos.y()).toDouble());
        st.endGroup();
    }
    st.endGroup();
}

void NodeManager::saveSettings() const
{
    QSettings st;
    st.setValue(QStringLiteral("th/mWarn"), thresholds_.moistureWarning);
    st.setValue(QStringLiteral("th/mCrit"), thresholds_.moistureCritical);
    st.setValue(QStringLiteral("th/tWarn"), thresholds_.tiltWarningDeg);
    st.setValue(QStringLiteral("th/tCrit"), thresholds_.tiltCriticalDeg);

    st.beginGroup(QStringLiteral("nodes"));
    for (auto it = nodes_.constBegin(); it != nodes_.constEnd(); ++it) {
        st.beginGroup(deviceIdString(it.key()));
        st.setValue(QStringLiteral("zone"), it->profile.zoneLabel);
        st.setValue(QStringLiteral("zoneId"), it->profile.zoneId);
        st.setValue(QStringLiteral("x"), it->profile.mapPos.x());
        st.setValue(QStringLiteral("y"), it->profile.mapPos.y());
        st.endGroup();
    }
    st.endGroup();
}

NodeStatus NodeManager::evaluate(const NodeData &data) const
{
    const float m = data.moisturePercent();
    const float t = data.tiltMagnitudeDeg();
    if (m >= thresholds_.moistureCritical || t >= thresholds_.tiltCriticalDeg)
        return NodeStatus::Critical;
    if (m >= thresholds_.moistureWarning || t >= thresholds_.tiltWarningDeg)
        return NodeStatus::Warning;
    return NodeStatus::Safe;
}

void NodeManager::raiseAlert(const NodeState &n, EventType type, const QString &value)
{
    AlertRecord a;
    a.id = nextAlertId_++;
    a.timestamp = QDateTime::currentDateTime();
    a.deviceId = n.profile.deviceId;
    a.zone = n.profile.zoneLabel;
    a.type = type;
    a.triggerValue = value;
    a.acknowledged = false;
    alerts_.prepend(a);
    emit alertRaised(a);
}

void NodeManager::trimHistory(NodeState &n)
{
    if (n.history.size() > kMaxHistory)
        n.history.erase(n.history.begin(), n.history.begin() + (n.history.size() - kMaxHistory));
}

} // namespace Slf
