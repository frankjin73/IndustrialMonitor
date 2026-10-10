#include "SerialConfigWidget.h"

SerialConfigWidget::SerialConfigWidget(QWidget *parent)
	:QWidget(parent)
	,ui(new Ui::SerialConfigWidgetClass())
	,m_serialPort(new QSerialPort(this))
{
	ui->setupUi(this);

}

SerialConfigWidget::~SerialConfigWidget() {
	if (m_serialPort->isOpen()) {
		m_serialPort->close();
	}
	delete ui;
}

bool SerialConfigWidget::isOpen() const
{
	return m_serialPort->isOpen();
}

QString SerialConfigWidget::currentPortName() const
{
	return ui->portComboBox->currentData().toString();
}

qint32 SerialConfigWidget::currentBaudRate() const
{
	return ui->baudRateComboBox->currentData().toInt();
}

void SerialConfigWidget::refreshPorts() {

}

void SerialConfigWidget::togglePort() {

}


//初始化波特率
void SerialConfigWidget::setupBaudRateOptions() {

	QList<int> baudRates = { 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600 };
	for (auto baud : baudRates) {
		ui->baudRateComboBox->addItem(QString::number(baud), baud);
	}

	ui->baudRateComboBox->setCurrentIndex(7);//默认115200
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
	ui->portComboBox->setEnabled(!open);
	ui->baudRateComboBox->setEnabled(!open);
	ui->dataBitsComboBox->setEnabled(!open);
	ui->parityComboBox->setEnabled(!open);
	ui->stopBitsComboBox->setEnabled(!open);
	ui->refreshButton->setEnabled(!open);
}