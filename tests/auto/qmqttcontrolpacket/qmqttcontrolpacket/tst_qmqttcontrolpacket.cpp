#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtMqtt/private/qmqttcontrolpacket_p.h>

class Tst_QMqttControlPacket : public QObject
{
    Q_OBJECT

public:
    Tst_QMqttControlPacket();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void header();
    void append();
    void simple_data();
    void simple();
};

Tst_QMqttControlPacket::Tst_QMqttControlPacket()
{
}

void Tst_QMqttControlPacket::initTestCase()
{
}

void Tst_QMqttControlPacket::cleanupTestCase()
{
}

void Tst_QMqttControlPacket::header()
{
    {
        QMqttControlPacket packet;
        QVERIFY(packet.header() == QMqttControlPacket::UNKNOWN);
    }
    {
        QMqttControlPacket packet(QMqttControlPacket::CONNECT);
        QVERIFY(packet.header() == QMqttControlPacket::CONNECT);

        packet.setHeader(QMqttControlPacket::DISCONNECT);
        QVERIFY(packet.header() == QMqttControlPacket::DISCONNECT);

        packet.clear();
        QVERIFY(packet.header() == QMqttControlPacket::UNKNOWN);

        packet.setHeader(42);
        QVERIFY(packet.header() == QMqttControlPacket::UNKNOWN);
    }
}

void Tst_QMqttControlPacket::append()
{
    QMqttControlPacket packet;
    QCOMPARE(packet.payload().size(), 0);

    packet.append('0');
    QCOMPARE(packet.payload().size(), 1);
    QVERIFY(packet.payload() == QByteArray("0"));

    packet.clear();
    QCOMPARE(packet.payload().size(), 0);

    const quint16 value = 100;
    packet.append(value);
    QByteArray payload = packet.payload();
    QCOMPARE(payload.size(), 2);
    const quint16 valueBigEndian = qToBigEndian<quint16>(value);
    const quint16 payloadUint16 = *reinterpret_cast<const quint16 *>(payload.constData());
    QCOMPARE(payloadUint16, valueBigEndian);

    packet.clear();
    QCOMPARE(packet.payload().size(), 0);

    const QByteArray data("some data in the packet");
    packet.append(data);
    payload = packet.payload();
    QCOMPARE(payload.size(), data.size() + 2);

    const QByteArray partSize = payload.left(2);
    const QByteArray partContent = payload.mid(2);
    const int partSizeInt = qFromBigEndian<quint16>(*reinterpret_cast<const quint16 *>(partSize.constData()));
    QCOMPARE(partSizeInt, data.size());
    QCOMPARE(partContent, data);
}

void Tst_QMqttControlPacket::simple_data()
{
    QTest::addColumn<QString>("data");
    QTest::newRow("0") << QString();
}

void Tst_QMqttControlPacket::simple()
{
    QFETCH(QString, data);
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(Tst_QMqttControlPacket)

#include "tst_qmqttcontrolpacket.moc"
