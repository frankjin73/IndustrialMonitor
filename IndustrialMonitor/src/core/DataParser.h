#pragma once

#include <qlist.h>
#include <qbytearray.h>

class DataParser
{
public:

	QList<SensorData> parse(const QByteArray& chunk)

	QByteArray m_buffer;

};

