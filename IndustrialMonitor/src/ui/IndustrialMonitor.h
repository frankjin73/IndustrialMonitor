#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_IndustrialMonitor.h"
#include <Qlabel>
#include <QList>
#include "SensorData.h"
#include "RealtimeChartWidget.h"
#include "DatabaseManager.h"
#include "DataRepository.h"
#include "AlarmManager.h"
#include "AlarmSettingDialog.h"
#include "ConfigManager.h"
#include "TcpServerManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class IndustrialMonitorClass; };
QT_END_NAMESPACE

class SerialConfigWidget;
class SerialWorker;

class IndustrialMonitor : public QMainWindow
{
    Q_OBJECT

public:
    IndustrialMonitor(QWidget *parent = nullptr);
    ~IndustrialMonitor();
    void initUI();
    void updateAlarmCountLabel();

private slots:
    void onPortOpened(const QString& portName, qint32 baudRate);
    void onPortClosed();
    void onSerialError(const QString &message);
    void ondataParsed(const QList<SensorData>& dataList);
    void onPlotTimeout();
    void onQueryHistoryData();
    void onExportToCsv();
    void onclearAlarmRecords();
    void showAlarmSettingDialog();
    void actionStartTcpTriggered();

private:
    Ui::IndustrialMonitorClass *ui;
    ConfigManager* m_configManager;
    SerialConfigWidget *m_serialConfig; //串口设置模块
    SerialWorker *m_serialWorker;   //串口数据解析
    QThread* m_serialThread;    //串口线程
    QLabel *m_statusLabel = nullptr;    //串口状态栏
    RealtimeChartWidget* m_realtimeChart;   //实时串口数据显示
    QList<SensorData> m_dataList;   //解析后的串口数据
    QTimer* m_plotTimer;    //定时刷新ui，减少卡顿
    DatabaseManager* m_dbManager;   //管理数据库
    DataRepository* m_repository;   //查询&插入表格
    AlarmManager* m_alarmManager;   //管理报警数据
    TcpServerManager* m_tcpManager; //管理tcp连接，负责广播数据
    quint16 m_tcpPort;  
    QLabel* m_tcpStatusLabel = nullptr; //tcp状态栏

};

