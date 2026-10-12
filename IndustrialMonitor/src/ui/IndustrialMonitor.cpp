#include "IndustrialMonitor.h"
#include "SerialConfigWidget.h"

#include <QVBoxLayout>

IndustrialMonitor::IndustrialMonitor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IndustrialMonitorClass())
    , m_serialConfig(new SerialConfigWidget(this))
{
    ui->setupUi(this);

    this->resize(1200, 800);
    
    initUI();
    
    //连接信号与槽
    connect(m_serialConfig, &SerialConfigWidget::portOpened, this, &IndustrialMonitor::onPortOpened);
    connect(m_serialConfig, &SerialConfigWidget::portClosed, this, &IndustrialMonitor::onPortClosed);
    connect(m_serialConfig, &SerialConfigWidget::errorOccurred, this, &IndustrialMonitor::onSerialError);
    connect(m_serialConfig, &SerialConfigWidget::dataReceived, this, &IndustrialMonitor::onDataReceived);
}

IndustrialMonitor::~IndustrialMonitor()
{
    delete ui;
}

void IndustrialMonitor::initUI()
{
    //初始化串口设置模块
    auto layout = new QVBoxLayout(ui->serialConfigContainer);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_serialConfig);

    //初始化状态栏
    m_statusLabel = new QLabel("未连接", this);
    ui->statusBar->addWidget(m_statusLabel);
}

void IndustrialMonitor::onPortOpened(const QString& portName, qint32 baudRate) {
    m_statusLabel->setText("已连接");
}

void IndustrialMonitor::onPortClosed() {
    m_statusLabel->setText("未连接");
}

void IndustrialMonitor::onSerialError(const QString& message) {
    m_statusLabel->setText("错误：" + message);
}

void IndustrialMonitor::onDataReceived(const QByteArray& data) {
    // 把原始字节转成可读文本，追加到文本框
    const QString text = QString::fromLatin1(data);
    ui->rawDataTextEdit->moveCursor(QTextCursor::End);
    ui->rawDataTextEdit->insertPlainText(text);

    static qint64 totalBytes = 0;
    totalBytes += data.size();
    m_statusLabel->setText(QString("已连接 | 累计接收 %1 字节").arg(totalBytes));
}