#ifndef SERIALPORTREADER_H
#define SERIALPORTREADER_H

#include "core/PacketParser.h"
#include "core/Types.h"

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

signals:
    void packetReceived(Slf::NodeData data);
    void connectionStateChanged(Slf::ConnectionState state);
    void errorOccurred(QString message);

private slots:
    void onReadyRead();
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort serial_;
    PacketParser parser_;
    QString portName_;
    QString lastError_;
};

} // namespace Slf

#endif
