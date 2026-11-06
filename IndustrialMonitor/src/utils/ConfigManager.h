#pragma once

#include <QString>
#include <QCoreApplication>
#include <QSettings>
#include "SerialConfig.h"
#include "Threshold.h"

class ConfigManager
{
public:
	ConfigManager();
	~ConfigManager();

	//串口
	void saveSerialConfig(const SerialConfig& cfg);
	SerialConfig loadSerialConfig() const;

	// 阈值
	void saveThresholds(const QMap<quint8, Threshold>& thresholds);
	QMap<quint8, Threshold> loadThresholds() const;

private:
	QSettings* m_settings = nullptr;
};

