#pragma once

#include <QString>
#include <QMutex>
#include <QFile>
#include <QDate>

class Logger
{
public:
	// 定义日志级别（数值越大，级别越高）
	enum LogLevel {
		Debug = 0,
		Warning,
		Critical,
		Fatal,
		Info
	};
		
	Logger();
	~Logger();

	static Logger* instance();
	static void install();
	static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
	// 设置和获取当前日志级别
	void setLogLevel(LogLevel level);
	LogLevel logLevel() const;

private:
	void writeToFile(const QString& msg);
	void checkDateRollover();

	QMutex m_mutex;
	QFile* m_logFile = nullptr;
	QTextStream* m_stream = nullptr;
	QDate m_currentDate;
	QString m_logDirPath;            // 日志目录
	LogLevel m_logLevel = Debug; // 默认级别为 Debug
};