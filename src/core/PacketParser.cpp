#include "PacketParser.h"
#include "Crc8.h"

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

QVector<NodeData> PacketParser::feed(const QByteArray &chunk)
{
    buffer_.append(chunk);
    if (buffer_.size() > 8192)
        buffer_ = buffer_.right(256);

    QVector<NodeData> packets;
    while (buffer_.size() >= kPacketSize) {
        const auto *raw = reinterpret_cast<const quint8 *>(buffer_.constData());
        NodeData data;
        if (decode(raw, data)) {
            packets.append(data);
            buffer_.remove(0, kPacketSize);
        } else {
            buffer_.remove(0, 1);
        }
    }
    return packets;
}

void PacketParser::reset()
{
    buffer_.clear();
}

} // namespace Slf
