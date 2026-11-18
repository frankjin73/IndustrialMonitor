# IndustrialMonitor
Qt6 + C++14 + VS2026  工业设备数据采集与监控上位机

## 功能
-串口配置（端口刷新与选择、波特率、数据位、校验位、停止位）
-主线程只负责UI显示，剥离串口数据采集到子线程，高频数据UI不卡顿
-串口二进制协议解析，使用缓冲区处理粘包/半包
-实时曲线显示设备数据，每秒刷新，动态滚动，数据点过多时自动移除最旧的点
-状态栏实时显示连接状态与累计接受字节数
-采集数据与异常报警信息自动写入SQLite，支持按时间范围查询数据库，结果以表格展示并且可以保存到CSV文件
-每个传感器可独立配置阈值下限、上限和启用状态
-报警状态机去重：同一传感器从“正常”变为“超限”时产生一条报警记录，持续超限期间不重复报警，恢复正常后再次超限才产生新记录。
-上位机作为TCP Server监听端口，允许多个远程客户端同时连接，实时数据以 JSON 格式广播
-统一日志模块，控制台+文件双输出，按日期分文件（logs/app_yyyy_MM_dd.log）。

## 技术栈
-语言：C++14
-框架：Qt 6.11.2
-构建：MSBuild
-界面：Qt Widgets + Qt Designer
-图表：Qt Charts
-通信：QSerialPort + QSerialPortInfo + QTcpServer + QTcpSocket
-数据库：SQLite + QSqlDatabase + QSqlQuery
-并发：QThread + moveToThread
-配置：QSettings + IniFormat
-日志：qInstallMessageHandler + QMutex
-测试：Python + pyserial + socket
-版本管理：Git + Git Extensions + GitHub

## 项目结构
IndustrialMonitor/
├─docs
│      protocol.md #协议文档
├─resources
│      IndustrialMonitor.qrc #qrc 资源
│
├─src
│  │  main.cpp
│  │
│	├─communication
│	│		TcpServerManager.cpp/.h #TCP客户端，负责广播数据到客户端
│	│
│	│
│  ├─core
│  │      AlarmManager.cpp/.h #管理阈值，判断是否报警
│  │		DataParser.cpp/.h #数据解析
│	│
│  ├─model #数据封装
│  │		AlarmRecord.cpp/.h #报警记录类
│  │      SensorData.cpp/.h #串口数据模型
│  │		SerialConfig.h #记录串口配置的结构体
│  │		Threshold.h #阈值结构体
│  │
│  ├─storage
│  │      DatabaseManager.cpp/.h #开关数据库&建表
│  │      DataRepository.cpp/.h #查询&插入数据
│  │
│  ├─thread
│  │      SerialWoker.cpp/.h #子线程：采集&分析数据、管理串口
│  │
│  ├─ui
│  │  │  IndustrialMonitor.cpp/.h/.ui #主窗口
│  │  │
│  │  └─widgets
│  │           AlarmSettingDialog.cpp/.h/.ui #数据阈值设置面板
│  │           RealtimeChartWidget.cpp/.h #实时曲线
│  │           SerialConfigWidget.cpp/.h/.ui #串口参数配置面板
│  │
│  ├─utils
│  │      ConfigManager.cpp/.h #管理配置文件，读取&写入.ini文件
│  │      Logger.cpp/.h #日志信息管理，输出到.log文件
│	
├─tools
│      device_simulator.py # Python 串口设备模拟器
│      tcp_client.py # Python TCP客户端模拟器

## 构建与运行

### 环境要求
- Visual Studio 2026（含“使用 C++ 的桌面开发”）
- Qt 6.11.2（msvc2022_64），安装 Qt VS Tools 插件
- Python 3 + pyserial（仅模拟器需要）
- com0com（虚拟串口工具）

### 编译
1. 用 VS 打开 `IndustrialMonitor.slnx`
2. 确认 Qt VS Tools → Qt Versions 已配置 Qt 6.11.2
3. 选择 Debug / x64，按 `Ctrl+Shift+B` 生成
4. 按 `F5` 运行

### 运行串口模拟器
1. 用 com0com 创建虚拟串口对 `COM10` / `COM11`
2. 命令行运行：`python tools/device_simulator.py`
3. 上位机选择 `COM10`，波特率 `115200`，点击“打开串口”

### 运行TCP客户端模拟器
1. 上位机点击工具栏中的`启动远程监控`，若端口被占用，请在弹出的对话框里输入新端口。
2. 命令行运行：`python tools/tcp_client.py`

## 当前进度
【第一部分】串口配置、子线程采集数据、协议解析、实时曲线
【第二部分】SQLite 存储、历史数据查询界面、阈值报警、配置持久化、TCP远程监控、日志模块、异常处理

## 技术亮点
-使用moveToThread把串口数据采集和解析放到子线程，主线程负责UI，高频数据不卡顿
-DataParser使用缓冲区处理粘包/半包，并用CRC16校验保证数据完整性
-RealtimeChartWidget限制数据点数并降频更新坐标轴，保证长时间运行流程
-报警状态机去重：AlarmManager每次check()根据“当前状态+本条是否超限”返回Triggered/Recovered/NoChange三种事件。
-ConfigManager基于QSettings的INI格式，集中管理串口参数、报警阈值、TCP端口。读取失败时用默认值，INI文件不存在或内容损坏时自动回退默认值，不崩溃。
-历史查询按时间范围索引，时间戳用INTEGER而非TEXT类型，加快查询时比较速度