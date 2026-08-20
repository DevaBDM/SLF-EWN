#pragma once

#include <QColor>
#include <QDateTime>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QtMath>
#include <cmath>

namespace Slf {

enum class NodeStatus {
    Unknown = 0,
    Safe,
    Warning,
    Critical
};

enum class EventType {
    Warning,
    Critical,
    EmergencyInterrupt
};

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Failed,
    Simulated
};

enum class Page {
    Dashboard,
    AlertLog,
    MapView,
    Settings
};

enum class TimeRange {
    Hours24,
    Days7,
    Days30
};

inline constexpr int kPacketSize = 12;
inline constexpr quint32 kBaudRate = 115200;

struct NodeData {
    quint16 deviceId = 0;
    quint16 moistureRaw = 0; // 0..10000 => 0.00% .. 100.00%
    qint16 tiltX = 0;        // milli-g
    qint16 tiltY = 0;
    qint16 tiltZ = 1000;
    quint8 hardwareStatus = 0;
    QDateTime timestamp;

    float moisturePercent() const { return moistureRaw / 100.0f; }
    float tiltXg() const { return tiltX / 1000.0f; }
    float tiltYg() const { return tiltY / 1000.0f; }
    float tiltZg() const { return tiltZ / 1000.0f; }

    float tiltXDeg() const
    {
        const float x = tiltXg();
        const float y = tiltYg();
        const float z = tiltZg();
        return std::atan2(x, std::sqrt(y * y + z * z)) * 180.0f / float(M_PI);
    }

    float tiltYDeg() const
    {
        const float x = tiltXg();
        const float y = tiltYg();
        const float z = tiltZg();
        return std::atan2(y, std::sqrt(x * x + z * z)) * 180.0f / float(M_PI);
    }

    float tiltZDeg() const
    {
        const float x = tiltXg();
        const float y = tiltYg();
        const float z = tiltZg();
        return std::atan2(std::sqrt(x * x + y * y), z) * 180.0f / float(M_PI);
    }

    float tiltMagnitudeDeg() const { return tiltZDeg(); }

    bool hardwareEmergency() const { return (hardwareStatus & 0x80) != 0; }
};

struct Sample {
    QDateTime timestamp;
    float moisture = 0.f;
    float tiltXDeg = 0.f;
    float tiltYDeg = 0.f;
    float tiltZDeg = 0.f;
    float tiltMagDeg = 0.f;
};

struct Thresholds {
    float moistureWarning = 38.0f;
    float moistureCritical = 58.0f;
    float tiltWarningDeg = 8.0f;
    float tiltCriticalDeg = 16.0f;
};

struct NodeProfile {
    quint16 deviceId = 0;
    QString zoneId;
    QString zoneLabel;
    QPointF mapPos{0.5, 0.5};
};

struct ZoneDef {
    QString id;
    QString name;
    QRectF bounds; // 0..1 canvas space
};

struct AlertRecord {
    qint64 id = 0;
    QDateTime timestamp;
    quint16 deviceId = 0;
    QString zone;
    EventType type = EventType::Warning;
    QString triggerValue;
    bool acknowledged = false;
};

inline QString deviceIdString(quint16 id)
{
    return QString("%1").arg(id, 4, 16, QChar('0')).toUpper();
}

inline QString eventTypeLabel(EventType type)
{
    switch (type) {
    case EventType::Warning: return QStringLiteral("Warning");
    case EventType::Critical: return QStringLiteral("Critical");
    case EventType::EmergencyInterrupt: return QStringLiteral("Emergency-Interrupt");
    }
    return {};
}

inline QString statusLabel(NodeStatus status)
{
    switch (status) {
    case NodeStatus::Safe: return QStringLiteral("SAFE");
    case NodeStatus::Warning: return QStringLiteral("WARNING");
    case NodeStatus::Critical: return QStringLiteral("CRITICAL");
    case NodeStatus::Unknown: return QStringLiteral("NO DATA");
    }
    return {};
}

inline QString relativeTime(const QDateTime &when, const QDateTime &now)
{
    if (!when.isValid())
        return QStringLiteral("—");
    const qint64 s = when.secsTo(now);
    if (s < 5)
        return QStringLiteral("just now");
    if (s < 60)
        return QStringLiteral("%1 s ago").arg(s);
    if (s < 3600)
        return QStringLiteral("%1 min ago").arg(s / 60);
    if (s < 86400)
        return QStringLiteral("%1 h ago").arg(s / 3600);
    return when.toString(QStringLiteral("MMM d  HH:mm"));
}

} // namespace Slf
