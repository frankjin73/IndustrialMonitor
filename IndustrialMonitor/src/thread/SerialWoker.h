#pragma once

#include <QObject>
#include <QSerialPort>
#include "DataParser.h"

class SerialWoker :
    public QObject
{
    Q_OBJECT

public:
    SerialWoker(QObject *parent);
    SerialWoker() = default;
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
    //void dataReceived(const QByteArray &date);
    //串口数据解析完成
    void dataParsed(const QList<SensorData>& dataList);
    
public slots:
    void openPort(const QString& portName, qint32 baudRate, qint8 dataBit, qint8 parity, qint8 stopBits);
    void closePort();
    void onReadyRead();

private:
    QSerialPort* m_serialPort;
    DataParser m_parser;
};

