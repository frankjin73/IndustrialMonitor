#pragma once

#include <QObject>
#include <QSerialPort>

class SerialWoker :
    public QObject
{
    Q_OBJECT

public:
    SerialWoker(QObject *parent);
    ~SerialWoker();

    bool isOpen() const;

signals:
    //串口打开成功
    void portOpened(const QString &portName, qint32 baudRate);
    //串口关闭
    void portClosed();
    //出错
    void errorOccurred(const QString &message);
    //串口收到数据
    void dataReceived(const QByteArray &date);
    
private slots:
    void refreshPorts();
    void togglePort(const QString& portName, qint32 baudRate, qint8 dataBit, qint8 parity, qint8 stopBits);
    void onReadyRead();

private:
    QSerialPort* m_serialPort;
};

