#include "IndustrialMonitor.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    IndustrialMonitor window;
    window.show();
    return app.exec();
}
