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
	SerialConfigWidget(QWidget * arent = nullptr);
	~SerialConfigWidget();

	bool isOpen() const;
	QString currentPortName() const;
	qint32 currentBaudRate() const;

signals:
	//串口打开成功
	void portOpened(const QString &portName, qint32 baudRate);
	//串口关闭
	void portClosed();
	//出错
	void errorOccurred(const QString &message);

private slots:
	void refreshPorts();
	void togglePort();

private:
	void setupBaudRateOptions();
	void setupDataBitsOptions();
	void setupParityOptions();
	void setupStopBitsOptions();
	void setUiEnabled(bool open);

	Ui::SerialConfigWidgetClass* ui;
	QSerialPort* m_serialPort;

};

