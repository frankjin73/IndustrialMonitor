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

QT_BEGIN_NAMESPACE
namespace Ui { class IndustrialMonitorClass; };
QT_END_NAMESPACE

class SerialConfigWidget;
class SerialWoker;

class IndustrialMonitor : public QMainWindow
{
    Q_OBJECT

public:
    IndustrialMonitor(QWidget *parent = nullptr);
    ~IndustrialMonitor();
    void initUI();

private slots:
    void onPortOpened(const QString& portName, qint32 baudRate);
    void onPortClosed();
    void onSerialError(const QString &message);
    void ondataParsed(const QList<SensorData>& dataList);
    void onPlotTimeout();
    void onQueryHistoryData();
    void onExportToCsv();

private:
    Ui::IndustrialMonitorClass *ui;
    SerialConfigWidget *m_serialConfig;
    SerialWoker *m_serialWorker;
    QThread* m_serialThread;
    QLabel *m_statusLabel = nullptr;
    RealtimeChartWidget* m_realtimeChart;
    QList<SensorData> m_dataList;
    QTimer* m_plotTimer;
    DatabaseManager* m_dbManager;
    DataRepository* m_repository;
    AlarmManager* m_alarmManager;
};

