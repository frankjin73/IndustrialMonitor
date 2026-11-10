#pragma once

#include <QString>

class Logger
{
public:
	Logger();
	~Logger();
	static void install();
	static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

private:

	QString m_logDirPath;            // 日志目录
};