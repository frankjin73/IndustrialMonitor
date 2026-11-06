#pragma once

#include<QString>

#define MAX_SENSOR_QUANTITY 10   //传感器编号从1开始，最大为 MAX_SENSOR_QUANTITY

typedef struct {
    double minValue;
    double maxValue;
    bool enabled;
    QString sensorName;
}Threshold;

inline QMap<quint8, Threshold> getDefaultThresholds() {
    QMap<quint8, Threshold> defaultMap;
    // 初始默认值（传感器号 -> {最小值, 最大值，是否启用, 名称}）
    defaultMap.insert(1, { 25.0, 30.0, true, "温度" });
    defaultMap.insert(2, { 0.0, 100.0, true, "压力" });
    defaultMap.insert(3, { 20.0, 80.0, true, "液位" });
    // ... 加到10个 ...
    return defaultMap;
}