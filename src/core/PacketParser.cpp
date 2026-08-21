#include "PacketParser.h"
#include "Crc8.h"

#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QtMath>
#include <cmath>

namespace Slf {

namespace {

quint16 readU16be(const quint8 *p)
{
    return static_cast<quint16>((p[0] << 8) | p[1]);
}

qint16 readI16be(const quint8 *p)
{
    return static_cast<qint16>(readU16be(p));
}

void writeU16be(quint8 *p, quint16 v)
{
    p[0] = static_cast<quint8>(v >> 8);
    p[1] = static_cast<quint8>(v & 0xFF);
}

void writeI16be(quint8 *p, qint16 v)
{
    writeU16be(p, static_cast<quint16>(v));
}

qint16 degToMilliG(double deg)
{
    const double rad = deg * M_PI / 180.0;
    return static_cast<qint16>(qBound(-16000, int(std::tan(rad) * 1000.0), 16000));
}

} // namespace

bool PacketParser::decode(const quint8 *frame, NodeData &out)
{
    const quint8 crc = Crc8::compute(frame, kPacketSize - 1);
    if (crc != frame[kPacketSize - 1])
        return false;

    out.deviceId = readU16be(frame + 0);
    out.moistureRaw = readU16be(frame + 2);
    out.tiltX = readI16be(frame + 4);
    out.tiltY = readI16be(frame + 6);
    out.tiltZ = readI16be(frame + 8);
    out.hardwareStatus = frame[10];
    out.timestamp = QDateTime::currentDateTime();
    return true;
}

QByteArray PacketParser::encode(const NodeData &data)
{
    QByteArray frame(kPacketSize, '\0');
    auto *p = reinterpret_cast<quint8 *>(frame.data());
    writeU16be(p + 0, data.deviceId);
    writeU16be(p + 2, data.moistureRaw);
    writeI16be(p + 4, data.tiltX);
    writeI16be(p + 6, data.tiltY);
    writeI16be(p + 8, data.tiltZ);
    p[10] = data.hardwareStatus;
    p[11] = Crc8::compute(p, kPacketSize - 1);
    return frame;
}

bool PacketParser::looksLikeText(const QByteArray &bytes)
{
    if (bytes.isEmpty())
        return false;
    int printable = 0;
    for (char c : bytes) {
        const auto u = static_cast<unsigned char>(c);
        if (u == '\n' || u == '\r' || u == '\t' || (u >= 32 && u < 127))
            ++printable;
    }
    return printable * 100 / bytes.size() >= 85;
}

bool PacketParser::parseAsciiLine(const QByteArray &line, NodeData &out)
{
    const QString text = QString::fromLatin1(line).trimmed();
    if (text.isEmpty() || text.startsWith(QLatin1Char('#')))
        return false;

    const QStringList parts = text.split(QRegularExpression(QStringLiteral("[,;\\s]+")), Qt::SkipEmptyParts);
    if (parts.size() < 5)
        return false;

    bool idOk = false;
    QString idTok = parts.at(0);
    if (idTok.startsWith(QLatin1String("0x"), Qt::CaseInsensitive))
        idTok = idTok.mid(2);
    quint16 id = static_cast<quint16>(idTok.toUInt(&idOk, 16));
    if (!idOk || id == 0)
        return false;

    bool mOk = false, xOk = false, yOk = false, zOk = false;
    const double moisture = parts.at(1).toDouble(&mOk);
    const double tx = parts.at(2).toDouble(&xOk);
    const double ty = parts.at(3).toDouble(&yOk);
    const double tz = parts.at(4).toDouble(&zOk);
    if (!mOk || !xOk || !yOk || !zOk)
        return false;

    out.deviceId = id;
    if (moisture <= 100.0)
        out.moistureRaw = static_cast<quint16>(qBound(0, int(moisture * 100.0 + 0.5), 10000));
    else
        out.moistureRaw = static_cast<quint16>(qBound(0, int(moisture + 0.5), 10000));

    const bool degrees = std::fabs(tx) <= 90.0 && std::fabs(ty) <= 90.0 && std::fabs(tz) <= 90.0
                         && (parts.at(2).contains(QLatin1Char('.')) || parts.at(3).contains(QLatin1Char('.'))
                             || (std::fabs(tx) < 90.0 && std::fabs(tz) < 90.0 && std::fabs(tz) != 1000.0));
    if (degrees && std::fabs(tz) < 200.0) {
        out.tiltX = degToMilliG(tx);
        out.tiltY = degToMilliG(ty);
        out.tiltZ = degToMilliG(tz);
    } else {
        out.tiltX = static_cast<qint16>(qBound(-32000, int(tx), 32000));
        out.tiltY = static_cast<qint16>(qBound(-32000, int(ty), 32000));
        out.tiltZ = static_cast<qint16>(qBound(-32000, int(tz), 32000));
    }

    if (parts.size() >= 6) {
        bool sOk = false;
        const int st = parts.at(5).toInt(&sOk);
        out.hardwareStatus = sOk ? static_cast<quint8>(st) : 0;
    } else {
        out.hardwareStatus = 0;
    }
    out.timestamp = QDateTime::currentDateTime();
    return true;
}

QVector<NodeData> PacketParser::feed(const QByteArray &chunk)
{
    buffer_.append(chunk);
    QVector<NodeData> packets;

    if (looksLikeText(buffer_) && buffer_.contains('\n')) {
        int nl = 0;
        while ((nl = buffer_.indexOf('\n')) >= 0) {
            QByteArray line = buffer_.left(nl);
            buffer_.remove(0, nl + 1);
            if (line.endsWith('\r'))
                line.chop(1);
            NodeData data;
            if (parseAsciiLine(line, data))
                packets.append(data);
            else if (!line.trimmed().isEmpty())
                discarded_ += static_cast<quint64>(line.size());
        }
        return packets;
    }

    if (buffer_.size() > 8192) {
        discarded_ += static_cast<quint64>(buffer_.size() - 256);
        buffer_ = buffer_.right(256);
    }

    while (buffer_.size() >= kPacketSize) {
        const auto *raw = reinterpret_cast<const quint8 *>(buffer_.constData());
        NodeData data;
        if (decode(raw, data)) {
            packets.append(data);
            buffer_.remove(0, kPacketSize);
        } else {
            buffer_.remove(0, 1);
            ++discarded_;
        }
    }
    return packets;
}

void PacketParser::reset()
{
    buffer_.clear();
    discarded_ = 0;
}

} // namespace Slf
