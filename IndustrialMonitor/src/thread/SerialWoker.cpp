#include "SerialWoker.h"
#include <QSerialPortInfo>

SerialWoker::SerialWoker(QObject* parent)
: QObject(parent){
    
}

SerialWoker::~SerialWoker() {
    if (m_serialPort) {
        if (m_serialPort->isOpen()) {
            m_serialPort->close();
        }
    }
}

bool SerialWoker::isOpen() const
{
    return m_serialPort->isOpen();
}

void SerialWoker::openPort(const QString& portName, qint32 baudRate, qint8 dataBit, qint8 parity, qint8 stopBits) {
    if (!m_serialPort) {
        m_serialPort = new QSerialPort(this);
    }

    if (m_serialPort->isOpen()) {
        return;
    }

    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialWoker::onReadyRead);

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBit));
    m_serialPort->setParity(static_cast<QSerialPort::Parity>(
        parity));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(
        stopBits));
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_serialPort->errorString());
        return;
    }

    emit portOpened(portName, baudRate);
}

void SerialWoker::closePort() {
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        emit portClosed();
    }

}

void SerialWoker::onReadyRead() {
    const QByteArray data = m_serialPort->readAll();
    if (!data.isEmpty()) {
        m_parser.m_buffer.append(data);
        emit dataParsed(m_parser.parse(data));
        //emit dataReceived(data);
    }
}
