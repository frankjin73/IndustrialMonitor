#include "AlarmRecord.h"

AlarmRecord::AlarmRecord(){}

AlarmRecord::AlarmRecord(const QDateTime& timestamp, const quint8& sensorId, const QString& message)
	: timestamp(timestamp)
	, sensorId(sensorId)
	, message(message){

}