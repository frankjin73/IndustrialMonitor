#include "SerialConfigWidget.h"
#include <QSerialPortInfo>
#include <QMessageBox>

SerialConfigWidget::SerialConfigWidget(QWidget *parent)
	:QWidget(parent)
	,ui(new Ui::SerialConfigWidgetClass())
	,m_serialPort(new QSerialPort(this))
{
	ui->setupUi(this);

	setupBaudRateOptions();
	setupDataBitsOptions();
	setupParityOptions();
	setupStopBitsOptions();

	refreshPorts();

    //连接信号与槽
    connect(ui->refreshButton, &QPushButton::clicked, this, &SerialConfigWidget::refreshPorts);
    connect(ui->toggleButton,&QPushButton::clicked, this,&SerialConfigWidget::togglePort);
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialConfigWidget::onReadyRead);
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
    //记住当前选中项
    const QString currentPort = ui->portComboBox->currentData().toString();

    ui->portComboBox->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
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
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        setUiEnabled(false);
        ui->toggleButton->setText("打开串口");
        emit portClosed();
        return;
    }

    const QString portName = currentPortName();
    if (portName.isEmpty()) {
        QMessageBox::warning(this, "提示", "请先选择一个串口。");
        return;
    }

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(currentBaudRate());
    m_serialPort->setDataBits(static_cast<QSerialPort::DataBits>(
        ui->dataBitsComboBox->currentData().toInt()));
    m_serialPort->setParity(static_cast<QSerialPort::Parity>(
        ui->parityComboBox->currentData().toInt()));
    m_serialPort->setStopBits(static_cast<QSerialPort::StopBits>(
        ui->stopBitsComboBox->currentData().toInt()));
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serialPort->open(QIODevice::ReadWrite)) {
        QMessageBox::critical(this, "错误",
            QString("无法打开 %1：%2")
            .arg(portName, m_serialPort->errorString()));
        emit errorOccurred(m_serialPort->errorString());
        return;
    }

    setUiEnabled(true);
    ui->toggleButton->setText("关闭串口");
    emit portOpened(portName, currentBaudRate());
}

void SerialConfigWidget::onReadyRead() {
    const QByteArray data = m_serialPort->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
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
	ui->portComboBox->setEnabled(!open);
	ui->baudRateComboBox->setEnabled(!open);
	ui->dataBitsComboBox->setEnabled(!open);
	ui->parityComboBox->setEnabled(!open);
	ui->stopBitsComboBox->setEnabled(!open);
	ui->refreshButton->setEnabled(!open);
}