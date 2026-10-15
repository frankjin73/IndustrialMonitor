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

signals:
	void openRequested(const QString& portName, qint32 baudRate, qint8 dataBit, qint8 parity, qint8 stopBits);
	void closeRequested();
/*signals:
	//串口打开成功
	void portOpened(const QString &portName, qint32 baudRate);
	//串口关闭
	void portClosed();
	//出错
	void errorOccurred(const QString &message);
	//串口收到数据
	void dataReceived(const QByteArray &data);

private slots:
	void refreshPorts();
	void togglePort();
	void onReadyRead();*/

private slots: 
	void refreshPorts();
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
	bool isOpen = false;
};

