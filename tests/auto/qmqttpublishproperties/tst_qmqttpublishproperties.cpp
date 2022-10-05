// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "broker_connection.h"

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttPublishProperties>
#include <QtMqtt/QMqttSubscription>

class tst_QMqttPublishProperties : public QObject
{
    Q_OBJECT

public:
    tst_QMqttPublishProperties();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void getSet();
    void propertyConsistency();
    void topicAlias();

private:
    QProcess m_brokerProcess;
    QString m_testBroker;
    quint16 m_port{1883};
};

tst_QMqttPublishProperties::tst_QMqttPublishProperties()
{
}

void tst_QMqttPublishProperties::initTestCase()
{
    m_testBroker = invokeOrInitializeBroker(&m_brokerProcess);
    if (m_testBroker.isEmpty())
        qFatal("No MQTT broker present to test against.");
}

void tst_QMqttPublishProperties::cleanupTestCase()
{
}

void tst_QMqttPublishProperties::getSet()
{
    QMqttPublishProperties p;

    QCOMPARE(p.availableProperties(), 0);

    QVERIFY(!(p.availableProperties() & QMqttPublishProperties::PayloadFormatIndicator));
    QCOMPARE(p.payloadFormatIndicator(), QMqtt::PayloadFormatIndicator::Unspecified);
    p.setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator::UTF8Encoded);
    QCOMPARE(p.payloadFormatIndicator(), QMqtt::PayloadFormatIndicator::UTF8Encoded);
    QVERIFY(p.availableProperties() & QMqttPublishProperties::PayloadFormatIndicator);

    QVERIFY(!(p.availableProperties() & QMqttPublishProperties::MessageExpiryInterval));
    p.setMessageExpiryInterval(200);
    QVERIFY(p.availableProperties() & QMqttPublishProperties::MessageExpiryInterval);
    QCOMPARE(p.messageExpiryInterval(), 200u);

    QVERIFY(!(p.availableProperties() & QMqttPublishProperties::TopicAlias));
    p.setTopicAlias(1);
    QCOMPARE(p.topicAlias(), 1);
    QVERIFY(p.availableProperties() & QMqttPublishProperties::TopicAlias);
    p.setTopicAlias(0); // Zero is not allowed
    QCOMPARE(p.topicAlias(), 1);

    const QString responseTopic = QLatin1String("reply/to/this");
    QVERIFY(!(p.availableProperties() & QMqttPublishProperties::ResponseTopic));
    QCOMPARE(p.responseTopic(), QString());
    p.setResponseTopic(responseTopic);
    QVERIFY(p.availableProperties() & QMqttPublishProperties::ResponseTopic);
    QCOMPARE(p.responseTopic(), responseTopic);

    const QByteArray data = QByteArray(1, char('c'));
    QVERIFY(!(p.availableProperties() & QMqttPublishProperties::CorrelationData));
    QCOMPARE(p.correlationData(), QByteArray());
    p.setCorrelationData(data);
    QVERIFY(p.availableProperties() & QMqttPublishProperties::CorrelationData);
    QCOMPARE(p.correlationData(), data);

    const QString userKey = QLatin1String("UserName");
    const QString userValue = QLatin1String("SomeValue");
    QMqttUserProperties userProperty;
    QVERIFY(!(p.availableProperties() & QMqttPublishProperties::UserProperty));
    QCOMPARE(p.userProperties(), userProperty);
    userProperty.append(QMqttStringPair(userKey, userValue));
    p.setUserProperties(userProperty);
    QVERIFY(p.availableProperties() & QMqttPublishProperties::UserProperty);
    QCOMPARE(p.userProperties(), userProperty);

    QVERIFY(!(p.availableProperties() & QMqttPublishProperties::SubscriptionIdentifier));
    const QList<quint32> one{1};
    p.setSubscriptionIdentifiers(one);
    QCOMPARE(p.subscriptionIdentifiers(), one);

    const QList<quint32> invalidZero{1, 0};
    p.setSubscriptionIdentifiers(invalidZero);
    QVERIFY(p.availableProperties() & QMqttPublishProperties::SubscriptionIdentifier);
    QCOMPARE(p.subscriptionIdentifiers(), one);

    const QString contentType = QLatin1String("MultimediaContent123");
    QVERIFY(!(p.availableProperties() & QMqttPublishProperties::ContentType));
    QCOMPARE(p.contentType(), QString());
    p.setContentType(contentType);
    QVERIFY(p.availableProperties() & QMqttPublishProperties::ContentType);
    QCOMPARE(p.contentType(), contentType);
}

void tst_QMqttPublishProperties::propertyConsistency()
{

    QMqttClient client;
    client.setProtocolVersion(QMqttClient::MQTT_5_0);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    QMqttClient client2;
    client2.setProtocolVersion(QMqttClient::MQTT_5_0);
    client2.setHostname(m_testBroker);
    client2.setPort(m_port);

    client2.connectToHost();
    QTRY_VERIFY2(client2.state() == QMqttClient::Connected, "Could not connect to broker.");


    const QByteArray correlation(10, 'c');
    const QString responseTopic = QLatin1String("topic/to/reply");
    const QString userKey1 = QLatin1String("UserName1");
    const QString userValue1 = QLatin1String("SomeValue");
    const QString userKey2 = QLatin1String("UserName2");
    const QString userValue2 = QLatin1String("OtherValue");
    QMqttUserProperties userProperty;
    userProperty.append(QMqttStringPair(userKey1, userValue1));
    userProperty.append(QMqttStringPair(userKey2, userValue2));
    const QString content = QLatin1String("ContentType");

    const QString testTopic = QLatin1String("Qt/PublishProperties/publish/consistent");

    auto sub = client2.subscribe(testTopic, 1);
    QTRY_COMPARE(sub->state(), QMqttSubscription::Subscribed);

    QMqttPublishProperties pubProp;
    pubProp.setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator::UTF8Encoded);
    pubProp.setMessageExpiryInterval(60);
    //pubProp.setTopicAlias(1);
    pubProp.setResponseTopic(responseTopic);
    pubProp.setCorrelationData(correlation);
    pubProp.setUserProperties(userProperty);
    pubProp.setContentType(content);

    QSignalSpy publishSpy(&client, SIGNAL(messageSent(qint32)));
    QSignalSpy subscribeSpy(sub, SIGNAL(messageReceived(QMqttMessage)));
    client.publish(testTopic, pubProp, QByteArray("Some Content"), 1);

    QTRY_COMPARE(publishSpy.size(), 1);
    QTRY_COMPARE(subscribeSpy.size(), 1);

    auto msg = subscribeSpy.at(0).at(0).value<QMqttMessage>();

    QCOMPARE(msg.payload(), QByteArray("Some Content"));
    QMqttPublishProperties receivalProp = msg.publishProperties();
    QCOMPARE(pubProp.payloadFormatIndicator(), receivalProp.payloadFormatIndicator());
    QVERIFY(pubProp.messageExpiryInterval() >= receivalProp.messageExpiryInterval()); // The broker MIGHT reduce
    QCOMPARE(pubProp.responseTopic(), receivalProp.responseTopic());
    QCOMPARE(pubProp.correlationData(), receivalProp.correlationData());

    // Paho test server sends identical userProperties, Flespi adds additional properties (ie timestamp)
    const auto userSend = pubProp.userProperties();
    const auto userReceive = receivalProp.userProperties();
    QVERIFY(userSend.size() <= userReceive.size());
    for (auto it = userSend.constBegin(); it != userSend.constEnd(); ++it)
        QVERIFY(userReceive.contains(*it));

    QCOMPARE(pubProp.contentType(), receivalProp.contentType());
}

void tst_QMqttPublishProperties::topicAlias()
{
    // Get serverproperties
    // Send data with higher alias than available
    // Connection gets killed
}

QTEST_MAIN(tst_QMqttPublishProperties)

#include "tst_qmqttpublishproperties.moc"
