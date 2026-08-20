#include "SerialPortReader.h"

namespace Slf {

SerialPortReader::SerialPortReader(QObject *parent)
    : QObject(parent)
{
    serial_.setBaudRate(static_cast<qint32>(kBaudRate));
    serial_.setDataBits(QSerialPort::Data8);
    serial_.setParity(QSerialPort::NoParity);
    serial_.setStopBits(QSerialPort::OneStop);
    serial_.setFlowControl(QSerialPort::NoFlowControl);

    connect(&serial_, &QSerialPort::readyRead, this, &SerialPortReader::onReadyRead);
    connect(&serial_, &QSerialPort::errorOccurred, this, &SerialPortReader::onErrorOccurred);
}

SerialPortReader::~SerialPortReader()
{
    close();
}

bool SerialPortReader::open(const QString &portName)
{
    close();
    portName_ = portName;
    serial_.setPortName(portName);
    parser_.reset();

    emit connectionStateChanged(ConnectionState::Connecting);

    if (!serial_.open(QIODevice::ReadOnly)) {
        lastError_ = serial_.errorString();
        emit errorOccurred(lastError_);
        emit connectionStateChanged(ConnectionState::Failed);
        return false;
    }

    lastError_.clear();
    emit connectionStateChanged(ConnectionState::Connected);
    return true;
}

void SerialPortReader::close()
{
    if (serial_.isOpen())
        serial_.close();
    parser_.reset();
    emit connectionStateChanged(ConnectionState::Disconnected);
}

bool SerialPortReader::isOpen() const
{
    return serial_.isOpen();
}

void SerialPortReader::onReadyRead()
{
    const QByteArray chunk = serial_.readAll();
    const auto packets = parser_.feed(chunk);
    for (const NodeData &p : packets)
        emit packetReceived(p);
}

void SerialPortReader::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;
    if (error == QSerialPort::ResourceError || error == QSerialPort::ReadError) {
        lastError_ = serial_.errorString();
        emit errorOccurred(lastError_);
        if (serial_.isOpen())
            serial_.close();
        emit connectionStateChanged(ConnectionState::Failed);
    }
}

} // namespace Slf
