#pragma once

#include <QDateTime>

class AlarmRecord
{
public:
    AlarmRecord();
    AlarmRecord(const QDateTime&, const quint8&, const QString&);
    QDateTime timestamp;
    quint8 sensorId;
    QString message;
};

