#include <QtCore/QCoreApplication>
#include <QDebug>
#include "test_data.h"

bool runTest(const QString& name, std::function<bool()> fn) {
    bool ok = false;
    try
    {
        ok = fn();
    }
    catch (...)
    {
        qDebug().noquote() << "[EXCEPTION] " << name;
        return false;
    }

    if (ok)
    {
        qDebug().noquote() << "[PASS] " << name;
    }
    else
    {
        qDebug().noquote() << "[FAIL] " << name;
    }
    return ok;
}

bool testCompleteFrame() {
    auto parser = DataParser();
    auto frame = makeFrame(1,22.5);
    parser.m_buffer.append(frame);
    auto result = parser.parse(frame);

    Q_ASSERT_X(result.size() == 1, "testCompleteFrame", "帧数错误");
    Q_ASSERT_X(result.at(0).value == 22.5, "testCompleteFrame", "数据错误");

    return true;
}

bool testStickyPackets() {
    auto parser = DataParser();
    auto frame1 = makeFrame(1, 22.5);
    auto frame2 = makeFrame(2, 60.0);
    auto frame = frame1 + frame2;
    parser.m_buffer.append(frame);
    auto result = parser.parse(frame);
    Q_ASSERT_X(result.size() == 2, "testCompleteFrame", "帧数错误");
    Q_ASSERT_X(result.at(0).sensorId == 1&& result.at(1).sensorId == 2, "testCompleteFrame", "数据错误");

    return true;
}

bool testHalfPacket() {
    auto parser = DataParser();
    auto frame = makeFrame(1, 22.5);

    parser.m_buffer.append(frame.mid(0, 4));
    auto result1 = parser.parse(frame.mid(0, 4));
    Q_ASSERT_X(result1.size() == 0, "testCompleteFrame", "帧数错误");

    parser.m_buffer.append(frame.mid(4));
    auto result2 = parser.parse(frame.mid(0, 4));
    Q_ASSERT_X(result2.size() == 1, "testCompleteFrame", "帧数错误");

    return true;
}

bool testCrcFailure() {
    auto parser = DataParser();
    auto frame = makeFrame(1, 22.5);
    frame[frame.size() - 1] ^= 0xFF;
    parser.m_buffer.append(frame);
    auto result = parser.parse(frame);

    Q_ASSERT_X(result.size() == 0, "testCompleteFrame", "帧数错误");
    return true;
}

bool testNoiseBeforeFrame() {
    auto parser = DataParser();
    QByteArray noise; noise.append(0x11); noise.append(0x22); noise.append(0x33);
    auto frame = makeFrame(1, 22.5);
    frame = noise + frame;
    parser.m_buffer.append(frame);
    auto result = parser.parse(frame);

    Q_ASSERT_X(result.size() == 1, "testCompleteFrame", "帧数错误");
    return true;
}

bool testCrcKnownValue() {
    QByteArray testData = "123456789";
    quint16 crc = DataParser::crc16Modbus(testData);
    Q_ASSERT_X(crc == 0x4B37, "testCompleteFrame", "crc计算函数错误");
    return true;
}

int main(int argc, char *argv[])
{
    int passCnt = 0;
    int failCnt = 0;

    auto run = [&](const QString& n, std::function<bool()> f) {
        if (runTest(n, f))
            ++passCnt;
        else ++failCnt;
    };

    run("完整帧", testCompleteFrame);
    run("粘包", testStickyPackets);
    run("半包", testHalfPacket);
    run("CRC失败", testCrcFailure);
    run("帧头噪声", testNoiseBeforeFrame);
    run("CRC已知值测试", testCrcKnownValue);

    qDebug() << "\n==== Test Summary ====";
    qDebug() << "Pass:" << passCnt << " Fail:" << failCnt;
    qDebug() << "Total:" << passCnt + failCnt;
    return failCnt > 0 ? 1 : 0;
}

