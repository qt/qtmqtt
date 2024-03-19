// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
#ifdef QT_BUILD_INTERNAL
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
#else
    QSKIP("This test requires a Qt -developer-build.");
#endif
}

void Tst_QMqttControlPacket::append()
{
#ifdef QT_BUILD_INTERNAL
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

    packet.clear();
    packet.appendRaw(data);
    payload = packet.payload();
    QCOMPARE(payload.size(), data.size());
    QCOMPARE(payload, data);

    const QByteArray containsZero("Some data\0 with zero", 21);
    packet.clear();
    packet.appendRaw(containsZero);
    payload = packet.payload();
    QCOMPARE(containsZero, payload);
    QCOMPARE(payload.size(), 21);

    packet.clear();
    packet.append(containsZero);
    payload = packet.payload().mid(2); // mid because size got prepended
    QCOMPARE(containsZero, payload);
    QCOMPARE(payload.size(), 21);
#else
    QSKIP("This test requires a Qt -developer-build.");
#endif
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
