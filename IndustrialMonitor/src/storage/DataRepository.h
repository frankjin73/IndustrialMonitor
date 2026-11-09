#pragma once
#include <QSqlQuery>
#include <QSqlError>
#include "SensorData.h"
#include "AlarmRecord.h"

class DataRepository
{
public:
	explicit DataRepository(const QString& connName) : m_connName(connName) {}
	void insertSensorData(const SensorData& data);
	QList<SensorData> querySensorData(qint64 fromMs,qint64 toMs);
	void insertAlarmRecord(const AlarmRecord& record);
	QList<AlarmRecord> queryAlarmRecord(qint64 fromMs, qint64 toMs);

private:
	QString m_connName;
};

