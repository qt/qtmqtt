/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtMqtt module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#include "broker_connection.h"

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
    void sendReceive_data();
    void sendReceive();
    void retainMessage();
    void willMessage();
    void longTopic_data();
    void longTopic();
    void subscribeLongTopic();
private:
    QProcess m_brokerProcess;
    QString m_testBroker;
    quint16 m_port{1883};
};

Tst_QMqttClient::Tst_QMqttClient()
{
}

void Tst_QMqttClient::initTestCase()
{
    m_testBroker = invokeOrInitializeBroker(&m_brokerProcess);
    if (m_testBroker.isEmpty())
        qFatal("No MQTT broker present to test against.");
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

    QCOMPARE(client.protocolVersion(), QMqttClient::MQTT_3_1_1);
    client.setProtocolVersion(QMqttClient::ProtocolVersion(0));
    QCOMPARE(client.protocolVersion(), QMqttClient::MQTT_3_1_1);
    client.setProtocolVersion(QMqttClient::ProtocolVersion(5));
    QCOMPARE(client.protocolVersion(), QMqttClient::MQTT_3_1_1);
    client.setProtocolVersion(QMqttClient::MQTT_3_1);
    QCOMPARE(client.protocolVersion(), QMqttClient::MQTT_3_1);

    QCOMPARE(client.username(), QString());
    QCOMPARE(client.password(), QString());
    QCOMPARE(client.cleanSession(), true);
    QCOMPARE(client.willTopic(), QString());
    QCOMPARE(client.willMessage(), QByteArray());
    QCOMPARE(client.willQoS(), quint8(0));
    QCOMPARE(client.willRetain(), false);
}

void Tst_QMqttClient::sendReceive_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::newRow("empty") << QByteArray();
    QTest::newRow("simple") << QByteArray("This is a test message");
    QByteArray d;
    d.fill('A', 500);
    QTest::newRow("big") << d;
    d.fill('B', (128 * 128 * 128) + 4);
    QTest::newRow("huge") << d;
}

void Tst_QMqttClient::sendReceive()
{
    QFETCH(QByteArray, data);
    const QString testTopic = QLatin1String("Topic");

    QMqttClient publisher;
    publisher.setClientId(QLatin1String("publisher"));
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    publisher.connectToHost();
    QTRY_COMPARE(publisher.state(), QMqttClient::Connected);

    QMqttClient subscriber;
    subscriber.setClientId(QLatin1String("subscriber"));
    subscriber.setHostname(m_testBroker);
    subscriber.setPort(m_port);

    subscriber.connectToHost();
    QTRY_COMPARE(subscriber.state(), QMqttClient::Connected);

    bool received = false;
    bool verified = false;
    auto sub = subscriber.subscribe(testTopic, 1);
    QVERIFY(sub);
    connect(sub.data(), &QMqttSubscription::messageReceived, [&](QMqttMessage msg) {
        verified = msg.payload() == data;
        received = true;
    });

    QTRY_COMPARE(sub.data()->state(), QMqttSubscription::Subscribed);

    publisher.publish(testTopic, data, 1);

    QTRY_VERIFY2(received, "Subscriber did not receive message");
    QVERIFY2(verified, "Subscriber received different message");
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
    connect(client1Sub.data(), &QMqttSubscription::messageReceived, [=](QMqttMessage message) {
        Q_UNUSED(message);
        // Just debug purposes
        //qDebug() << "Got something:" << message;
    });
    QTRY_COMPARE(client1Sub->state(), QMqttSubscription::Subscribed);

    QSignalSpy messageSpy(client1Sub.data(), SIGNAL(messageReceived(QMqttMessage)));

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

void Tst_QMqttClient::longTopic_data()
{
    QTest::addColumn<QString>("topic");
    QTest::newRow("simple") << QString::fromLatin1("topic");
    QTest::newRow("subPath") << QString::fromLatin1("topic/subtopic");

    QString l;
    l.fill(QLatin1Char('T'), UINT16_MAX);
    QTest::newRow("maxSize") << l;
    l.fill(QLatin1Char('M'), 2 * UINT16_MAX);
    QTest::newRow("overflow") << l;
}

void Tst_QMqttClient::longTopic()
{
    QFETCH(QString, topic);
    QString truncTopic = topic;
    truncTopic.truncate(UINT16_MAX);

    QMqttClient publisher;
    publisher.setClientId(QLatin1String("publisher"));
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    publisher.connectToHost();
    QTRY_COMPARE(publisher.state(), QMqttClient::Connected);

    QMqttClient subscriber;
    subscriber.setClientId(QLatin1String("subscriber"));
    subscriber.setHostname(m_testBroker);
    subscriber.setPort(m_port);

    subscriber.connectToHost();
    QTRY_COMPARE(subscriber.state(), QMqttClient::Connected);

    bool received = false;
    bool verified = false;
    connect(&subscriber, &QMqttClient::messageReceived, [&](const QByteArray &, const QString &t) {
        received = true;
        verified = t == truncTopic;
    });

    auto client1Sub = subscriber.subscribe(truncTopic, 1);
    QTRY_COMPARE(client1Sub->state(), QMqttSubscription::Subscribed);

    publisher.publish(topic, QByteArray("Msgs"), 1);

    QTRY_VERIFY2(received, "Subscriber did not receive message");
    QVERIFY2(verified, "Subscriber received different message");
}

void Tst_QMqttClient::subscribeLongTopic()
{
    QMqttClient subscriber;
    subscriber.setClientId(QLatin1String("subscriber"));
    subscriber.setHostname(m_testBroker);
    subscriber.setPort(m_port);

    subscriber.connectToHost();
    QTRY_COMPARE(subscriber.state(), QMqttClient::Connected);

    QString topic;
    topic.fill(QLatin1Char('s'), 2 * UINT16_MAX);
    auto sub = subscriber.subscribe(topic);
    QVERIFY(sub.isNull());
}

QTEST_MAIN(Tst_QMqttClient)

#include "tst_qmqttclient.moc"
