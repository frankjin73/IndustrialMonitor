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
    , m_alarmManager(new AlarmManager())
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

    //改变SerialConfigWidget的ui
    connect(m_serialWorker, &SerialWoker::portOpened, m_serialConfig, [&] {m_serialConfig->isOpen = m_serialWorker->isOpen(); m_serialConfig->portOpened(); });
    connect(m_serialWorker, &SerialWoker::portClosed, m_serialConfig, [&] {m_serialConfig->isOpen = m_serialWorker->isOpen(); m_serialConfig->portClosed(); });
    
    //历史数据查询
    connect(ui->queryButton, &QPushButton::clicked, this, &IndustrialMonitor::onQueryHistoryData);
    //历史数据导出
    connect(ui->exportButton, &QPushButton::clicked, this, &IndustrialMonitor::onExportToCsv);

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
    QDateTime now = QDateTime::currentDateTime();

    //历史数据查询范围默认从当前时间到一小时前
    ui->startTimeEdit->setDateTime(now.addSecs(-3600));
    ui->endTimeEdit->setDateTime(now);

    //历史数据表
    ui->historyTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //只读
    ui->historyTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);    //整行选中。
    ui->historyTableWidget->setAlternatingRowColors(true);  //隔行变色，好看。
    //时间列宽度随内容调整，其余列宽自适应
    auto* header = ui->historyTableWidget->horizontalHeader();
    ui->historyTableWidget->setColumnWidth(0, 120);
    //header->setSectionResizeMode(0, QHeaderView::ResizeToContents);  // 时间列
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::Stretch);
    header->setSectionResizeMode(4, QHeaderView::Stretch);
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
        
        //检测传感器状态
        auto result = m_alarmManager->check(data);
        switch (result) {
        case AlarmResult::Triggered:
                   // 插表格 + 入库
            break;
        case AlarmResult::Recovered:
                // 就是上面那个倒序扫
            break;
        case AlarmResult::Normal:
            break;
        }

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


void IndustrialMonitor::onQueryHistoryData() {
    auto startTime = ui->startTimeEdit->dateTime().toMSecsSinceEpoch();
    auto endTime = ui->endTimeEdit->dateTime().toMSecsSinceEpoch();

    if (startTime >= endTime) {
        qWarning() << "起始时间必须小于结束时间!";
        QMessageBox::warning(this, "警告", "起始时间必须小于结束时间!");
    }
    else {
        QList<SensorData> historyData = m_repository->querySensorData(startTime, endTime);
        ui->historyTableWidget->clearContents();    // 清空内容，保留表头

        if (historyData.isEmpty()) {
            ui->queryStatusLabel->setText("无数据");
            ui->historyTableWidget->setRowCount(0);
        }
        else {
            if (historyData.size() != 10000) {
                ui->queryStatusLabel->setText(QString("共查询到[%1]条记录").arg(historyData.size()));
            }
            else {
                ui->queryStatusLabel->setText("仅显示前 10000 条");
            }
            
            ui->historyTableWidget->setRowCount(historyData.size());
            ui->historyTableWidget->setUpdatesEnabled(false);
            ui->historyTableWidget->setSortingEnabled(false);

            for (int row = 0; row < historyData.size(); ++row) {
                const auto& data = historyData[row];

                //第0列：时间
                auto timeStr = data.timestamp.toString("yyyy-MM-dd HH:mm:ss");
                QTableWidgetItem* itemTime = new QTableWidgetItem(timeStr);
                itemTime->setTextAlignment(Qt::AlignCenter);
                ui->historyTableWidget->setItem(row, 0, itemTime);

                //第1列：设备ID
                QTableWidgetItem* itemDevice = new QTableWidgetItem(QString::number(data.deviceId));
                itemDevice->setTextAlignment(Qt::AlignCenter);
                ui->historyTableWidget->setItem(row, 1, itemDevice);

                //第2列：传感器ID
                QTableWidgetItem* itemSensor = new QTableWidgetItem(QString::number(data.sensorId));
                itemSensor->setTextAlignment(Qt::AlignCenter);
                ui->historyTableWidget->setItem(row, 2, itemSensor);

                //第3列：类型
                QTableWidgetItem* itemType = new QTableWidgetItem(data.typeName());
                itemType->setTextAlignment(Qt::AlignCenter);
                ui->historyTableWidget->setItem(row, 3, itemType);

                //第4列：数据
                QTableWidgetItem* itemValue = new QTableWidgetItem(QString::number(data.value, 'f', 2));
                itemValue->setTextAlignment(Qt::AlignCenter);
                ui->historyTableWidget->setItem(row, 4, itemValue);
            }

            ui->historyTableWidget->setSortingEnabled(true);
            ui->historyTableWidget->setUpdatesEnabled(true);
        }
    }

}

void IndustrialMonitor::onExportToCsv() {
    QString filePath = QFileDialog::getSaveFileName(this, "保存csv文件", "data.csv", "CSV Files (*.csv)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "错误", "无法创建文件！\n详细错误："+ file.errorString());
        return;
    }

    QTextStream out(&file);
    //解决 Excel 打开中文乱码
    out.setEncoding(QStringConverter::Utf8);
    out << "\xEF\xBB\xBF";

    //写表头
    out << "时间,设备ID,传感器ID,类型,数值\n";

    int rowCount = ui->historyTableWidget->rowCount();
    int colCount = ui->historyTableWidget->columnCount();

    for (int row = 0; row < rowCount; ++row) {
        QString temp = "";
        for (int col = 0; col < colCount-1; ++col) {
            QTableWidgetItem* item = ui->historyTableWidget->item(row, col);
            temp += item->text()+",";
        }
        out << temp + ui->historyTableWidget->item(row, colCount - 1)->text() + "\n";
    }
    file.close();
    QMessageBox::information(this, "成功", "数据导出完成！");
}