#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
class DatabaseManager
{
public :
	DatabaseManager();
	~DatabaseManager();
	QString connectionName() const { return "monitor_conn"; }
private:
	QSqlDatabase db;

	void open();
	void initTables();
	void close();
};

