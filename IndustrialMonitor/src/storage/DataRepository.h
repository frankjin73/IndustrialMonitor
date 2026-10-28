#pragma once
#include "SensorData.h"
#include <QSqlQuery>
#include <QSqlError>

class DataRepository
{
public:
	explicit DataRepository(const QString& connName) : m_connName(connName) {}
	void insertSensorData(const SensorData& data);
	QList<SensorData> querySensorData(qint64 fromMs,qint64 toMs);
private:
	QString m_connName;
};

