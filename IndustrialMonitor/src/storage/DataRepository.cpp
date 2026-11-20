#include "DataRepository.h"

void DataRepository::insertSensorData(const SensorData& data) {
	m_pendingBuffer.append(data);
}

QList<SensorData> DataRepository::querySensorData(qint64 fromMs, qint64 toMs) {
	QList<SensorData> dataList;
	QSqlDatabase db = QSqlDatabase::database(m_connName);
	QSqlQuery query(db);
	query.prepare("SELECT timestamp,device_id,sensor_id,data_type,value FROM sensor_data WHERE timestamp >= ? AND timestamp <= ? ORDER BY timestamp ASC LIMIT 10000");
	query.addBindValue(fromMs);
	query.addBindValue(toMs);

	if (query.exec()) {
		SensorData data;
		while (query.next()) {
			data.timestamp = QDateTime::fromMSecsSinceEpoch(query.value("timestamp").toLongLong());
			data.deviceId = query.value("device_id").toUInt();
			data.sensorId = query.value("sensor_id").toUInt();
			data.dataType = query.value("data_type").toUInt();
			data.value = query.value("value").toDouble();
			dataList.append(data);
		}
	}

	return dataList;
}

void DataRepository::insertAlarmRecord(const AlarmRecord& record) {
	QSqlDatabase db = QSqlDatabase::database(m_connName);

	QSqlQuery query(db);
	query.prepare("INSERT INTO alarm_records (timestamp, sensor_id, message) VALUES ( ?, ?, ?)");
	query.addBindValue(record.timestamp.toMSecsSinceEpoch());	//存毫秒数方便快速查找
	query.addBindValue(record.sensorId);
	query.addBindValue(record.message);

	if (query.exec()) {
		qDebug() << "Inserted alarm_records ID:" << query.lastInsertId().toLongLong();
	}
	else {
		qWarning() << "Insert alarm_records failed:" << query.lastError().text();
	}

}

QList<AlarmRecord> DataRepository::queryAlarmRecord(qint64 fromMs, qint64 toMs) {
	QList<AlarmRecord> recordList;
	QSqlDatabase db = QSqlDatabase::database(m_connName);
	QSqlQuery query(db);
	query.prepare("SELECT timestamp,sensor_id,message FROM alarm_records WHERE timestamp >= ? AND timestamp <= ? ORDER BY timestamp ASC LIMIT 5000");
	query.addBindValue(fromMs);
	query.addBindValue(toMs);

	if (query.exec()) {
		AlarmRecord record;
		while (query.next()) {
			record.timestamp = QDateTime::fromMSecsSinceEpoch(query.value("timestamp").toLongLong());
			record.sensorId = query.value("sensor_id").toUInt();
			record.message = query.value("message").toString();
			recordList.append(record);
		}
	}

	return recordList;
}

void DataRepository::onDatabaseTimeout() {
	if (m_pendingBuffer.isEmpty()) {
		return;
	}

	QSqlDatabase db = QSqlDatabase::database(m_connName);
	if (!db.transaction())   // 开启事务
	{
		qDebug() << "事务开启失败：" << db.lastError();
		return;
	}

	QSqlQuery query(db);
	query.prepare("INSERT INTO sensor_data (timestamp, device_id, sensor_id, data_type, value) VALUES (?, ?, ?, ?, ?)");

	bool allOk = true;
	qint64 okCount = 0;

	for (auto &data : m_pendingBuffer)
	{
		query.bindValue(0, data.timestamp.toMSecsSinceEpoch());	//存毫秒数方便快速查找
		query.bindValue(1, data.deviceId);
		query.bindValue(2, data.sensorId);
		query.bindValue(3, data.dataType);
		query.bindValue(4, data.value);

		if (!query.exec()) {
			qWarning() << "单条插入失败:" << query.lastError().text();
			allOk = false;
			break;
		}
		okCount++;
	}

	if (allOk)
	{
		if (db.commit())
		{
			qDebug() << "批量入库成功，条数:" << okCount;
			m_pendingBuffer.clear();
		}
		else
		{
			qWarning() << "commit失败，回滚:" << db.lastError();
			db.rollback();
		}
	}
	else
	{
		qWarning() << "中间插入失败，全部回滚";
		db.rollback();
	}
}