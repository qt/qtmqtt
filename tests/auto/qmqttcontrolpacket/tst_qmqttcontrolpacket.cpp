/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Mqtt module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
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

    packet.clear();
    packet.appendRaw(data);
    payload = packet.payload();
    QCOMPARE(payload.size(), data.size());
    QCOMPARE(payload, data);
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
