#include "RealtimeChartWidget.h"
RealtimeChartWidget::RealtimeChartWidget(QWidget* parent)
	:QWidget(parent)
	,m_chart(new QChart())
	,m_lineSeries(new QLineSeries())
	,m_chartView(new QChartView(m_chart,this))
	,m_axisX(new QDateTimeAxis())
	,m_axisY(new QValueAxis){
	
	m_chart->addSeries(m_lineSeries);
	m_chart->addAxis(m_axisX, Qt::AlignBottom);
	m_chart->addAxis(m_axisY, Qt::AlignLeft);
	m_chart->setAnimationOptions(QChart::SeriesAnimations);

	m_axisX->setFormat("hh:mm:ss");
	m_axisX->setTitleText("时间");
	m_axisY->setTitleText("数据");
	
	QDateTime now = QDateTime::currentDateTime();
	m_axisX->setRange(now.addSecs(-10), now);//X 轴范围设为最近 10 秒。
	m_axisY->setRange(20,31);

	m_lineSeries->setName("实时数据曲线");
	m_lineSeries->setUseOpenGL(true);
	m_lineSeries->attachAxis(m_axisX);
	m_lineSeries->attachAxis(m_axisY);
}

RealtimeChartWidget::~RealtimeChartWidget() {

}
