#include "Logger.h"
#include <QCoreApplication>
#include <QDir>

Logger::Logger() {
	m_logDirPath = QCoreApplication::applicationDirPath() + "/logs";
	QDir().mkpath(m_logDirPath);
}

Logger::~Logger() {

}

void Logger::install() {
	// 绑定全局的 qDebug 等日志
	qInstallMessageHandler(Logger::messageHandler);
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {

}