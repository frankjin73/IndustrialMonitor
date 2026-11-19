#include "IndustrialMonitor.h"
#include <QtWidgets/QApplication>
#include "Logger.h"

// 读取 QSS
void loadStyleSheet(QApplication& app, const QString& qssPath)
{
    QFile file(qssPath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        // 读取所有内容，并设置为应用程序的全局样式表
        QString styleSheet = QLatin1String(file.readAll());
        app.setStyleSheet(styleSheet);
        file.close();
        qDebug() << "QSS 加载成功:" << qssPath;
    }
    else {
        qWarning() << "QSS 加载失败:" << qssPath;
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 安装日志拦截器
    Logger::install();

    // 设置日志级别
#ifdef QT_DEBUG
    // 如果是开发版（Debug），显示所有日志
    Logger::instance()->setLogLevel(Logger::Debug);
    loadStyleSheet(app, "resources/styles/app.qss"); // 直接读磁盘，改完保存立刻生效
#else
    // 如果是发布版（Release），只显示 警告 及以上级别
    Logger::instance()->setLogLevel(Logger::Warning);
    loadStyleSheet(app, ":/styles/app.qss");        // 发布时读资源
#endif

    qDebug() << "程序启动。";

    IndustrialMonitor window;
    window.show();
    return app.exec();
}
