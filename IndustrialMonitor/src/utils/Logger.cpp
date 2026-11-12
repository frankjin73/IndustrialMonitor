#include "Logger.h"
#include <QCoreApplication>
#include <QDir>
#include <Windows.h>

Logger::Logger() {
	m_logDirPath = QCoreApplication::applicationDirPath() + "/logs";
	QDir().mkpath(m_logDirPath);
}

Logger::~Logger() {
	if (m_stream) {
		m_stream->flush();
		delete m_stream;
	}

	if (m_logFile) {
		m_logFile->close();
		delete m_logFile;
	}
}

Logger* Logger::instance() {
	static Logger inst;
	return &inst;
}

void Logger::install() {
	// 绑定全局的 qDebug 等日志
	qInstallMessageHandler(Logger::messageHandler);
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
	//级别过滤
	Logger::LogLevel currentLevel = Logger::instance()->logLevel();
	//如果当前消息级别低于设定的级别，直接返回（不打印也不写文件）
	if (type < currentLevel) {
		//如果是 Fatal 级别的错误，即使过滤了也必须让程序崩溃，防止死掉
		if (type == QtFatalMsg) abort();
		return;
	}

	QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");

	QString typeStr;

	switch (type)
	{
	case QtDebugMsg:
		typeStr = "Debug";
		break;
	case QtWarningMsg:
		typeStr = "Warn";
		break;
	case QtCriticalMsg:
		typeStr = "Error";
		break;
	case QtFatalMsg:
		typeStr = "Fatal";
		break;
	case QtInfoMsg:
		typeStr = "Info";
		break;
	default:
		break;
	}

	QString moduleName = "App";
	if (context.file) {
		QFileInfo fi(context.file);
		moduleName = fi.baseName(); // 例如：如果是 "src/thread/SerialWorker.cpp"，提取出 "SerialWorker"
	}

	//拼接最终的日志字符串
	QString finalLog = QString("[%1] [%2] [%3] : %4").arg(timeStr, typeStr, moduleName, msg);

	//输出到 VS 的“输出”窗口
	OutputDebugStringA(finalLog.toLocal8Bit().constData());
	OutputDebugStringA("\n");

	//输出到控制台
	fprintf(stderr, "%s\n", finalLog.toLocal8Bit().constData());

	Logger::instance()->writeToFile(finalLog);

	// 如果是 Fatal 错误，必须终止程序（Qt 默认行为）
	if (type == QtFatalMsg) {
		abort();
	}
}

void Logger::writeToFile(const QString& msg) {
	QMutexLocker locker(&m_mutex);
	checkDateRollover();

	if (m_stream) {
		*m_stream << msg << "\n";
		m_stream->flush();
	}
}

void Logger::checkDateRollover() {
	QDate today = QDate::currentDate();

	// 如果日期变了，或者第一次打开
	if (!m_logFile || m_currentDate != today) {
		if (m_stream) {
			m_stream->flush();
			delete m_stream;
			m_stream = nullptr;
		}
		if (m_logFile) {
			m_logFile->close();
			delete m_logFile;
			m_logFile = nullptr;
		}

		m_currentDate = today;
		QString dateStr = today.toString("yyyy_MM_dd"); // 例如 "2026_07_13"
		QString fileName = m_logDirPath + "/" + "app_" + dateStr;	 // 文件名：logs/app_2026_07_13.log

		m_logFile = new QFile(fileName);
		if (m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
			m_stream = new QTextStream(m_logFile);
		}
		else {
			delete m_logFile;
			m_logFile = nullptr;
		}
	}
}

void Logger::setLogLevel(LogLevel level) {
	instance()->m_logLevel = level;
}

Logger::LogLevel Logger::logLevel() const {
	return m_logLevel;
}