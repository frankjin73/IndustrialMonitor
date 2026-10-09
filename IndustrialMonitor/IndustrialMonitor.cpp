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
    //初始化波特率
    QList<int> baudRates = { 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 };
    for (auto baud : baudRates) {
        ui->baudRateComboBox->addItem(QString::number(baud), baud);
    }

    //初始化数据位
    QList<int> dataBits = { 5,6,7,8 };
    for (auto bit : dataBits) {
        ui->dataBitsComboBox->addItem(QString::number(bit), bit);
    }

    //初始化状态栏
    QLabel* statusLabel = new QLabel("未连接", this);
    ui->statusBar->addWidget(statusLabel);
}

