#include "DatabaseManager.h"

DatabaseManager::DatabaseManager() {
	open();
}

DatabaseManager::~DatabaseManager() {
	close();
}

void DatabaseManager::open() {
	db = QSqlDatabase::addDatabase("QSQLITE", connectionName());
	db.setDatabaseName(QCoreApplication::applicationDirPath() + "/monitor.db");
	if (db.open()) {
		qDebug() << "open database successfully";
		initTables();
	}
	else {
		qWarning() << "Error:Failed to open database:" << db.lastError().text();
		return;
	}
}

void DatabaseManager::initTables() {
	QSqlQuery query(db);
	
	query.exec("PRAGMA journal_mode=WAL;");
	query.exec("PRAGMA synchronous=NORMAL;");  // WAL 下兼顾性能和安全,读写互不阻塞
	query.exec("PRAGMA foreign_keys=ON;");     // 启用外键

	if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS sensor_data (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER,
            device_id INTEGER,
            sensor_id INTEGER,
			data_type INTEGER,
			value	  REAL
        )
    )")) {
		qWarning() << "create sensor_data failed:" << query.lastError().text();
	}

	if (!query.exec(R"(
        CREATE TABLE IF NOT EXISTS alarm_records (
            id        INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER,
            sensor_id  INTEGER,
			message	  TEXT
        )
    )")) {
		qWarning() << "create alarm_records failed:" << query.lastError().text();
	}
}

void DatabaseManager::close() {
	if (db.isOpen()) {
                db.close();
            }
	db = QSqlDatabase();
	QSqlDatabase::removeDatabase(connectionName());
}