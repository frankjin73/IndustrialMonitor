#include "IndustrialMonitor.h"
#include "SerialConfigWidget.h"
#include "SerialWorker.h"

#include <QVBoxLayout>

IndustrialMonitor::IndustrialMonitor(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::IndustrialMonitorClass())
    , m_configManager(new ConfigManager())
    , m_serialConfig(new SerialConfigWidget(this, m_configManager->loadSerialConfig()))
    , m_serialWorker(new SerialWorker())
    , m_serialThread(new QThread(this))
    , m_realtimeChart(new RealtimeChartWidget(ui->chartContainer))
    , m_plotTimer(new QTimer(this))
    , m_dbManager(new DatabaseManager())
    , m_repository(new DataRepository(m_dbManager->connectionName()))
    , m_alarmManager(new AlarmManager(this, m_configManager->loadThresholds()))
    , m_tcpManager(new TcpServerManager(this))
    , m_tcpPort(m_configManager->loadTcpPort())
{
    ui->setupUi(this);
    this->resize(1200, 800);
    initUI();

    m_serialWorker->moveToThread(m_serialThread);
    
    //关闭串口
    connect(m_serialConfig, &SerialConfigWidget::closeRequested, m_serialWorker, &SerialWorker::closePort);
    //打开串口
    connect(m_serialConfig, &SerialConfigWidget::openRequested, m_serialWorker, &SerialWorker::openPort);

    //设置statusLabe
    connect(m_serialWorker, &SerialWorker::portOpened, this, &IndustrialMonitor::onPortOpened);
    connect(m_serialWorker, &SerialWorker::portClosed, this, &IndustrialMonitor::onPortClosed);
    connect(m_serialWorker, &SerialWorker::errorOccurred, this, &IndustrialMonitor::onSerialError);
    connect(m_serialWorker, &SerialWorker::portDisconnected, this, [this](const QString& message) {
        m_statusLabel->setText(message);
        m_serialConfig->isOpen = m_serialWorker->isOpen();
        m_serialConfig->portClosed();
    });
    
    //处理收到的数据
    connect(m_serialWorker, &SerialWorker::dataParsed, this, &IndustrialMonitor::ondataParsed);

    //定时器每一秒钟重绘一次图像，防止卡顿
    m_plotTimer->setInterval(1000);
    m_plotTimer->setTimerType(Qt::CoarseTimer);
    connect(m_plotTimer, &QTimer::timeout, this, &IndustrialMonitor::onPlotTimeout);

    //改变SerialConfigWidget的ui
    connect(m_serialWorker, &SerialWorker::portOpened, m_serialConfig, [&] {m_serialConfig->isOpen = m_serialWorker->isOpen(); m_serialConfig->portOpened(); });
    connect(m_serialWorker, &SerialWorker::portClosed, m_serialConfig, [&] {m_serialConfig->isOpen = m_serialWorker->isOpen(); m_serialConfig->portClosed(); });
    
    
    //弹出传感器阈值设置对话框
    connect(ui->actionSettings,&QAction::triggered, this,&IndustrialMonitor::showAlarmSettingDialog);

    //设置TCP状态栏
    connect(m_tcpManager, &TcpServerManager::tcpStatusUpdate, this,[this](const QString&message){
        m_tcpStatusLabel->setText(message);
    });

    //启动远程监控
    connect(ui->actionStartTcp, &QAction::triggered, this, &IndustrialMonitor::actionStartTcpTriggered);

    //停止远程监控
    connect(ui->actionStopTcp, &QAction::triggered, this, [this] {
        m_tcpManager->stop();
        ui->actionStartTcp->setEnabled(true);
        ui->actionStopTcp->setEnabled(false);
    });

    //清空报警记录
    connect(ui->clearAlarmButton, &QPushButton::clicked, this, &IndustrialMonitor::onclearAlarmRecords);

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
    ui->statusBar->setStyleSheet("QStatusBar::item { border: none; }");

    m_statusLabel = new QLabel("未连接", this);
    m_statusLabel->setStyleSheet("border: none; padding: 0px 5px;"); // 明确无边框，加一点左右边距
    ui->statusBar->addWidget(m_statusLabel);

    QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::VLine);     // 垂直线
    line->setFrameShadow(QFrame::Plain);    
    line->setStyleSheet("color: #cccccc;"); // 设置线的颜色（浅灰色）
    line->setFixedHeight(14);               // 限制高度，上下留白
    ui->statusBar->addWidget(line);

    m_tcpStatusLabel = new QLabel("远程监控：未启动", this);
    m_tcpStatusLabel->setStyleSheet("border: none; padding: 0px 5px;");
    ui->statusBar->addWidget(m_tcpStatusLabel);

    ui->actionStartTcp->setEnabled(true);
    ui->actionStopTcp->setEnabled(false);

    //暂时不显示
    ui->dataTableWidget->hide();
    ui->rawDataGroupBox->hide();

    //报警记录表
    ui->alarmTableWidget->setColumnWidth(0, 150);
  //  ui->alarmTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->alarmTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //只读
    ui->alarmTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);    //整行选中。
    ui->alarmTableWidget->setAlternatingRowColors(true);  //隔行变色，好看。

    //历史数据查询范围默认从当前时间到一小时前
    QDateTime now = QDateTime::currentDateTime();
    ui->startTimeEdit->setDateTime(now.addSecs(-3600));
    ui->endTimeEdit->setDateTime(now);

    //历史数据表
    ui->historyTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //只读
    ui->historyTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);    //整行选中。
    ui->historyTableWidget->setAlternatingRowColors(true);  //隔行变色，好看。
    //时间列宽度随内容调整，其余列宽自适应
    auto* header = ui->historyTableWidget->horizontalHeader();
    ui->historyTableWidget->setColumnWidth(0, 150);
    //header->setSectionResizeMode(0, QHeaderView::ResizeToContents);  // 时间列
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::Stretch);
    header->setSectionResizeMode(4, QHeaderView::Stretch);
}

void IndustrialMonitor::onPortOpened(const QString& portName, qint32 baudRate) {
    //讲串口设置的参数存到ini文件
    m_configManager->saveSerialConfig(m_serialConfig->m_serialConfig);
    
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
        m_tcpManager->broadcastMessage(data.toJson());;
        //检测传感器状态
        auto result = m_alarmManager->check(data);
        switch (result) {
        case AlarmResult::Triggered:{
            // 报警信息存入数据库
            QString message = m_alarmManager->getAlarmMessage(data.sensorId) + QString::number(data.value, 'f', 2);
            auto record = AlarmRecord(data.timestamp, data.sensorId, message);
            m_repository->insertAlarmRecord(record);

            {   // 报警信息插表格
                ui->alarmTableWidget->insertRow(0);
                ui->alarmTableWidget->setUpdatesEnabled(false);
                ui->alarmTableWidget->setSortingEnabled(false);

                //第0列：时间
                auto timeStr = record.timestamp.toString("yyyy-MM-dd HH:mm:ss");
                QTableWidgetItem* itemTime = new QTableWidgetItem(timeStr);
                itemTime->setTextAlignment(Qt::AlignCenter);
                ui->alarmTableWidget->setItem(0, 0, itemTime);

                //第1列：传感器ID
                QTableWidgetItem* itemSensor = new QTableWidgetItem(QString::number(record.sensorId));
                itemSensor->setTextAlignment(Qt::AlignCenter);
                ui->alarmTableWidget->setItem(0, 1, itemSensor);

                //第2列：类型
                QTableWidgetItem* itemMessage = new QTableWidgetItem(record.message);
                itemMessage->setTextAlignment(Qt::AlignCenter);
                ui->alarmTableWidget->setItem(0, 2, itemMessage);

                int maxRows = 100;
                if (ui->alarmTableWidget->rowCount() > maxRows) {
                    ui->alarmTableWidget->removeRow(100); // 删掉第 100 行（最旧的数据）
                }

                ui->alarmTableWidget->setSortingEnabled(true);
                ui->alarmTableWidget->setUpdatesEnabled(true);
            }
            updateAlarmCountLabel();
            qWarning() << message;
            break;
        }
        case AlarmResult::Recovered:
            qInfo()<< "传感器" + QString::number(data.sensorId) +" 数值" + QString::number(data.value, 'f', 2);
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

void IndustrialMonitor::updateAlarmCountLabel() {
    ui->alarmStatusLabel->setText(QString("当前报警数: %1 条").arg(ui->alarmTableWidget->rowCount()));
}

void IndustrialMonitor::onclearAlarmRecords() {
    ui->alarmTableWidget->clearContents();    // 清空内容，保留表头
    ui->alarmTableWidget->setRowCount(0);

    updateAlarmCountLabel();
}

void IndustrialMonitor::showAlarmSettingDialog() {
    AlarmSettingDialog dlg(this, m_configManager->loadThresholds());

    switch (dlg.exec()) {
    case QDialog::Accepted:
        auto newThs = dlg.getThresholds();
        m_alarmManager->setThreshold(newThs);
        m_configManager->saveThresholds(newThs);
        break;
    }

}

void IndustrialMonitor::actionStartTcpTriggered() {
    int count = 3;
TRYAGAIN:
    bool result = m_tcpManager->start(m_tcpPort);
    if (result) {
        ui->actionStartTcp->setEnabled(false);
        ui->actionStopTcp->setEnabled(true);
        m_configManager->saveTcpPort(m_tcpPort);
    }
    else {
        bool ok;
        // 弹出输入框，默认值为 0
        int portValue = QInputDialog::getInt(this,
            "端口不可用",
            QString("端口 %1 被占用或不可用，请指定新端口（1-65535）：").arg(m_tcpPort),
            m_tcpPort, 1, 65535, 1, &ok);

        if (ok) {
            m_tcpPort = portValue;
            if (count != 0) {
                --count;
                goto TRYAGAIN;
            }
        }
        else {
            m_tcpStatusLabel->setText("远程监控：端口不可用");
        }

    }
}