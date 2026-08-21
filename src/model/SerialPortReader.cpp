#include "SerialPortReader.h"

#include <QSerialPortInfo>

namespace Slf {

namespace {
constexpr int kRawTailBytes = 48;

void applyLinkSettings(QSerialPort &serial)
{
    serial.setBaudRate(static_cast<qint32>(kBaudRate));
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
}
}

SerialPortReader::SerialPortReader(QObject *parent)
    : QObject(parent)
{
    applyLinkSettings(serial_);

    connect(&serial_, &QSerialPort::readyRead, this, &SerialPortReader::onReadyRead);
    connect(&serial_, &QSerialPort::errorOccurred, this, &SerialPortReader::onErrorOccurred);
}

SerialPortReader::~SerialPortReader()
{
    close();
}

bool SerialPortReader::open(const QString &portName)
{
    if (serial_.isOpen())
        serial_.close();

    portName_ = portName;

    bool matched = false;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts()) {
        if (info.portName().compare(portName, Qt::CaseInsensitive) == 0
            || info.systemLocation().compare(portName, Qt::CaseInsensitive) == 0) {
            serial_.setPort(info);
            matched = true;
            break;
        }
    }
    if (!matched)
        serial_.setPortName(portName);

    parser_.reset();
    resetStats();
    applyLinkSettings(serial_);

    emit connectionStateChanged(ConnectionState::Connecting);

    // ReadWrite keeps the DTR/RTS lines asserted, which many USB-UART bridges
    // and Arduino-style boards require before they will start transmitting.
    if (!serial_.open(QIODevice::ReadWrite)) {
        lastError_ = serial_.errorString();
        emit errorOccurred(lastError_);
        emit connectionStateChanged(ConnectionState::Failed);
        return false;
    }

    applyLinkSettings(serial_);
    serial_.setDataTerminalReady(true);
    serial_.setRequestToSend(true);
    serial_.clear(QSerialPort::AllDirections);

    lastError_.clear();
    emit connectionStateChanged(ConnectionState::Connected);
    return true;
}

void SerialPortReader::close()
{
    const bool wasOpen = serial_.isOpen();
    if (wasOpen)
        serial_.close();
    parser_.reset();
    if (wasOpen)
        emit connectionStateChanged(ConnectionState::Disconnected);
}

bool SerialPortReader::isOpen() const
{
    return serial_.isOpen();
}

QString SerialPortReader::rawTailHex() const
{
    if (rawTail_.isEmpty())
        return QString();
    return QString::fromLatin1(rawTail_.toHex(' ')).toUpper();
}

void SerialPortReader::resetStats()
{
    bytesReceived_ = 0;
    framesDecoded_ = 0;
    lastByteAt_ = QDateTime();
    lastFrameAt_ = QDateTime();
    rawTail_.clear();
    emit statsChanged();
}

void SerialPortReader::onReadyRead()
{
    const QByteArray chunk = serial_.readAll();
    if (chunk.isEmpty())
        return;

    bytesReceived_ += static_cast<quint64>(chunk.size());
    lastByteAt_ = QDateTime::currentDateTime();

    rawTail_.append(chunk);
    if (rawTail_.size() > kRawTailBytes)
        rawTail_ = rawTail_.right(kRawTailBytes);

    const auto packets = parser_.feed(chunk);
    if (!packets.isEmpty()) {
        framesDecoded_ += static_cast<quint64>(packets.size());
        lastFrameAt_ = QDateTime::currentDateTime();
    }
    for (const NodeData &p : packets)
        emit packetReceived(p);

    emit statsChanged();
}

void SerialPortReader::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;

    // A yanked cable surfaces as ResourceError. Report it so the dashboard stops
    // claiming the link is healthy while no packets are arriving.
    switch (error) {
    case QSerialPort::ResourceError:
    case QSerialPort::ReadError:
    case QSerialPort::DeviceNotFoundError:
    case QSerialPort::PermissionError:
    case QSerialPort::OpenError:
    case QSerialPort::TimeoutError:
        lastError_ = serial_.errorString();
        emit errorOccurred(lastError_);
        if (serial_.isOpen())
            serial_.close();
        emit connectionStateChanged(ConnectionState::Failed);
        break;
    default:
        break;
    }
}

} // namespace Slf
