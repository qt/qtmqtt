// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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

    void basic_test_data();
    void basic_test();
    void retained_message_test_data();
    void retained_message_test();
    void will_message_test_data();
    void will_message_test();
    void zero_length_clientid_test_data();
    void zero_length_clientid_test();
    void offline_message_queueing_test_data();
    void offline_message_queueing_test();
    // overlapping_subscriptions_test // Skipped at the module emits multiple messages for each sub
    // keepalive_test // The module handles sending ping requests
    void subscribe_failure_test_data();
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

DefaultVersionTestData(Tst_MqttConformance::basic_test_data)

void Tst_MqttConformance::basic_test()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    VersionClient(mqttVersion, client);

    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker");

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect from broker");

    // The MQTT 5 broker might provide topic alias by default. Hence, disable it.
    if (mqttVersion == QMqttClient::MQTT_5_0) {
        QMqttConnectionProperties p;
        p.setMaximumTopicAlias(0);
        client.setConnectionProperties(p);
    }

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker");

    const QString topic(QLatin1String("Qt/conformance"));

    auto sub = client.subscribe(topic, 1);
    QTRY_VERIFY2(sub->state() == QMqttSubscription::Subscribed, "Could not subscribe");

    int msgCount = 0;
    connect(sub, &QMqttSubscription::messageReceived, this, [&msgCount](QMqttMessage) {
        msgCount++;
    });

    connect(&client, &QMqttClient::messageReceived, this, [](const QByteArray &message, const QMqttTopicName &topic)
    {
        Q_UNUSED(message)
        Q_UNUSED(topic)
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
    QTest::addColumn<QMqttClient::ProtocolVersion>("mqttVersion");
    QTest::addColumn<QStringList>("messages");
    QTest::addColumn<int>("expectedMsgCount");

    QList<QMqttClient::ProtocolVersion> versions{QMqttClient::MQTT_3_1_1, QMqttClient::MQTT_5_0};

    for (int i = 0; i < 2; ++i) {
        const QStringList topics1{"qos 0", "qos 1", "qos 2"};
        const QStringList topics2{"", "", ""};

        QTest::newRow(qPrintable(QString::number(versions[i]) + ":receiveRetain")) << versions[i] << topics1 << 3;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":clearRetain")) << versions[i] << topics2 << 0;
    }
}

void Tst_MqttConformance::retained_message_test()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);
    QFETCH(QStringList, messages);
    QFETCH(int, expectedMsgCount);

    VersionClient(mqttVersion, client);

    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker");

    const QStringList topics{"Qt/conformance/tests/retain1", "Qt/conformance/tests/retain2", "Qt/conformance/tests2/retain1"};
    const QString subTop{"Qt/conformance/#"}; // ### TODO: The test suite uses {"Qt/+/+"}; but we do not support ++ yet.

    client.publish(topics[0], messages[0].toUtf8(), 0, true);
    client.publish(topics[1], messages[1].toUtf8(), 1, true);
    qint32 id = client.publish(topics[2], messages[2].toUtf8(), 2, true);
    bool lastPublishSucceeded = false;
    connect(&client, &QMqttClient::messageSent, this, [id, &lastPublishSucceeded](qint32 recId) {
        if (recId == id)
            lastPublishSucceeded = true;
    });
    QTRY_VERIFY2(lastPublishSucceeded, "Could not send retained messages.");

    auto sub = client.subscribe(subTop, 2);
    int msgCount = 0;

    connect(sub, &QMqttSubscription::messageReceived, this, [&msgCount](QMqttMessage) {
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

DefaultVersionTestData(Tst_MqttConformance::will_message_test_data)

void Tst_MqttConformance::will_message_test()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    VersionClient(mqttVersion, client);

    client.setHostname(m_testBroker);
    client.setPort(m_port);

    const QString wTopic{"Qt/conformance/willtest"};
    const QByteArray wMessage{"client got lost"};

    client.setWillMessage(wMessage);
    client.setWillQoS(2);
    client.setWillTopic(wTopic);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker");


    VersionClient(mqttVersion, recipient);
    recipient.setHostname(m_testBroker);
    recipient.setPort(m_port);
    recipient.connectToHost();
    QTRY_VERIFY2(recipient.state() == QMqttClient::Connected, "Could not connect to broker");

    bool receivedWill = false;
    auto sub = recipient.subscribe(wTopic, 1);
    connect(sub, &QMqttSubscription::messageReceived, this, [wMessage, &receivedWill](QMqttMessage m) {
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
    QTest::addColumn<QMqttClient::ProtocolVersion>("mqttVersion");
    QTest::addColumn<bool>("session");

    QList<QMqttClient::ProtocolVersion> versions{QMqttClient::MQTT_3_1_1, QMqttClient::MQTT_5_0};

    for (int i = 0; i < 2; ++i) {
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":noncleanSession")) << versions[i] << false;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":cleanSession")) << versions[i] << true;
    }
}

void Tst_MqttConformance::zero_length_clientid_test()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);
    QFETCH(bool, session);

    VersionClient(mqttVersion, client);

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.setClientId(QLatin1String(""));
    client.setCleanSession(session);

    client.connectToHost();
    QVERIFY2(client.state() == QMqttClient::Connecting, "Could not set state to connecting.");

    if (!session) {
        if (client.protocolVersion() == QMqttClient::MQTT_5_0) {
            // For MQTT 5 the broker creates an ID and returns it in CONNACK
            QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");
            QVERIFY(!client.clientId().isEmpty());
        } else {
            QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Sessions with empty client should not be allowed.");
        }
    } else {
        QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");
        client.disconnectFromHost();
        QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect.");
    }
}

DefaultVersionTestData(Tst_MqttConformance::offline_message_queueing_test_data)

void Tst_MqttConformance::offline_message_queueing_test()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);
    VersionClient(mqttVersion, client);

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.setCleanSession(false);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    const QString subTopic{"Qt/conformance/offline/#"};
    auto sub = client.subscribe(subTopic, 2);
    Q_UNUSED(sub);

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect.");

    VersionClient(mqttVersion, publisher);
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);
    publisher.connectToHost();
    QTRY_VERIFY2(publisher.state() == QMqttClient::Connected, "Could not connect to broker.");

    QSignalSpy pubCounter(&publisher, SIGNAL(messageSent(qint32)));
    publisher.publish(QLatin1String("Qt/conformance/offline/foo/bar"), "msg1", 1);
    publisher.publish(QLatin1String("Qt/conformance/offline/foo/bar2"), "msg2", 1);
    publisher.publish(QLatin1String("Qt/conformance/offline/foo2/bar"), "msg3", 1);
    QTRY_VERIFY2(pubCounter.size() == 3, "Could not publish all messages.");

    publisher.disconnectFromHost();
    QTRY_VERIFY2(publisher.state() == QMqttClient::Disconnected, "Could not disconnect.");

    QSignalSpy receiveCounter(&client, SIGNAL(messageReceived(QByteArray,QMqttTopicName)));

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    // ### TODO: MQTT5 Investigate / Fixme
    if (client.protocolVersion() == QMqttClient::MQTT_5_0)
        QEXPECT_FAIL("", "Offline messages seem not supported with MQTT5", Continue);
    QTRY_VERIFY2(receiveCounter.size() == 3, "Did not receive all offline messages.");

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect.");
}

DefaultVersionTestData(Tst_MqttConformance::subscribe_failure_test_data)

void Tst_MqttConformance::subscribe_failure_test()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    VersionClient(mqttVersion, client);

    const QByteArray forbiddenTopic{"Qt/conformance/nosubscribe"};
    // We do not have a test broker with forbidden topics.
    QSKIP("Missing infrastructure to set forbidden topics");

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    auto sub = client.subscribe(QMqttTopicFilter(forbiddenTopic), 1);
    QVERIFY2(sub->state() == QMqttSubscription::SubscriptionPending, "Could not initiate subscription");

    QTRY_VERIFY2(sub->state() == QMqttSubscription::Error, "Did not receive error state for sub.");

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect.");
}

QTEST_MAIN(Tst_MqttConformance)

#include "tst_conformance.moc"
