#include "AlarmManager.h"

AlarmManager::AlarmManager(QObject* parent)
    : QObject(parent) {
    m_alarmState.insert(1,false);
    m_sensorThreshold.insert(1, { 25, 30, true ,"温度"});
}

void AlarmManager::setThreshold(quint8 sensorId, double min, double max, QString sensorName) {

}


AlarmResult AlarmManager::check(const SensorData& data) {

    if (!m_sensorThreshold.contains(data.sensorId)|| !m_alarmState.contains(data.sensorId))
        return Error_Threshold;

    Threshold th = m_sensorThreshold.value(data.sensorId);
    bool state = m_alarmState.value(data.sensorId);
    if (th.enabled == false) {
        return Normal;  //无阈值限制时默认正常
    }

    if (data.value< th.minValue || data.value>th.maxValue) {
        if (state == false) {
            m_alarmState[data.sensorId] = true;
            return Triggered;   //产生一条新报警
        }
        else {
            return Normal;  //持续超限，不重复报警
        }
    }
    else {
        if (state == true) {
            m_alarmState[data.sensorId] = false;
            return Recovered;   //报警恢复
        }
        else {
            return Normal;  
        }
    }
}

QString AlarmManager::getAlarmMessage(quint8 sensorId) {
    Threshold th = m_sensorThreshold.value(sensorId);
    QString message = th.sensorName + QString("超阈值【%1-%2】,当前温度").arg(QString::number(th.minValue, 'f', 2), QString::number(th.maxValue, 'f', 2));
    return message;
}