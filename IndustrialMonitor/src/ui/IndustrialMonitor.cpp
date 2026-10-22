#include "IndustrialMonitor.h"
#include "SerialConfigWidget.h"
#include "SerialWoker.h"

#include <QVBoxLayout>

IndustrialMonitor::IndustrialMonitor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IndustrialMonitorClass())
    , m_serialConfig(new SerialConfigWidget(this))
    , m_serialWorker(new SerialWoker())
    , m_serialThread(new QThread(this))
    , m_realtimeChart(new RealtimeChartWidget(ui->chartContainer))
{
    ui->setupUi(this);
    this->resize(1200, 800);
    initUI();

    m_serialWorker->moveToThread(m_serialThread);
    
    //关闭串口
    connect(m_serialConfig, &SerialConfigWidget::closeRequested, m_serialWorker, &SerialWoker::closePort);
    //打开串口
    connect(m_serialConfig, &SerialConfigWidget::openRequested, m_serialWorker, &SerialWoker::openPort);

    //设置statusLabe
    connect(m_serialWorker, &SerialWoker::portOpened, this, &IndustrialMonitor::onPortOpened);
    connect(m_serialWorker, &SerialWoker::portClosed, this, &IndustrialMonitor::onPortClosed);
    connect(m_serialWorker, &SerialWoker::errorOccurred, this, &IndustrialMonitor::onSerialError);
    
    //处理收到的数据
    //connect(m_serialWorker, &SerialWoker::dataReceived, this, &IndustrialMonitor::onDataReceived);
    connect(m_serialWorker, &SerialWoker::dataParsed, this, &IndustrialMonitor::ondataParsed);


    //改变SerialConfigWidget的ui
    connect(m_serialWorker, &SerialWoker::portOpened, m_serialConfig, [&] {m_serialConfig->isOpen = m_serialWorker->isOpen(); m_serialConfig->portOpened(); });
    connect(m_serialWorker, &SerialWoker::portClosed, m_serialConfig, [&] {m_serialConfig->isOpen = m_serialWorker->isOpen(); m_serialConfig->portClosed(); });
    
    m_serialThread->start();
}

IndustrialMonitor::~IndustrialMonitor()
{
    if (m_serialThread->isRunning()) {
        // 核心：使用 BlockingQueuedConnection 阻塞调用子线程的 closePort 槽
        QMetaObject::invokeMethod(m_serialWorker, "closePort", Qt::BlockingQueuedConnection);

        m_serialThread->quit(); 
        m_serialThread->wait();
    }

    if (m_serialWorker) {
        delete m_serialWorker;
        m_serialWorker = nullptr;
    }

    if (m_serialConfig) {
        delete m_serialConfig;
        m_serialConfig = nullptr;
    }

    if (ui) {
        delete ui;
        ui = nullptr;
    }
}

void IndustrialMonitor::initUI()
{
    // 将图表对象设置到QVBoxLayout上进行显示
    QVBoxLayout* layout_chartView = new QVBoxLayout(ui->chartContainer);
    layout_chartView->addWidget(m_realtimeChart->m_chartView);
    layout_chartView->setContentsMargins(0, 0, 0, 0);
    ui->chartContainer->setLayout(layout_chartView);

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

void IndustrialMonitor::ondataParsed(const QList<SensorData>& dataList) {
    static qint64 totalBytes = 0;

    for (auto data : dataList) {
        // 温度，追加到文本框

        const QString text = QString::number(data.value)+"\n";
        ui->rawDataTextEdit->moveCursor(QTextCursor::End);
        ui->rawDataTextEdit->insertPlainText(text);

        totalBytes += 15;   //目前是固定帧长
        m_statusLabel->setText(QString("已连接 | 累计接收 %1 字节").arg(totalBytes));
    }
    
    
}