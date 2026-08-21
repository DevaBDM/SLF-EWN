#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H

#include "core/PacketParser.h"
#include "core/Types.h"

#include <QDateTime>
#include <QObject>
#include <QSerialPort>

namespace Slf {

class SerialPortReader : public QObject
{
    Q_OBJECT
public:
    explicit SerialPortReader(QObject *parent = nullptr);
    ~SerialPortReader() override;

    bool open(const QString &portName);
    void close();
    bool isOpen() const;
    QString portName() const { return portName_; }
    QString lastError() const { return lastError_; }

    // Link diagnostics. These separate "the cable is dead" from "the cable is
    // alive but the framing does not match", which is otherwise indistinguishable
    // from the dashboard.
    quint64 bytesReceived() const { return bytesReceived_; }
    quint64 framesDecoded() const { return framesDecoded_; }
    quint64 discardedBytes() const { return parser_.discardedBytes(); }
    QDateTime lastByteAt() const { return lastByteAt_; }
    QDateTime lastFrameAt() const { return lastFrameAt_; }
    QString rawTailHex() const;
    void resetStats();

signals:
    void packetReceived(Slf::NodeData data);
    void connectionStateChanged(Slf::ConnectionState state);
    void errorOccurred(QString message);
    void statsChanged();

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort serial_;
    PacketParser parser_;
    QString portName_;
    QString lastError_;

    quint64 bytesReceived_ = 0;
    quint64 framesDecoded_ = 0;
    QDateTime lastByteAt_;
    QDateTime lastFrameAt_;
    QByteArray rawTail_;
};

} // namespace Slf

#endif
