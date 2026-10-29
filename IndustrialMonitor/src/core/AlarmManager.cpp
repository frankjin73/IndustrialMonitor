#include "AlarmManager.h"

AlarmManager::AlarmManager(QObject* parent)
    : QObject(parent) {
    m_alarmState.insert(1,false);
    m_sensorThreshold.insert(1, { 25, 30, true });
}

void AlarmManager::setThreshold(quint8 sensorId, double min, double max) {

}


AlarmResult AlarmManager::check(const SensorData& data) {

    if (!m_sensorThreshold.contains(data.sensorId)|| m_alarmState.contains(data.sensorId))
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
