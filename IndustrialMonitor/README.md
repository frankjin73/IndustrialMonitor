# IndustrialMonitor
Qt6 + C++14 + VS2026  工业设备数据采集与监控上位机

## 功能
-串口配置（端口刷新与选择、波特率、数据位、校验位、停止位）
-主线程只负责UI显示，剥离串口数据采集到子线程
-串口二进制协议解析，处理粘包/半包
-实时曲线，动态滚动
-状态栏实时显示连接状态与累计接受字节数

## 技术栈
-语言：C++14
-框架：Qt 6.11.2
-界面：Qt Widgets + Qt Designer
-图标：Qt Charts
-通信：QSerialPort
-构建：MSBuild
-并发：QThread + moveToThread

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
│  ├─core
│  │      DataParser.cpp/.h #数据解析
│  │
│  ├─model
│  │      SensorData.cpp/.h #数据模型
│  │
│  ├─thread
│  │      SerialWoker.cpp/.h #子线程采集
│  │
│  └─ui
│      │  IndustrialMonitor.cpp/.h/.ui #主窗口
│      │
│      └─widgets
│               RealtimeChartWidget.cpp/.h #实时曲线
│               SerialConfigWidget.cpp/.h/.ui #串口配置面板
│	
├─tools
│      device_simulator.py # Python 设备模拟器

## 构建与运行

### 环境要求
- Visual Studio 2026（含“使用 C++ 的桌面开发”）
- Qt 6.11.2（msvc2022_64），安装 Qt VS Tools 插件
- Python 3 + pyserial（仅模拟器需要）
- com0com（虚拟串口工具）

### 编译
1. 用 VS 打开 `IndustrialMonitor.slnx`
2. 确认 Qt VS Tools → Qt Versions 已配置 Qt 6.8 LTS
3. 选择 Debug / x64，按 `Ctrl+Shift+B` 生成
4. 按 `F5` 运行

### 运行模拟器
1. 用 com0com 创建虚拟串口对 `COM10` / `COM11`
2. 命令行运行：`python tools/device_simulator.py`
3. 上位机选择 `COM10`，波特率 `115200`，点击“打开串口”

## 当前进度
【第一部分】串口配置、子线程采集数据、协议解析、实时曲线

## 技术亮点
-使用moveToThread把串口数据采集和解析放到子线程，主线程负责UI，高频数据不卡顿
-DataParser使用缓冲区处理粘包/半包，并用CRC16校验包装数据完整性
-RealtimeChartWidget限制数据点数并降频更新坐标轴，包装长时间运行流程