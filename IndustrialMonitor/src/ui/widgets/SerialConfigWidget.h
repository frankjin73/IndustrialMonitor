#pragma once

#include <QtWidgets>
#include <QWidget>
#include <QSerialPort>
#include <ui_SerialConfigWidget.h>

QT_BEGIN_NAMESPACE
namespace Ui { class SerialConfigWidgetClass; }
QT_END_NAMESPACE

class SerialConfigWidget : public QWidget
{
	Q_OBJECT

public:
	SerialConfigWidget(QWidget *parent = nullptr);
	~SerialConfigWidget();

	QString currentPortName() const;
	qint32 currentBaudRate() const;
	qint32 currentDataBit() const;
	qint32 currentParity() const;
	qint32 currentStopBits() const;

	bool isOpen = false;

signals:
	void openRequested(const QString& portName, qint32 baudRate, qint8 dataBit, qint8 parity, qint8 stopBits);
	void closeRequested();

public slots: 
	//刷新串口
	void refreshPorts();
	//发送打开or关闭串口的请求信号
	void togglePort();
	//串口打开
	void portOpened();
	//串口关闭
	void portClosed();
	//串口打开时发生错误
	void errorOccurred(const QString& message);

private:
	void setupBaudRateOptions();
	void setupDataBitsOptions();
	void setupParityOptions();
	void setupStopBitsOptions();
	void setUiEnabled(bool open);

	Ui::SerialConfigWidgetClass* ui;

};

