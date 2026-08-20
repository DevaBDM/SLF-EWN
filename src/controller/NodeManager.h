#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include "core/Types.h"

#include <QHash>
#include <QObject>
#include <QSettings>
#include <QVector>

namespace Slf {

struct NodeState {
    NodeProfile profile;
    NodeData latest;
    NodeStatus status = NodeStatus::Unknown;
    bool acknowledged = false;
    bool everSeen = false;
    QVector<Sample> history;
};

class NodeManager : public QObject
{
    Q_OBJECT
public:
    explicit NodeManager(QObject *parent = nullptr);

    void ingest(const NodeData &data);
    void seedHistory(quint16 id, const QVector<Sample> &samples);

    QList<quint16> nodeIds() const;
    NodeState node(quint16 id) const;
    bool hasNode(quint16 id) const;

    Thresholds thresholds() const { return thresholds_; }
    void setThresholds(const Thresholds &t);

    void setZoneLabel(quint16 id, const QString &label);
    void setMapPos(quint16 id, const QPointF &pos);
    QVector<ZoneDef> zones() const { return zones_; }

    QVector<AlertRecord> alerts() const { return alerts_; }
    int unacknowledgedCount() const;
    int nodeCount() const { return nodes_.size(); }

    void acknowledgeAlert(qint64 alertId);
    void acknowledgeNode(quint16 id);
    void clearAlerts();

    QVector<Sample> samplesInRange(quint16 id, TimeRange range) const;

    void loadSettings();
    void saveSettings() const;

signals:
    void nodeAdded(quint16 id);
    void nodeUpdated(quint16 id, Slf::NodeData data);
    void nodeStateChanged(quint16 id, Slf::NodeStatus status);
    void emergencyTriggered(quint16 id);
    void alertRaised(Slf::AlertRecord alert);
    void alertsChanged();
    void thresholdsChanged(Slf::Thresholds t);
    void profilesChanged();

private:
    NodeStatus evaluate(const NodeData &data) const;
    void raiseAlert(const NodeState &n, EventType type, const QString &value);
    void ensureDefaults();
    void trimHistory(NodeState &n);

    QHash<quint16, NodeState> nodes_;
    QVector<ZoneDef> zones_;
    QVector<AlertRecord> alerts_;
    Thresholds thresholds_;
    qint64 nextAlertId_ = 1;
    static constexpr int kMaxHistory = 4000;
};

} // namespace Slf

#endif
