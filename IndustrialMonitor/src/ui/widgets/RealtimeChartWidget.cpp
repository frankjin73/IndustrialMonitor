#include "RealtimeChartWidget.h"
RealtimeChartWidget::RealtimeChartWidget(QWidget* parent)
	:QWidget(parent)
	,m_chart(new QChart())
	,m_lineSeries(new QLineSeries())
	,m_chartView(new QChartView(m_chart,this)){
}

RealtimeChartWidget::~RealtimeChartWidget() {

}
