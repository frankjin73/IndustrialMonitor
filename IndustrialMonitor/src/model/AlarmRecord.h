#pragma once

#include <QDateTime>

class AlarmRecord
{
    QDateTime timestamp;
    quint8 sensorId;
    QString message;
};

