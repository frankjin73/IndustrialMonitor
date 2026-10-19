#pragma once
#include <QWidget>
#include <QChart>
#include <QLineSeries>
#include <QChartView>

class RealtimeChartWidget :
    public QWidget
{
    Q_OBJECT

public:
    RealtimeChartWidget(QWidget* parent);
    ~RealtimeChartWidget();
private:
    QChart* m_chart;
    QChartView* m_chartView;
    QLineSeries* m_lineSeries;
};

