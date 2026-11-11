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
    SerialConfigWidget *m_serialConfig;
    SerialWorker *m_serialWorker;
    QThread* m_serialThread;
    QLabel *m_statusLabel = nullptr;
    RealtimeChartWidget* m_realtimeChart;
    QList<SensorData> m_dataList;
    QTimer* m_plotTimer;
    DatabaseManager* m_dbManager;
    DataRepository* m_repository;
    AlarmManager* m_alarmManager;
    TcpServerManager* m_tcpManager;
    quint16 m_tcpPort;
    QLabel* m_tcpStatusLabel = nullptr;

};

