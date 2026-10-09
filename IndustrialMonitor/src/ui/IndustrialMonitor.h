#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_IndustrialMonitor.h"
#include <Qlabel>
#include <QList>

QT_BEGIN_NAMESPACE
namespace Ui { class IndustrialMonitorClass; };
QT_END_NAMESPACE

class IndustrialMonitor : public QMainWindow
{
    Q_OBJECT

public:
    IndustrialMonitor(QWidget *parent = nullptr);
    ~IndustrialMonitor();
    void initUI();

private:
    Ui::IndustrialMonitorClass *ui;
};

