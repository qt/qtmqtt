// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "broker_connection.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttConnectionProperties>

class tst_QMqttLastWillProperties : public QObject
{
    Q_OBJECT

public:
    tst_QMqttLastWillProperties();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void getSet();
    void payloadFormat();
    void willDelay_data();
    void willDelay();

private:
    QProcess m_brokerProcess;
    QString m_testBroker;
    quint16 m_port{1883};
};

tst_QMqttLastWillProperties::tst_QMqttLastWillProperties()
{
}

void tst_QMqttLastWillProperties::initTestCase()
{
    m_testBroker = invokeOrInitializeBroker(&m_brokerProcess);
    if (m_testBroker.isEmpty())
        qFatal("No MQTT broker present to test against.");
}

void tst_QMqttLastWillProperties::cleanupTestCase()
{
}

void tst_QMqttLastWillProperties::getSet()
{
    QMqttLastWillProperties properties;

    QCOMPARE(properties.willDelayInterval(), 0u);
    properties.setWillDelayInterval(50);
    QCOMPARE(properties.willDelayInterval(), 50u);

    QCOMPARE(properties.payloadFormatIndicator(), QMqtt::PayloadFormatIndicator::Unspecified);
    properties.setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator::UTF8Encoded);
    QCOMPARE(properties.payloadFormatIndicator(), QMqtt::PayloadFormatIndicator::UTF8Encoded);

    QCOMPARE(properties.messageExpiryInterval(), 0u);
    properties.setMessageExpiryInterval(50);
    QCOMPARE(properties.messageExpiryInterval(), 50u);

    const QString content = QLatin1String("contentType");
    const QString response = QLatin1String("Qt/some/responseTopic");
    const QByteArray correlation(500, char('a'));

    QCOMPARE(properties.contentType(), QString());
    properties.setContentType(content);
    QCOMPARE(properties.contentType(), content);

    QCOMPARE(properties.responseTopic(), QString());
    properties.setResponseTopic(response);
    QCOMPARE(properties.responseTopic(), response);

    QCOMPARE(properties.correlationData(), QByteArray());
    properties.setCorrelationData(correlation);
    QCOMPARE(properties.correlationData(), correlation);

    QVERIFY(properties.userProperties().isEmpty());
    QMqttUserProperties user;
    user.append(QMqttStringPair(QLatin1String("SomeName"), QLatin1String("SomeValue")));
    user.append(QMqttStringPair(QLatin1String("SomeName2"), QLatin1String("SomeValue2")));
    properties.setUserProperties(user);
    QCOMPARE(properties.userProperties(), user);
}

void tst_QMqttLastWillProperties::payloadFormat()
{
    QMqttClient client;
    client.setProtocolVersion(QMqttClient::MQTT_5_0);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.setWillMessage(QString::fromLatin1("willmessage to utf8").toUtf8());
    client.setWillTopic(QLatin1String("Qt/LastWillProperties/willtopic"));
    client.setWillQoS(1);

    QMqttLastWillProperties lastWill;
    lastWill.setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator::UTF8Encoded);

    // The broker MAY verify the content is utf8
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");
}

void tst_QMqttLastWillProperties::willDelay_data()
{
    QTest::addColumn<int>("delay");
    QTest::addColumn<int>("expiry");
    QTest::newRow("delay == expiry") << 5 << 5;
    QTest::newRow("delay < expiry") << 3 << 10; // will delay is send first
    QTest::newRow("delay > expiry") << 10 << 3; // will is send at expiry
    QTest::newRow("delay > expiry(0)") << 5 << 0; // No expiry, hence will is send immediately
}

void tst_QMqttLastWillProperties::willDelay()
{
    QFETCH(int, delay);
    QFETCH(int, expiry);

    QMqttClient client;
    client.setProtocolVersion(QMqttClient::MQTT_5_0);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    const QString wTopic{"Qt/lastwillproperties/willdelay"};
    const QByteArray wMessage{"client got lost"};

    client.setWillMessage(wMessage);
    client.setWillQoS(2);
    client.setWillTopic(wTopic);

    if (expiry > 0) {
        // If a session expires first, then last will is send immediately
        QMqttConnectionProperties connectionProperties;
        connectionProperties.setSessionExpiryInterval(quint32(expiry));
        client.setConnectionProperties(connectionProperties);
    }

    QMqttLastWillProperties lastWillProperties;
    lastWillProperties.setWillDelayInterval(quint32(delay));
    client.setLastWillProperties(lastWillProperties);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker");

    QMqttClient recipient;
    recipient.setProtocolVersion(QMqttClient::MQTT_5_0);
    recipient.setHostname(m_testBroker);
    recipient.setPort(m_port);
    recipient.connectToHost();
    QTRY_VERIFY2(recipient.state() == QMqttClient::Connected, "Could not connect to broker");

    QElapsedTimer delayTimer;
    bool receivedWill = false;
    auto sub = recipient.subscribe(wTopic, 1);
    connect(sub, &QMqttSubscription::messageReceived, this, [wMessage, &receivedWill](QMqttMessage m) {
        if (m.payload() == wMessage)
            receivedWill = true;
    });
    QTRY_VERIFY2(sub->state() == QMqttSubscription::Subscribed, "Could not subscribe.");

    auto transport = client.transport();
    transport->close(); // closing transport does not send DISCONNECT
    delayTimer.start();

    const int minimalWait = qMin(delay, expiry) * 1000;
    const int maximumWait = 10 * (minimalWait == 0 ? 1000 : minimalWait);
    QTRY_VERIFY2_WITH_TIMEOUT(receivedWill, "Did not receive a will message", delay * 1000 * 3);
    const int elapsed = delayTimer.elapsed();
    QVERIFY(elapsed > minimalWait);
    QVERIFY(elapsed < maximumWait);
}

QTEST_MAIN(tst_QMqttLastWillProperties)

#include "tst_qmqttlastwillproperties.moc"
