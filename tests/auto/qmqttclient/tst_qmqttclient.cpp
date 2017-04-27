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
#include <QtTest/QSignalSpy>
#include <QtMqtt/QMqttClient>

class Tst_QMqttClient : public QObject
{
    Q_OBJECT

public:
    Tst_QMqttClient();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void getSetCheck();
    void retainMessage();
    void willMessage();
private:
    QString m_testBroker;
    quint16 m_port{1883};
};

Tst_QMqttClient::Tst_QMqttClient()
{
}

void Tst_QMqttClient::initTestCase()
{
    m_testBroker = qgetenv("MQTT_TEST_BROKER");
    if (m_testBroker.isEmpty()) {
        QFAIL("No test server given. Please specify MQTT_TEST_BROKER in your environment.");
        return;
    }
}

void Tst_QMqttClient::cleanupTestCase()
{
}

void Tst_QMqttClient::getSetCheck()
{
    QMqttClient client;

    QVERIFY(client.clientId().size() > 0);
    const QString clientId = QLatin1String("testclient123");
    client.setClientId(clientId);
    QCOMPARE(client.clientId(), clientId);

    QCOMPARE(client.hostname(), QString());
    const QString hostname = QLatin1String("qt.io");
    client.setHostname(hostname);
    QCOMPARE(client.hostname(), hostname);

    QCOMPARE(client.port(), quint16(0));
    client.setPort(1883);
    QCOMPARE(client.port(), quint16(1883));

    QCOMPARE(client.keepAlive(), quint16(60));
    client.setKeepAlive(10);
    QCOMPARE(client.keepAlive(), quint16(10));

    QCOMPARE(client.protocolVersion(), quint8(3));
    client.setProtocolVersion(0);
    QCOMPARE(client.protocolVersion(), quint8(3));
    client.setProtocolVersion(5);
    QCOMPARE(client.protocolVersion(), quint8(3));

    QCOMPARE(client.username(), QString());
    QCOMPARE(client.password(), QString());
    QCOMPARE(client.cleanSession(), true);
    QCOMPARE(client.willTopic(), QString());
    QCOMPARE(client.willMessage(), QString());
    QCOMPARE(client.willQoS(), quint8(0));
    QCOMPARE(client.willRetain(), false);
}

void Tst_QMqttClient::retainMessage()
{
    const QString testTopic = QLatin1String("Topic2");
    const QByteArray testMessage("retainedMessage");

    // Publisher
    QMqttClient publisher;
    publisher.setClientId(QLatin1String("publisher"));
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    publisher.connectToHost();
    QTRY_COMPARE(publisher.state(), QMqttClient::Connected);

    // [MQTT-3.3.1-7]
    // If the Server receives a QoS 0 message with the RETAIN flag set to 1 it MUST
    // discard any message previously retained for that topic. It SHOULD store the
    // new QoS 0 message
    publisher.publish(testTopic, QByteArray(), 0, true);
    // We cannot wait as 0 QoS does not send confirmation
    QTest::qWait(500);

    for (int i = 0; i < 2; i++) {
        int msgCount = 0;

        QSignalSpy publishSpy(&publisher, SIGNAL(messageSent(qint32)));
        publisher.publish(testTopic, testMessage, 1, i == 1 ? true : false);
        QTRY_COMPARE(publishSpy.count(), 1);

        QMqttClient sub;
        sub.setClientId(QLatin1String("SubA"));
        sub.setHostname(m_testBroker);
        sub.setPort(m_port);
        connect(&sub, &QMqttClient::messageReceived, [&msgCount, testMessage](const QByteArray &msg) {
            if (msg == testMessage)
                msgCount++;
        });

        QSignalSpy messageSpy(&sub, SIGNAL(messageReceived(QByteArray,QString)));
        sub.connectToHost();
        QTRY_COMPARE(sub.state(), QMqttClient::Connected);

        auto subscription = sub.subscribe(testTopic);
        QTRY_COMPARE(subscription->state(), QMqttSubscription::Subscribed);

        QTest::qWait(5000);
        QVERIFY(msgCount == i);
    }
    publisher.disconnect();
}

void Tst_QMqttClient::willMessage()
{
    const QString willTopic = QLatin1String("will/topic");
    const QByteArray willMessage("The client died....");

    // Client A connects
    QMqttClient client1;
    client1.setHostname(m_testBroker);
    client1.setPort(m_port);
    client1.connectToHost();
    QTRY_COMPARE(client1.state(), QMqttClient::Connected);

    auto client1Sub = client1.subscribe(willTopic, 1);
    connect(client1Sub.data(), &QMqttSubscription::messageReceived, [=](QString message) {
        // Just debug purposes
        //qDebug() << "Got something:" << message;
    });
    QTRY_COMPARE(client1Sub->state(), QMqttSubscription::Subscribed);

    QSignalSpy messageSpy(client1Sub.data(), SIGNAL(messageReceived(QByteArray,QString)));

    // Client B connects (via TcpSocket)
    QTcpSocket sock;
    sock.connectToHost(m_testBroker, m_port);
    QVERIFY(sock.waitForConnected());

    for (int i = 1; i > 0; --i) {
        QMqttClient willClient;
        if (i == 1)
            willClient.setTransport(&sock, QMqttClient::AbstractSocket);
        else {
            willClient.setHostname(m_testBroker);
            willClient.setPort(m_port);
        }
        willClient.setWillQoS(1);
        willClient.setWillTopic(willTopic);
        willClient.setWillMessage(willMessage);
        willClient.connectToHost();
        QTRY_COMPARE(willClient.state(), QMqttClient::Connected);

        willClient.publish(QLatin1String("noninteresting"), "just something");

        // Be evil and kill the connection without DISCONNECT
        // Should send will message to client1.
        // When you manually disconnect (send the DISCONNECT command) no will message
        // is sent
        if (i == 1)
            sock.disconnectFromHost();
        else
            willClient.disconnectFromHost();
        QTest::qWait(500);
        QTRY_COMPARE(messageSpy.count(), i);
    }
}

QTEST_MAIN(Tst_QMqttClient)

#include "tst_qmqttclient.moc"
