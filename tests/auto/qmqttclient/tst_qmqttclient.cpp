﻿/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtMqtt module.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
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
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#include "broker_connection.h"

#include <QtCore/QString>
#include <QtNetwork/QTcpServer>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtMqtt/QMqttClient>

#include <limits>

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
    void compliantTopic_data();
    void compliantTopic();
    void subscribeLongTopic();
    void dataIncludingZero();
    void publishLongTopic();
    void reconnect_QTBUG65726();
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
    connect(sub, &QMqttSubscription::messageReceived, [&](QMqttMessage msg) {
        verified = msg.payload() == data;
        received = true;
    });

    QTRY_COMPARE(sub->state(), QMqttSubscription::Subscribed);

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

        QSignalSpy messageSpy(&sub, SIGNAL(messageReceived(QByteArray,QMqttTopicName)));
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
    connect(client1Sub, &QMqttSubscription::messageReceived, [=](QMqttMessage message) {
        Q_UNUSED(message);
        // Just debug purposes
        //qDebug() << "Got something:" << message;
    });
    QTRY_COMPARE(client1Sub->state(), QMqttSubscription::Subscribed);

    QSignalSpy messageSpy(client1Sub, SIGNAL(messageReceived(QMqttMessage)));

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

void Tst_QMqttClient::compliantTopic_data()
{
    QTest::addColumn<QString>("topic");
    QTest::newRow("simple") << QString::fromLatin1("topic");
    QTest::newRow("subPath") << QString::fromLatin1("topic/subtopic");

    QString l;
    l.fill(QLatin1Char('T'), std::numeric_limits<unsigned short>::max());
    QTest::newRow("maxSize") << l;
}

void Tst_QMqttClient::compliantTopic()
{
    QFETCH(QString, topic);
    QString truncTopic = topic;
    truncTopic.truncate(std::numeric_limits<unsigned short>::max());

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
    connect(&subscriber, &QMqttClient::messageReceived, [&](const QByteArray &, const QMqttTopicName &t) {
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
    topic.fill(QLatin1Char('s'), 2 * std::numeric_limits<unsigned short>::max());
    auto sub = subscriber.subscribe(topic);
    QObject *obj = nullptr;
    QCOMPARE(sub, obj);
}

void Tst_QMqttClient::dataIncludingZero()
{
    QByteArray data;
    const int dataSize = 200;
    data.fill('A', dataSize);
    data[100] = '\0';

    QMqttClient client;
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    bool received = false;
    bool verified = false;
    bool correctSize = false;
    const QString testTopic(QLatin1String("some/topic"));
    auto sub = client.subscribe(testTopic, 1);
    QVERIFY(sub);
    connect(sub, &QMqttSubscription::messageReceived, [&](QMqttMessage msg) {
        verified = msg.payload() == data;
        correctSize = msg.payload().size() == dataSize;
        received = true;
    });

    QTRY_COMPARE(sub->state(), QMqttSubscription::Subscribed);

    client.publish(testTopic, data, 1);

    QTRY_VERIFY2(received, "Subscriber did not receive message");
    QVERIFY2(verified, "Subscriber received different message");
    QVERIFY2(correctSize, "Subscriber received message of different size");
}

void Tst_QMqttClient::publishLongTopic()
{
    QMqttClient publisher;
    publisher.setClientId(QLatin1String("publisher"));
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    publisher.connectToHost();
    QTRY_COMPARE(publisher.state(), QMqttClient::Connected);

    QString topic;
    topic.fill(QLatin1Char('s'), 2 * std::numeric_limits<unsigned short>::max());
    auto pub = publisher.publish(topic);
    QCOMPARE(pub, -1);
}

class FakeServer : public QObject
{
    Q_OBJECT
public:
    FakeServer() {
        server = new QTcpServer();
        connect(server, &QTcpServer::newConnection, this, &FakeServer::createSocket);
        server->listen(QHostAddress::Any, 5726);
    }
public slots:
    void createSocket() {
        socket = server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &FakeServer::connectionRequested);
    }

    void connectionRequested() {
        // We assume it is always a connect statement, so no verification is done
        socket->readAll();
        QByteArray response;
        response += 0x20;
        response += quint8(2); // Payload size
        if (!connectionSuccess) {
            response += quint8(255); // Causes ProtocolViolation
            response += quint8(13);
        } else {
            response += char(0); // ackFlags
            response += char(0); // result
        }
        qDebug() << "Fake server response:" << connectionSuccess;
        socket->write(response);
    }
public:
    QTcpServer *server;
    QTcpSocket *socket;
    bool connectionSuccess{false};
};

void Tst_QMqttClient::reconnect_QTBUG65726()
{
    FakeServer server;

    QMqttClient client;
    client.setClientId(QLatin1String("bugclient"));
    client.setHostname(QLatin1String("localhost"));
    client.setPort(5726);

    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Disconnected);
    QTRY_COMPARE(client.error(), QMqttClient::ProtocolViolation);

    server.connectionSuccess = true;

    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);
    QTRY_COMPARE(client.error(), QMqttClient::NoError);
}

QTEST_MAIN(Tst_QMqttClient)

#include "tst_qmqttclient.moc"
