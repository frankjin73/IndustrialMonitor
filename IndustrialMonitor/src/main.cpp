#include "IndustrialMonitor.h"
#include <QtWidgets/QApplication>
#include "Logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 安装日志拦截器
    Logger::install();

    // 👈 设置日志级别
#ifdef QT_DEBUG
    // 如果是开发版（Debug），显示所有日志
    Logger::instance()->setLogLevel(Logger::Debug);
#else
    // 如果是发布版（Release），只显示 警告 及以上级别
    Logger::instance()->setLogLevel(Logger::Warning);
#endif

    qDebug() << "程序启动。";
    qInfo() << "这是一条普通信息。";
    qWarning() << "发生了一个警告！";

    IndustrialMonitor window;
    window.show();
    return app.exec();
}
