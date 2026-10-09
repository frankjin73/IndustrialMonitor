#include "IndustrialMonitor.h"

IndustrialMonitor::IndustrialMonitor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IndustrialMonitorClass())
{
    ui->setupUi(this);
}

IndustrialMonitor::~IndustrialMonitor()
{
    delete ui;
}

