#pragma once

#include <QString>

typedef struct {
	QString portName;
	qint32 baudRate;
	qint8 dataBit;
	qint8 parity; 
	qint8 stopBits;
}SerialConfig;