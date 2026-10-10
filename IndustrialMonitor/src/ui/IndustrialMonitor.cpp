#include "IndustrialMonitor.h"

IndustrialMonitor::IndustrialMonitor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IndustrialMonitorClass())
{
    ui->setupUi(this);

    this->resize(1200, 800);
    
    initUI();

}

IndustrialMonitor::~IndustrialMonitor()
{
    delete ui;
}

void IndustrialMonitor::initUI()
{
    //初始化状态栏
    QLabel* statusLabel = new QLabel("未连接", this);
    ui->statusBar->addWidget(statusLabel);
}

