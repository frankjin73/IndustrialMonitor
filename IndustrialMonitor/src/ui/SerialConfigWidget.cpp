#include "SerialConfigWidget.h"
#include <QSerialPortInfo>
#include <QMessageBox>

SerialConfigWidget::SerialConfigWidget(QWidget *parent)
	:QWidget(parent)
	,ui(new Ui::SerialConfigWidgetClass())
	, isOpen(false)
{
	ui->setupUi(this);

	setupBaudRateOptions();
	setupDataBitsOptions();
	setupParityOptions();
	setupStopBitsOptions();

	refreshPorts();

	connect(ui->refreshButton, &QPushButton::clicked, this, &SerialConfigWidget::refreshPorts);
	connect(ui->toggleButton, &QPushButton::clicked, this, &SerialConfigWidget::togglePort);

}

SerialConfigWidget::~SerialConfigWidget() {
	delete ui;
}

QString SerialConfigWidget::currentPortName() const
{
	return ui->portComboBox->currentData().toString();
}

qint32 SerialConfigWidget::currentBaudRate() const
{
	return ui->baudRateComboBox->currentData().toInt();
}

qint32 SerialConfigWidget::currentDataBit() const
{
	return ui->dataBitsComboBox->currentData().toInt();
}

qint32 SerialConfigWidget::currentParity() const
{
	return ui->parityComboBox->currentData().toInt();
}
qint32 SerialConfigWidget::currentStopBits() const
{
	return ui->stopBitsComboBox->currentData().toInt();
}

void SerialConfigWidget::refreshPorts() {
	//记住当前选中项
	const QString currentPort = ui->portComboBox->currentData().toString();

	ui->portComboBox->clear();

	const auto ports = QSerialPortInfo::availablePorts();
	for (const QSerialPortInfo& info : ports) {
		QString label = info.portName();
		if (!info.description().isEmpty()) {
			label += " (" + info.description() + ")";
		}
		ui->portComboBox->addItem(label, info.portName());
	}
	ui->portComboBox->setMinimumWidth(90);
	ui->portComboBox->view()->setMinimumWidth(120);
	ui->portComboBox->view()->setTextElideMode(Qt::ElideNone);

	if (ports.isEmpty()) {
		ui->portComboBox->addItem("无可用串口", QString());
	}
	else {
		//恢复之前选中的端口
		int idx = ui->portComboBox->findData(currentPort);
		if (idx >= 0)
			ui->portComboBox->setCurrentIndex(idx);
	}
}

void SerialConfigWidget::togglePort() {
	if (isOpen) {
		qDebug() << "按下关闭按钮";
		emit closeRequested();
	}
	else {
		qDebug() << "按下打开按钮";
		const QString portName = currentPortName();
		if (portName.isEmpty()) {
			QMessageBox::warning(this, "提示", "请先选择一个串口。");
			return;
		}
		emit openRequested(portName, currentBaudRate(), currentDataBit(), currentParity(), currentStopBits());
	}
}

void SerialConfigWidget::portOpened() {
	ui->toggleButton->setText("关闭串口");
	setUiEnabled(false);
}

void SerialConfigWidget::portClosed() {
	ui->toggleButton->setText("打开串口");
	setUiEnabled(true);
}

void SerialConfigWidget::errorOccurred(const QString& message) {
	QMessageBox::critical(this, "错误",
		QString("无法打开 %1：%2")
		.arg(currentPortName(), message));
}

//初始化波特率
void SerialConfigWidget::setupBaudRateOptions() {

	QList<int> baudRates = { 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 };
	for (auto baud : baudRates) {
		ui->baudRateComboBox->addItem(QString::number(baud), baud);
	}

	ui->baudRateComboBox->setCurrentIndex(7);//默认115200
	ui->baudRateComboBox->view()->setTextElideMode(Qt::ElideNone);
}

//初始化数据位
void SerialConfigWidget::setupDataBitsOptions() {

	QList<int> dataBits = { 5,6,7,8 };
	for (auto bit : dataBits) {
		ui->dataBitsComboBox->addItem(QString::number(bit), bit);
	}
	ui->dataBitsComboBox->setCurrentIndex(3);//默认8
}

//
void SerialConfigWidget::setupParityOptions() {
	ui->parityComboBox->addItem("None", QSerialPort::NoParity);
	ui->parityComboBox->addItem("Even", QSerialPort::EvenParity);
	ui->parityComboBox->addItem("Odd", QSerialPort::OddParity);
}

//
void SerialConfigWidget::setupStopBitsOptions() {
	ui->stopBitsComboBox->addItem("1", QSerialPort::OneStop);
	ui->stopBitsComboBox->addItem("1.5", QSerialPort::OneAndHalfStop);
	ui->stopBitsComboBox->addItem("2", QSerialPort::TwoStop);
}

//
void SerialConfigWidget::setUiEnabled(bool open) {
	ui->portComboBox->setEnabled(open);
	ui->baudRateComboBox->setEnabled(open);
	ui->dataBitsComboBox->setEnabled(open);
	ui->parityComboBox->setEnabled(open);
	ui->stopBitsComboBox->setEnabled(open);
	ui->refreshButton->setEnabled(open);
}