#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>

class TcpServerManager :
    public QObject
{
    Q_OBJECT

public:

    TcpServerManager(QObject* parent = nullptr);
    ~TcpServerManager();
    bool start(quint16 port);
    void stop();
    void broadcastMessage(const QJsonObject& data);

signals:
    void tcpStatusUpdate(const QString& message);
    void clientConnected(const QString& ip, quint16 port);
    void clientDisconnected(const QString& ip, quint16 port);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpServer* m_server;
    QList<QTcpSocket*> m_clients;
};

