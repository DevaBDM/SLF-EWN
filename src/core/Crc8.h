#ifndef CRC8_H
#define CRC8_H

#include <QtGlobal>
#include <QByteArray>

namespace Slf {

// CRC-8/SMBUS — poly 0x07, init 0x00, xorout 0x00, no reflect.
// Computed over the first 12 bytes of the 13-byte telemetry frame.
class Crc8
{
public:
    static quint8 compute(const quint8 *data, int length);
    static quint8 compute(const QByteArray &bytes);
};

} // namespace Slf

#endif
