#include "ConfigManager.h"

ConfigManager::ConfigManager() {
	const QString& configPath = QCoreApplication::applicationDirPath() + "/config.ini";

	 m_settings = new QSettings(configPath, QSettings::IniFormat);

     if (!m_settings->contains("general/initialized")) {
         qDebug() << "未找到配置文件，正在创建默认配置...";

         // 写入默认配置
         m_settings->setValue("general/initialized", true); // 写入标志位，下次就不会进来了

         // 写入串口默认参数
         saveSerialConfig(DEFAULT_SERIALCONFIG);
         
         //写入传感器默认阈值
         saveThresholds(getDefaultThresholds());

         // 写Tcp远程监控参数
         saveTcpPort(DEFAULT_TCPPORT);
     }
     else {
         qDebug() << "已找到配置文件，正在读取...";
     }
}

ConfigManager::~ConfigManager() {
	delete m_settings;
	m_settings = nullptr;
}

void ConfigManager::saveSerialConfig(const SerialConfig& cfg) {
    m_settings->beginGroup("Serial");
    m_settings->setValue("portName", cfg.portName);
    m_settings->setValue("baudRate", cfg.baudRate);
    m_settings->setValue("dataBit", cfg.dataBit);
    m_settings->setValue("parity", cfg.parity);
    m_settings->setValue("stopBits", cfg.stopBits);
    m_settings->endGroup();
    // 强制同步写入磁盘（防止程序突然崩溃导致配置没存上）
    m_settings->sync();
}

SerialConfig ConfigManager::loadSerialConfig() const {
    SerialConfig config;

    m_settings->beginGroup("Serial");
    config.portName = m_settings->value("portName", "COM1").toString();
    config.baudRate = m_settings->value("baudRate", 115200).toInt();
    config.dataBit = m_settings->value("dataBit", 8).toInt();
    config.parity = m_settings->value("parity", 0).toInt();
    config.stopBits = m_settings->value("stopBits", 1).toInt();
    m_settings->endGroup();

    return config;
}

void ConfigManager::saveThresholds(const QMap<quint8, Threshold>& thresholds) {
    m_settings->beginGroup("Alarm");
    for (auto it = thresholds.cbegin(); it!=thresholds.cend();it++) {
        m_settings->beginGroup(QString::number(it.key()));
        auto& threshold = it.value();
        m_settings->setValue("min", threshold.minValue);
        m_settings->setValue("max", threshold.maxValue);
        m_settings->setValue("enabled", threshold.enabled);
        m_settings->setValue("name", threshold.sensorName);
        m_settings->endGroup();
    }
    m_settings->endGroup();
    m_settings->sync();
}

QMap<quint8, Threshold> ConfigManager::loadThresholds() const {
    QMap<quint8, Threshold> thresholds;

    m_settings->beginGroup("Alarm");

    for (int ch = 1; ch <= MAX_SENSOR_QUANTITY; ++ch) {
        m_settings->beginGroup(QString("%1").arg(ch));

        // 如果这个组里连一个键都没有，说明没配置这个通道，直接跳过
        if (!m_settings->contains("name")) {
            m_settings->endGroup();
            continue;
        }

        Threshold th;
        th.minValue = m_settings->value("min", 0.0).toDouble();
        th.maxValue = m_settings->value("max", 100.0).toDouble();
        th.enabled = m_settings->value("enabled", true).toBool();
        th.sensorName = m_settings->value("name", "未知").toString();

        thresholds.insert(ch, th);

        m_settings->endGroup();
    }
    m_settings->endGroup();

    return thresholds;
}

void ConfigManager::saveTcpPort(const quint16& port) {
    m_settings->beginGroup("Tcp");
    m_settings->setValue("port", port);
    m_settings->endGroup();
    m_settings->sync();
}

quint16 ConfigManager::loadTcpPort() const{
    quint16 port;

    m_settings->beginGroup("Tcp");
    port = static_cast<quint16>(m_settings->value("port", DEFAULT_TCPPORT).toInt());
    m_settings->endGroup();

    return port;
}