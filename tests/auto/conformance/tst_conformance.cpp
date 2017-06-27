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
#include <QtMqtt>
#include <QtMqtt/private/qmqttclient_p.h>

#include <functional>

// We mimick the Conformance test suite by the paho project
// paho uses python scripts (client_test.py). Function names are identical
// to better match updates

class Tst_MqttConformance : public QObject
{
    Q_OBJECT

public:
    Tst_MqttConformance();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void basic_test();
    void retained_message_test_data();
    void retained_message_test();
    void will_message_test();
    void zero_length_clientid_test_data();
    void zero_length_clientid_test();
    void offline_message_queueing_test();
    // overlapping_subscriptions_test // Skipped at the module emits multiple messages for each sub
    // keepalive_test // The module handles sending ping requests
    void subscribe_failure_test();
private:
    QProcess m_brokerProcess;
    QString m_testBroker;
    quint16 m_port{1883};
};

Tst_MqttConformance::Tst_MqttConformance()
{
}

void Tst_MqttConformance::initTestCase()
{
    m_testBroker = invokeOrInitializeBroker(&m_brokerProcess);
    if (m_testBroker.isEmpty()) {
        QFAIL("No test server given. Please specify MQTT_TEST_BROKER in your environment.");
        return;
    }
}

void Tst_MqttConformance::cleanupTestCase()
{
}

void Tst_MqttConformance::basic_test()
{
    QMqttClient client;

    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker");

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect from broker");

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker");

    const QString topic(QLatin1String("Qt/conformance"));

    auto sub = client.subscribe(topic, 2);
    QTRY_VERIFY2(sub->state() == QMqttSubscription::Subscribed, "Could not subscribe");

    int msgCount = 0;
    connect(sub.data(), &QMqttSubscription::messageReceived, this, [&msgCount](QMqttMessage msg) {
        qDebug() << "Message received:" << msg.payload();
        msgCount++;
    });

    client.publish(topic, "qos 0", 0);
    client.publish(topic, "qos 1", 1);
    client.publish(topic, "qos 2", 2);

    QTRY_VERIFY2(msgCount == 3, "Did not receive all messages");

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect");

    // Other tests in paho
    // - Connecting twice is captured in the library
    // - Cannot connect with invalid protocol name
}

void Tst_MqttConformance::retained_message_test_data()
{
    QTest::addColumn<QStringList>("messages");
    QTest::addColumn<int>("expectedMsgCount");

    const QStringList topics1{"qos 0", "qos 1", "qos 2"};
    const QStringList topics2{"", "", ""};

    QTest::newRow("receiveRetain") << topics1 << 3;
    QTest::newRow("clearRetain") << topics2 << 0;
}

void Tst_MqttConformance::retained_message_test()
{
    QFETCH(QStringList, messages);
    QFETCH(int, expectedMsgCount);

    QMqttClient client;

    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker");

    const QStringList topics{"Qt/tests/retain1", "Qt/tests/retain2", "Qt/tests2/retain1"};
    const QString subTop{"Qt/#"}; // ### TODO: The test suite uses {"Qt/+/+"}; but we do not support ++ yet.

    client.publish(topics[0], messages[0].toLocal8Bit(), 0, true);
    client.publish(topics[1], messages[1].toLocal8Bit(), 1, true);
    qint32 id = client.publish(topics[2], messages[2].toLocal8Bit(), 2, true);
    bool lastPublishSucceeded = false;
    connect(&client, &QMqttClient::messageSent, this, [id, &lastPublishSucceeded](qint32 recId) {
        if (recId == id)
            lastPublishSucceeded = true;
    });
    QTRY_VERIFY2(lastPublishSucceeded, "Could not send retained messages.");

    auto sub = client.subscribe(subTop, 2);
    int msgCount = 0;

    connect(sub.data(), &QMqttSubscription::messageReceived, this, [&msgCount](QMqttMessage) {
        msgCount++;
        qDebug() << "Message received, current count:" << msgCount;
    });

    if (expectedMsgCount) {
        QTRY_VERIFY2(msgCount == expectedMsgCount, "Did not receive all retained messages.");
    } else {
        QTest::qWait(3000);
        QVERIFY2(msgCount == expectedMsgCount, "Receive retained message though queue empty.");
    }

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect");
}

void Tst_MqttConformance::will_message_test()
{
    QMqttClient client;

    client.setHostname(m_testBroker);
    client.setPort(m_port);

    const QString wTopic{"Qt/willtest"};
    const QByteArray wMessage{"client got lost"};

    client.setWillMessage(wMessage);
    client.setWillQoS(2);
    client.setWillTopic(wTopic);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker");


    QMqttClient recipient;
    recipient.setHostname(m_testBroker);
    recipient.setPort(m_port);
    recipient.connectToHost();
    QTRY_VERIFY2(recipient.state() == QMqttClient::Connected, "Could not connect to broker");

    bool receivedWill = false;
    auto sub = recipient.subscribe(wTopic, 1);
    connect(sub.data(), &QMqttSubscription::messageReceived, this, [wMessage, &receivedWill](QMqttMessage m) {
        if (m.payload() == wMessage)
            receivedWill = true;
    });
    QTRY_VERIFY2(sub->state() == QMqttSubscription::Subscribed, "Could not subscribe.");

    auto transport = client.transport();
    transport->close(); // closing transport does not send DISCONNECT

    QTRY_VERIFY2(receivedWill, "Did not receive a will message");
}

void Tst_MqttConformance::zero_length_clientid_test_data()
{
    QTest::addColumn<bool>("session");

    QTest::newRow("noncleanSession") << false;
    QTest::newRow("cleanSession") << true;
}

void Tst_MqttConformance::zero_length_clientid_test()
{
    QFETCH(bool, session);

    QMqttClient client;

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.setClientId(QLatin1String(""));
    client.setCleanSession(session);

    client.connectToHost();
    QVERIFY2(client.state() == QMqttClient::Connecting, "Could not set state to connecting.");

    if (!session) {
        QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Sessions with empty client should not be allowed.");
    } else {
        QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");
        client.disconnectFromHost();
        QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect.");
    }
}

void Tst_MqttConformance::offline_message_queueing_test()
{
    QMqttClient client;

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.setCleanSession(false);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    const QString subTopic{"Qt/offline/#"};
    auto sub = client.subscribe(subTopic, 2);

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect.");

    QMqttClient publisher;
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);
    publisher.connectToHost();
    QTRY_VERIFY2(publisher.state() == QMqttClient::Connected, "Could not connect to broker.");

    QSignalSpy pubCounter(&publisher, SIGNAL(messageSent(qint32)));
    publisher.publish("Qt/offline/foo/bar", "msg1", 1);
    publisher.publish("Qt/offline/foo/bar2", "msg2", 1);
    publisher.publish("Qt/offline/foo2/bar", "msg3", 1);
    QTRY_VERIFY2(pubCounter.size() == 3, "Could not publish all messages.");

    publisher.disconnectFromHost();
    QTRY_VERIFY2(publisher.state() == QMqttClient::Disconnected, "Could not disconnect.");

    QSignalSpy receiveCounter(&client, SIGNAL(messageReceived(QByteArray,QString)));

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    QTRY_VERIFY2(receiveCounter.size() == 3, "Did not receive all offline messages.");

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect.");
}

void Tst_MqttConformance::subscribe_failure_test()
{
    QMqttClient client;

    const QByteArray forbiddenTopic{"nosubscribe"};
    // We do not have a test broker with forbidden topics.
    QSKIP("Missing infrastructure to set forbidden topics");

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    auto sub = client.subscribe(forbiddenTopic, 1);
    QVERIFY2(sub->state() == QMqttSubscription::SubscriptionPending, "Could not initiate subscription");

    QTRY_VERIFY2(sub->state() == QMqttSubscription::Error, "Did not receive error state for sub.");

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect.");
}

QTEST_MAIN(Tst_MqttConformance)

#include "tst_conformance.moc"
