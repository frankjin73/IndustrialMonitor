#include "TcpServerManager.h"

TcpServerManager::TcpServerManager(QObject* parent) 
	: QObject(parent)
{
	m_server = new QTcpServer(this);
	// 当有新客户端连接时，触发 onNewConnection
	connect(m_server, &QTcpServer::newConnection, this, &TcpServerManager::onNewConnection);
}

TcpServerManager::~TcpServerManager() {

}

bool TcpServerManager::start(quint16 port) {
	if (m_server->isListening()) m_server->close();

	if (m_server->listen(QHostAddress::Any, port)) {
		qInfo() << "TCP 启动成功 " + QString::number(port);
		emit tcpStatusUpdate(QString("远程监控：端口 <%1>，客户端 %2").arg(QString::number(port), QString::number(m_clients.size())));
		return true;
	}
	else {
		if (m_server->serverError() == QAbstractSocket::AddressInUseError) {
			qCritical() << "TCP 启动失败 端口[" + QString::number(port)+"]已被其他程序占用";
			emit tcpStatusUpdate(QString("端口 %1 已被其他程序占用，请换一个").arg(QString::number(port)));
		}
		else {
			qCritical() << "TCP 启动失败 端口[" + QString::number(port)+"] "+ m_server->errorString();
			emit tcpStatusUpdate(QString("端口 %1 无法监听：<%2>，请换一个").arg(QString::number(port), m_server->errorString()));
		}
		return false;
	}
}

void TcpServerManager::stop() {
	for (auto socket : m_clients) {
		socket->disconnectFromHost();

		socket->deleteLater();
	}
	m_clients.clear();

	if (m_server->isListening()) m_server->close();

	emit tcpStatusUpdate("远程监控：未启动");
}

//只要代码进入了 onNewConnection() ，就代表连接已经成功了
void TcpServerManager::onNewConnection() {
	while (m_server->hasPendingConnections()) {
		auto socket = m_server->nextPendingConnection();
		if (!socket) {
			qCritical() << m_server->errorString();
			continue;
		}
		m_clients.append(socket);

		// 为每个新连接的 Socket 绑定信号槽
		connect(socket, &QTcpSocket::readyRead, this, &TcpServerManager::onReadyRead);
		connect(socket, &QTcpSocket::disconnected, this, &TcpServerManager::onDisconnected);
		connect(socket, &QTcpSocket::errorOccurred, this, &TcpServerManager::onErrorOccurred);
		qInfo() << "TCP 客户端连接 " + socket->peerAddress().toString() + " "+ QString::number(socket->peerPort());
		emit clientConnected(socket->peerAddress().toString(), socket->peerPort());
	}
	emit tcpStatusUpdate(QString("远程监控：新客户端接入，当前在线数 %1").arg(QString::number(m_clients.size())));
}

//服务端广播数据
void TcpServerManager::broadcastMessage(const QJsonObject& data)
{
	if (m_clients.isEmpty()) {
		qDebug() << "当前没有客户端连接";
		return;
	}

	QByteArray byteArray = QJsonDocument(data).toJson(QJsonDocument::Compact);
	byteArray.append('\n');	//加上换行符，告诉客户端这是一条完整的数据

	for (QTcpSocket* client : m_clients) {
		// 检查连接状态，确保是已连接状态才发
		if (client->state() == QAbstractSocket::ConnectedState) {
			client->write(byteArray);
			client->flush(); // 强制立即发送缓冲区数据
		}
	}
}

void TcpServerManager::onReadyRead() {

}

void TcpServerManager::onDisconnected() {
	auto client = qobject_cast<QTcpSocket*>(sender());
	if (!client)return;
	
	qInfo() << "TCP 客户端断开 " + client->peerAddress().toString() + " " + QString::number(client->peerPort());
	emit clientDisconnected(client->peerAddress().toString(), client->peerPort());

	m_clients.removeOne(client);
	client->deleteLater();

	emit tcpStatusUpdate(QString("远程监控：客户端断开，当前在线数 %1").arg(QString::number(m_clients.size())));
}

void TcpServerManager::onErrorOccurred(QAbstractSocket::SocketError socketError) {

}