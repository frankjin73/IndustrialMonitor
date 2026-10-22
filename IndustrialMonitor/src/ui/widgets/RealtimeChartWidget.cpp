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
	//X 轴范围设为 [最新时间 - 窗口时长, 最新时间]，比如最近 60 秒。
	QDateTime now = QDateTime::currentDateTime();
	QDateTime future = now.addSecs(10);
	m_axisX->setRange(now, future);
	//Y 轴范围根据当前窗口内数据的最小值 / 最大值，留一点余量（比如上下各扩 10 % ）。
	m_axisY->setRange(20,30);

	m_lineSeries->setName("实时数据曲线");
	m_lineSeries->attachAxis(m_axisX);
	m_lineSeries->attachAxis(m_axisY);
}

RealtimeChartWidget::~RealtimeChartWidget() {

}
