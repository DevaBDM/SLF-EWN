#ifndef PACKETPARSER_H
#define PACKETPARSER_H

#include "Types.h"

#include <QByteArray>
#include <QVector>

namespace Slf {

class PacketParser
{
public:
    QVector<NodeData> feed(const QByteArray &chunk);
    void reset();

    static bool decode(const quint8 *frame, NodeData &out);
    static QByteArray encode(const NodeData &data);

private:
    QByteArray buffer_;
};

} // namespace Slf

#endif
