#pragma once
#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QTimer>
#include "SensorData.h"
#include "AlarmRecord.h"

class DataRepository : public QObject
{
	Q_OBJECT
public:
	explicit DataRepository(QObject* parent = nullptr,const QString& connName = "")
		: QObject(parent), m_connName(connName),m_dbTimer(new QTimer(this)) 
	{
		m_dbTimer->setInterval(1000);
		m_dbTimer->setTimerType(Qt::CoarseTimer);
		connect(m_dbTimer,&QTimer::timeout,this,&DataRepository::onDatabaseTimeout);
		m_dbTimer->start();
	}

	void insertSensorData(const SensorData& data);
	QList<SensorData> querySensorData(qint64 fromMs,qint64 toMs);
	void insertAlarmRecord(const AlarmRecord& record);
	QList<AlarmRecord> queryAlarmRecord(qint64 fromMs, qint64 toMs);
	inline void flushAll() { onDatabaseTimeout(); qInfo() << "缓存刷库"; }
private slots:
	void onDatabaseTimeout();

private:
	QTimer* m_dbTimer;    //定时批量存入数据库，减少卡顿
	QList<SensorData> m_pendingBuffer;
	QString m_connName;
};

