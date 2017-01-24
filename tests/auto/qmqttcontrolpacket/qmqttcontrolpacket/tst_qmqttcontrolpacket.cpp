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
    }
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
