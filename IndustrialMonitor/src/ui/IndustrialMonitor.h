#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_IndustrialMonitor.h"
#include <Qlabel>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class IndustrialMonitorClass; };
QT_END_NAMESPACE

class SerialConfigWidget;

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

private:
    Ui::IndustrialMonitorClass *ui;
    SerialConfigWidget *m_serialConfig;
    QLabel* m_statusLabel = nullptr;
};

