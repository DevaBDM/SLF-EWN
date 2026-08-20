#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include "core/Types.h"
#include "model/AlertListModel.h"
#include "model/NodeListModel.h"
#include "model/SerialPortReader.h"
#include "model/SimulatedTelemetrySource.h"
#include "controller/NodeManager.h"

#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

namespace Slf {

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int page READ page WRITE setPage NOTIFY pageChanged)
    Q_PROPERTY(int connectionState READ connectionState NOTIFY connectionChanged)
    Q_PROPERTY(QString portName READ portName WRITE setPortName NOTIFY connectionChanged)
    Q_PROPERTY(QStringList availablePorts READ availablePorts NOTIFY portsChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY connectionChanged)
    Q_PROPERTY(bool simulationEnabled READ simulationEnabled WRITE setSimulationEnabled NOTIFY simulationChanged)
    Q_PROPERTY(int nodeCount READ nodeCount NOTIFY countsChanged)
    Q_PROPERTY(int liveNodeCount READ liveNodeCount NOTIFY countsChanged)
    Q_PROPERTY(int alertCount READ alertCount NOTIFY countsChanged)
    Q_PROPERTY(QString clock READ clock NOTIFY clockChanged)
    Q_PROPERTY(QString uiFont READ uiFont CONSTANT)
    Q_PROPERTY(QString monoFont READ monoFont CONSTANT)
    Q_PROPERTY(NodeListModel *nodes READ nodes CONSTANT)
    Q_PROPERTY(AlertListModel *alerts READ alerts CONSTANT)
    Q_PROPERTY(int selectedId READ selectedId NOTIFY selectedChanged)
    Q_PROPERTY(bool detailOpen READ detailOpen NOTIFY selectedChanged)
    Q_PROPERTY(QVariantMap selectedNode READ selectedNode NOTIFY selectedChanged)
    Q_PROPERTY(QVariantList mapNodes READ mapNodes NOTIFY mapNodesChanged)
    Q_PROPERTY(QVariantList zones READ zones NOTIFY mapNodesChanged)
    Q_PROPERTY(QStringList zoneNames READ zoneNames NOTIFY mapNodesChanged)
    Q_PROPERTY(float moistureWarning READ moistureWarning WRITE setMoistureWarning NOTIFY thresholdsChanged)
    Q_PROPERTY(float moistureCritical READ moistureCritical WRITE setMoistureCritical NOTIFY thresholdsChanged)
    Q_PROPERTY(float tiltWarning READ tiltWarning WRITE setTiltWarning NOTIFY thresholdsChanged)
    Q_PROPERTY(float tiltCritical READ tiltCritical WRITE setTiltCritical NOTIFY thresholdsChanged)
    Q_PROPERTY(int baudRate READ baudRate CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);

    int page() const { return page_; }
    void setPage(int p);
    int connectionState() const { return static_cast<int>(connection_); }
    QString portName() const { return portName_; }
    void setPortName(const QString &p);
    QStringList availablePorts() const { return ports_; }
    QString lastError() const { return lastError_; }
    bool simulationEnabled() const { return simulationEnabled_; }
    void setSimulationEnabled(bool on);
    int nodeCount() const;
    int liveNodeCount() const { return nodes_ ? nodes_->rowCount() : 0; }
    int alertCount() const;
    QString clock() const { return clock_; }
    QString uiFont() const { return uiFont_; }
    QString monoFont() const { return monoFont_; }
    NodeListModel *nodes() const { return nodes_; }
    AlertListModel *alerts() const { return alerts_; }
    int selectedId() const { return selectedId_; }
    bool detailOpen() const { return detailOpen_; }
    QVariantMap selectedNode() const;
    QVariantList mapNodes() const;
    QVariantList zones() const;
    QStringList zoneNames() const;
    float moistureWarning() const;
    float moistureCritical() const;
    float tiltWarning() const;
    float tiltCritical() const;
    void setMoistureWarning(float v);
    void setMoistureCritical(float v);
    void setTiltWarning(float v);
    void setTiltCritical(float v);
    int baudRate() const { return static_cast<int>(kBaudRate); }

    Q_INVOKABLE void refreshPorts();
    Q_INVOKABLE void connectSerial();
    Q_INVOKABLE void disconnectSerial();
    Q_INVOKABLE void openDetail(int deviceId);
    Q_INVOKABLE void closeDetail();
    Q_INVOKABLE void acknowledgeSelected();
    Q_INVOKABLE void acknowledgeAlert(int alertId);
    Q_INVOKABLE void acknowledgeNode(int deviceId);
    Q_INVOKABLE void clearAlerts();
    Q_INVOKABLE bool exportAlerts(const QUrl &url);
    Q_INVOKABLE void setZoneLabel(int deviceId, const QString &label);
    Q_INVOKABLE QVariantList moistureSeries(int range) const;
    Q_INVOKABLE QVariantList tiltSeries(int range) const;
    Q_INVOKABLE QString statusLabel(int status) const;
    Q_INVOKABLE QString statusColor(int status) const;
    Q_INVOKABLE int probeStatus(float moisture, float tilt) const;

signals:
    void pageChanged();
    void connectionChanged();
    void portsChanged();
    void simulationChanged();
    void countsChanged();
    void clockChanged();
    void selectedChanged();
    void mapNodesChanged();
    void thresholdsChanged();
    void serialShake();
    void chartNeedsReload();

private:
    void applyConnectionState(ConnectionState s);
    void startSimulation();
    void stopSimulation();
    void seedHistory();
    TimeRange toRange(int range) const;
    QVariantList seriesOf(quint16 id, int range, int field) const;
    void detectFonts();

    NodeManager manager_;
    SerialPortReader serial_;
    SimulatedTelemetrySource sim_;
    NodeListModel *nodes_ = nullptr;
    AlertListModel *alerts_ = nullptr;
    QTimer clockTimer_;
    QTimer settleTimer_;

    int page_ = 0;
    ConnectionState connection_ = ConnectionState::Disconnected;
    QString portName_;
    QStringList ports_;
    QString lastError_;
    bool simulationEnabled_ = true;
    QString clock_;
    QString uiFont_;
    QString monoFont_;
    int selectedId_ = 0;
    bool detailOpen_ = false;
};

} // namespace Slf

#endif
