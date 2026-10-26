#pragma once
#include <QWidget>
#include <QChart>
#include <QLineSeries>
#include <QChartView>
#include <QValueAxis>
#include <QDateTimeAxis >
#include <QDateTime>

class RealtimeChartWidget :
    public QWidget
{
    Q_OBJECT

public:
    RealtimeChartWidget(QWidget* parent);
    ~RealtimeChartWidget();

    QChart* m_chart;
    QChartView* m_chartView;
    QLineSeries* m_lineSeries;
    QDateTimeAxis* m_axisX;
    QValueAxis* m_axisY;
};

