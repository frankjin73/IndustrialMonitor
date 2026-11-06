#pragma once

#include<QString>

#define MAX_SENSOR_QUANTITY 10   //传感器编号从1开始，最大为 MAX_SENSOR_QUANTITY

typedef struct {
    double minValue;
    double maxValue;
    bool enabled;
    QString sensorName;
}Threshold;