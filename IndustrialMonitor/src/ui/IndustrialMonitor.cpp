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
    , m_plotTimer(new QTimer(this))
    , m_dbManager(new DatabaseManager())
    , m_repository(new DataRepository(m_dbManager->connectionName()))
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
    connect(m_serialWorker, &SerialWoker::dataParsed, this, &IndustrialMonitor::ondataParsed);

    //定时器每一秒钟重绘一次图像，防止卡顿
    m_plotTimer->setInterval(1000);
    m_plotTimer->setTimerType(Qt::CoarseTimer);
    connect(m_plotTimer, &QTimer::timeout, this, &IndustrialMonitor::onPlotTimeout);
    m_plotTimer->start();

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

    if (m_dbManager) {
        delete m_dbManager;
        m_dbManager = nullptr;
    }

    if (m_repository) {
        delete m_repository;
        m_repository = nullptr;
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

    //暂时不显示
    ui->dataTableWidget->hide();
    ui->rawDataGroupBox->hide();
}

void IndustrialMonitor::onPortOpened(const QString& portName, qint32 baudRate) {
    m_statusLabel->setText("已连接");
    m_realtimeChart->m_lineSeries->clear();
    m_plotTimer->start();
}

void IndustrialMonitor::onPortClosed() {
    m_statusLabel->setText("未连接");
    m_dataList.clear();
    m_plotTimer->stop();
}

void IndustrialMonitor::onSerialError(const QString& message) {
    m_statusLabel->setText("错误：" + message);
}

void IndustrialMonitor::ondataParsed(const QList<SensorData>& dataList) {
    static qint64 totalBytes = 0;

    m_dataList.append(dataList);

    for (const auto &data : dataList) {
        //存入sensor_data表
        m_repository->insertSensorData(data);

        // 温度，追加到文本框
        const QString text = QString::number(data.value)+"\n";
        ui->rawDataTextEdit->moveCursor(QTextCursor::End);
        ui->rawDataTextEdit->insertPlainText(text);

        totalBytes += 15;   //目前是固定帧长
        m_statusLabel->setText(QString("已连接 | 累计接收 %1 字节").arg(totalBytes));
    }
}

void IndustrialMonitor::onPlotTimeout() {

    m_realtimeChart->m_chartView->setUpdatesEnabled(false);

    QDateTime now = QDateTime::currentDateTime();
    QDateTime minTime = now.addSecs(-10);

    //最多显示近十秒收到的数据，多余部分删除
    int i = 0;
    while (i < m_dataList.size() && m_dataList[i].timestamp < minTime) {
        ++i;
    }
    if (i > 0) {
        m_dataList.erase(m_dataList.begin(), m_dataList.begin() + i);
    }

    QVector<QPointF> points;
    points.reserve(m_dataList.size());// 预分配，避免反复扩容

    for (const auto &data : m_dataList) {
        points.append(QPointF(static_cast<qreal>(data.timestamp.toMSecsSinceEpoch()), data.value));
    }
    m_realtimeChart->m_lineSeries->replace(points);
    m_realtimeChart->m_axisX->setRange(minTime, now);
    m_realtimeChart->m_chartView->setUpdatesEnabled(true);
}
