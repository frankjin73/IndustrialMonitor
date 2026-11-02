#pragma once
#include <QObject>
#include <SensorData.h>

enum AlarmResult { Normal,      //什么也不做
                   Triggered,   //发出报警
                   Recovered,   //恢复报警
                   Error_Threshold };

typedef struct {
    double minValue;
    double maxValue;
    bool enabled;
    QString sensorName;
}Threshold;

class AlarmManager :
    public QObject
{
    Q_OBJECT

public:
    AlarmManager(QObject* parent = nullptr);
    void setThreshold(quint8 sensorId, double min, double max, QString sensorName);
    AlarmResult check(const SensorData &data);
    QString getAlarmMessage(quint8 sensorId);

private:
    QMap<quint8, bool> m_alarmState;    //true为报警，false是正常不报警
    QMap<quint8, Threshold> m_sensorThreshold;
};

