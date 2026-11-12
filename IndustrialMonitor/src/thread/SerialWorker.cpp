#include "SerialWorker.h"
#include <QSerialPortInfo>

SerialWorker::SerialWorker(QObject* parent)
: QObject(parent){
    
}

SerialWorker::~SerialWorker() {
    if (m_serialPort) {
        if (m_serialPort->isOpen()) {
            m_serialPort->close();
        }
    }
}

bool SerialWorker::isOpen() const
{
    return m_serialPort->isOpen();
}

void SerialWorker::openPort(const SerialConfig& cfg) {
    if (!m_serialPort) {
        m_serialPort = new QSerialPort(this);
    }

    if (m_serialPort->isOpen()) {
        return;
    }

    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialWorker::onReadyRead);

    m_serialPort->setPortName(cfg.portName);
    m_serialPort->setBaudRate(cfg.baudRate);
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(cfg.dataBit));
    m_serialPort->setParity(static_cast<QSerialPort::Parity>(
        cfg.parity));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(
        cfg.stopBits));
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        auto msg = m_serialPort->errorString();
        m_serialPort->close();
        qCritical() << "串口打开失败 " + m_serialPort->portName() + " " + msg;
        emit errorOccurred(m_serialPort->errorString());
        return;
    }
    qInfo() << "串口打开成功 " + m_serialPort->portName();
    emit portOpened(cfg.portName, cfg.baudRate);
    // 监听串口错误
    connect(m_serialPort, &QSerialPort::errorOccurred, this, &SerialWorker::handleSerialError);
}

void SerialWorker::closePort() {
    if (m_serialPort&&m_serialPort->isOpen()) {
        m_serialPort->close();
        emit portClosed();
    }

}

void SerialWorker::onReadyRead() {
    const QByteArray data = m_serialPort->readAll();
    if (!data.isEmpty()) {
        m_parser.m_buffer.append(data);
        emit dataParsed(m_parser.parse(data));
        //emit dataReceived(data);
    }
}

void SerialWorker::handleSerialError(QSerialPort::SerialPortError error)
{
    // 没有错误，直接返回
    if (error == QSerialPort::NoError) return;

    //ResourceError 通常代表硬件被拔出或设备丢失
    if (error == QSerialPort::ResourceError) {
        qCritical() << "串口异常断开:" << m_serialPort->errorString();

        //立刻关闭串口，释放资源
        if (m_serialPort->isOpen()) {
            m_serialPort->close();
        }

        //通过信号通知主线程（UI 层）
        emit portDisconnected("设备已拔出或连接丢失！");
    }
    else {
        // 其他错误（如校验错、帧错）只记录日志，不关闭串口
        qWarning() << "串口错误:" << m_serialPort->errorString();
    }
}