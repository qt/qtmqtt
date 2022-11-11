// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "broker_connection.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttConnectionProperties>
#include <QtMqtt/QMqttSubscription>
#include <QtMqtt/private/qmqttconnectionproperties_p.h>

class tst_QMqttConnectionProperties : public QObject
{
    Q_OBJECT

public:
    tst_QMqttConnectionProperties();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void getSet();
    void receiveServerProperties();
    void maximumPacketSize();
    void maximumTopicAlias();
    void maximumTopicAliasReceive();
    void assignedClientId();
    void userProperties();
private:
    QProcess m_brokerProcess;
    QString m_testBroker;
    quint16 m_port{1883};
};

tst_QMqttConnectionProperties::tst_QMqttConnectionProperties()
{
}

void tst_QMqttConnectionProperties::initTestCase()
{
    m_testBroker = invokeOrInitializeBroker(&m_brokerProcess);
    if (m_testBroker.isEmpty())
        qFatal("No MQTT broker present to test against.");
}

void tst_QMqttConnectionProperties::cleanupTestCase()
{
}

void tst_QMqttConnectionProperties::getSet()
{
    QMqttConnectionProperties p;

    QVERIFY(p.userProperties().isEmpty());
    QMqttUserProperties properties;
    properties.append(QMqttStringPair(QLatin1String("someKey"), QLatin1String("someValue")));
    p.setUserProperties(properties);
    QCOMPARE(p.userProperties(), properties);

    QVERIFY(p.authenticationMethod().isEmpty());
    const QLatin1String authMethod("SomeAuthentication");
    p.setAuthenticationMethod(authMethod);
    QCOMPARE(p.authenticationMethod(), authMethod);

    QVERIFY(p.authenticationData().isEmpty());
    const QByteArray authData("AuthData123");
    p.setAuthenticationData(authData);
    QCOMPARE(p.authenticationData(), authData);

    QCOMPARE(p.sessionExpiryInterval(), 0u);
    p.setSessionExpiryInterval(1000);
    QCOMPARE(p.sessionExpiryInterval(), 1000u);

    QCOMPARE(p.maximumPacketSize(), std::numeric_limits<quint32>::max());
    p.setMaximumPacketSize(0);
    QVERIFY(p.maximumPacketSize() != 0u);
    p.setMaximumPacketSize(500);
    QCOMPARE(p.maximumPacketSize(), 500u);

    QCOMPARE(p.maximumReceive(), 65535);
    p.setMaximumReceive(0);
    QVERIFY(p.maximumReceive() != 0u);
    p.setMaximumReceive(30);
    QCOMPARE(p.maximumReceive(), 30u);

    QCOMPARE(p.maximumTopicAlias(), 0u);
    p.setMaximumTopicAlias(5);
    QCOMPARE(p.maximumTopicAlias(), 5u);

    QCOMPARE(p.requestResponseInformation(), false);
    p.setRequestResponseInformation(true);
    QCOMPARE(p.requestResponseInformation(), true);

    QCOMPARE(p.requestProblemInformation(), true);
    p.setRequestProblemInformation(false);
    QCOMPARE(p.requestProblemInformation(), false);
}

void tst_QMqttConnectionProperties::receiveServerProperties()
{
    QMqttClient client;
    client.setProtocolVersion(QMqttClient::MQTT_5_0);
    QMqttServerConnectionProperties server = client.serverConnectionProperties();

    QCOMPARE(server.isValid(), false);

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    server = client.serverConnectionProperties();

    QCOMPARE(server.isValid(), true);

    QMqttServerConnectionProperties::ServerPropertyDetails properties = server.availableProperties();
    qDebug() << "Specified properties:" << properties;

    if (properties & QMqttServerConnectionProperties::SessionExpiryInterval)
        qDebug() << "  SessionExpiryInterval:" << server.sessionExpiryInterval();
    if (properties & QMqttServerConnectionProperties::MaximumReceive)
        qDebug() << "  MaximumReceive:" << server.maximumReceive();
    if (properties & QMqttServerConnectionProperties::MaximumQoS)
        qDebug() << "  MaximumQoS:" << server.maximumQoS();
    if (properties & QMqttServerConnectionProperties::RetainAvailable)
        qDebug() << "  RetainAvailable:" << server.retainAvailable();
    if (properties & QMqttServerConnectionProperties::MaximumPacketSize)
        qDebug() << "  MaximumPacketSize:" << server.maximumPacketSize();
    if (properties & QMqttServerConnectionProperties::AssignedClientId)
        qDebug() << "  AssignedClientId:" << server.clientIdAssigned();
    if (properties & QMqttServerConnectionProperties::MaximumTopicAlias)
        qDebug() << "  MaximumTopicAlias:" << server.maximumTopicAlias();
    if (properties & QMqttServerConnectionProperties::ReasonString)
        qDebug() << "  ReasonString:" << server.reason();
    if (properties & QMqttServerConnectionProperties::UserProperty)
        qDebug() << "  UserProperty:" << server.userProperties();
    if (properties & QMqttServerConnectionProperties::WildCardSupported)
        qDebug() << "  WildCard Support:" << server.wildcardSupported();
    if (properties & QMqttServerConnectionProperties::SubscriptionIdentifierSupport)
        qDebug() << "  Subscription Identifier Support:" << server.subscriptionIdentifierSupported();
    if (properties & QMqttServerConnectionProperties::SharedSubscriptionSupport)
        qDebug() << "  Shared Subscription Support:" << server.sharedSubscriptionSupported();
    if (properties & QMqttServerConnectionProperties::ServerKeepAlive)
        qDebug() << "  Server KeepAlive:" << server.serverKeepAlive();
    if (properties & QMqttServerConnectionProperties::ResponseInformation)
        qDebug() << "  ResponseInformation:" << server.responseInformation();
    if (properties & QMqttServerConnectionProperties::ServerReference)
        qDebug() << "  Server Reference:" << server.serverReference();
    if (properties & QMqttServerConnectionProperties::AuthenticationMethod)
        qDebug() << "  AuthenticationMethod:" << server.authenticationMethod();
    if (properties & QMqttServerConnectionProperties::AuthenticationData)
        qDebug() << "  AuthenticationData:" << server.authenticationData();
}

void tst_QMqttConnectionProperties::maximumPacketSize()
{
    QMqttClient client;
    client.setProtocolVersion(QMqttClient::MQTT_5_0);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    QMqttConnectionProperties props;
    props.setMaximumPacketSize(500);
    client.setConnectionProperties(props);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    QMqttServerConnectionProperties serverProperties = client.serverConnectionProperties();
    if (serverProperties.availableProperties() & QMqttServerConnectionProperties::MaximumPacketSize) {
        if (serverProperties.maximumPacketSize() < props.maximumPacketSize()) {
            qDebug() << "Server accepts less data than required for this test.";
        }
    } else {
        QSKIP("Server has no max packet size defined. Default is unlimited.");
    }

    const QString topic = QLatin1String("Qt/ConnectionProperties/some/Topic/maxSize");

    auto sub = client.subscribe(topic, 1);
    QTRY_COMPARE(sub->state(), QMqttSubscription::Subscribed);

    QSignalSpy subscribeSpy(sub, SIGNAL(messageReceived(QMqttMessage)));

    QByteArray shortData(100, 'd');
    QByteArray overFlowData(1000, 'o');

    QSignalSpy publishSpy(&client, SIGNAL(messageSent(qint32)));
    client.publish(topic, shortData, 1);
    QTRY_COMPARE(publishSpy.size(), 1);

    QTRY_COMPARE(subscribeSpy.size(), 1);

    client.publish(topic, overFlowData, 1);
    QTRY_COMPARE(publishSpy.size(), 1);

    // We defined maximum size to receive is 500, hence the message should not be sent back
    // to us. Wait for some time and verify no message got sent to subscriber
    QTest::qWait(3000);
    QTRY_COMPARE(subscribeSpy.size(), 1);
}

void tst_QMqttConnectionProperties::maximumTopicAlias()
{
    const QByteArray msgContent("SomeContent");
    QMqttClient client;
    client.setProtocolVersion(QMqttClient::MQTT_5_0);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    const int userMaxTopicAlias = 20;
    QMqttConnectionProperties userProperties;
    userProperties.setMaximumTopicAlias(userMaxTopicAlias);
    client.setConnectionProperties(userProperties);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    int publishTransportSize = 0;
    auto transport = client.transport();
    QSignalSpy transportSpy(transport, SIGNAL(bytesWritten(qint64))); //&QIODevice::bytesWritten);

    const auto serverProperties = client.serverConnectionProperties();
    const quint16 serverMaxAlias = serverProperties.maximumTopicAlias();
    if (serverMaxAlias == 0)
        QSKIP("Need to skip this test due to topic aliases not supported on server");

    if (serverMaxAlias > userMaxTopicAlias)
        QSKIP("This test requires that the client contains more topic aliases than the server");

    //qDebug() << "Server Max Alias:" << serverMaxAlias;
    //QLoggingCategory::setFilterRules("qt.mqtt.connection*=true");

    // Fill up the internal publish vector
    const QLatin1String topicBase("Qt/connprop/alias/top");
    for (quint16 i = 0; i < serverMaxAlias; ++i) {
        QSignalSpy publishSpy(&client, SIGNAL(messageSent(qint32)));

        QMqttTopicName topic(topicBase + QString::number(i));
        client.publish(topic, msgContent, 1);
        QTRY_VERIFY(publishSpy.size() == 1);

        QVERIFY(transportSpy.size() == 1);
        const int dataSize = transportSpy.at(0).at(0).toInt();
        if (publishTransportSize == 0) {
            publishTransportSize = dataSize;
        } else {
            QCOMPARE(dataSize, publishTransportSize);
        }
        transportSpy.clear();
    }

    // Verify non auto assignable defaults to old behavior
    QSignalSpy fullSpy(&client, SIGNAL(messageSent(qint32)));
    transportSpy.clear();
    client.publish(QLatin1String("Qt/connprop/alias/full/with/long/topic/to/verify/bigger/size"), msgContent, 1);
    QTRY_VERIFY(fullSpy.size() == 1);
    QVERIFY(transportSpy.size() == 1);
    QVERIFY(transportSpy.at(0).at(0).toInt() > publishTransportSize);

    // Verify alias is used at sending second time
    transportSpy.clear();
    fullSpy.clear();

    client.publish(topicBase + QLatin1String("0"), msgContent, 1);
    QTRY_VERIFY(fullSpy.size() == 1);
    QVERIFY(transportSpy.size() == 1);
    int usageSize = transportSpy.at(0).at(0).toInt();
    QVERIFY(usageSize < publishTransportSize);

    // Manually overwrite topic alias 1
    const QMqttTopicName overwrite(QLatin1String("Qt/connprop/alias/overwrite/with/long/topic/to/verify/reset"));
    QMqttPublishProperties overProp;
    overProp.setTopicAlias(2);
    fullSpy.clear();
    transportSpy.clear();
    client.publish(overwrite, overProp, msgContent, 1);
    QTRY_VERIFY(fullSpy.size() == 1);
    QVERIFY(transportSpy.size() == 1);
    const int overwriteSize = transportSpy.at(0).at(0).toInt();
    QVERIFY(overwriteSize > publishTransportSize);
    // After resend new alias should be used and msg size reduced
    fullSpy.clear();
    transportSpy.clear();
    client.publish(overwrite, msgContent, 1);
    QTRY_VERIFY(fullSpy.size() == 1);
    QVERIFY(transportSpy.size() == 1);
    usageSize = transportSpy.at(0).at(0).toInt();
    QVERIFY(usageSize < overwriteSize);
}

void createTopicAliasClient(QMqttClient &client, const QString &hostname, quint16 port)
{
    client.setProtocolVersion(QMqttClient::MQTT_5_0);
    client.setHostname(hostname);
    client.setPort(port);

    const int userMaxTopicAlias = 9;
    QMqttConnectionProperties userProperties;
    userProperties.setMaximumTopicAlias(userMaxTopicAlias);
    client.setConnectionProperties(userProperties);
}

void tst_QMqttConnectionProperties::maximumTopicAliasReceive()
{
    const QByteArray msgContent("SomeContent");
    const QString topic("Qt/connprop/receive/alias");

    QMqttClient client;
    createTopicAliasClient(client, m_testBroker, m_port);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    QMqttClient subscriber;
    createTopicAliasClient(subscriber, m_testBroker, m_port);

    subscriber.connectToHost();
    QTRY_VERIFY2(subscriber.state() == QMqttClient::Connected, "Could not connect to broker.");

    auto sub = subscriber.subscribe(topic + "/#", 1);

    int receiveCounter = 0;
    connect(sub, &QMqttSubscription::messageReceived, [&receiveCounter](QMqttMessage msg) {
        qDebug() << "Received message with alias:" << msg.publishProperties().topicAlias();
        receiveCounter++;
    });

    QTRY_VERIFY2(sub->state() == QMqttSubscription::Subscribed, "Could not subscribe");

    QSignalSpy publishSpy(&client, &QMqttClient::messageSent);
    //QLoggingCategory::setFilterRules("qt.mqtt.connection*=true");
    client.publish(topic, msgContent, 1);
    QTRY_VERIFY2(publishSpy.size() == 1, "Could not publish");
    QTRY_VERIFY2(receiveCounter == 1, "Did not receive initial message");

    publishSpy.clear();
    client.publish(topic, msgContent, 1);
    QTRY_VERIFY2(publishSpy.size() == 1, "Could not publish");
    QTRY_VERIFY2(receiveCounter == 2, "Did not receive second aliases message");
}

void tst_QMqttConnectionProperties::assignedClientId()
{
    QMqttClient client;
    client.setProtocolVersion(QMqttClient::MQTT_5_0);
    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.setClientId(QLatin1String(""));

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    QMqttServerConnectionProperties serverProperties = client.serverConnectionProperties();
    QVERIFY2(serverProperties.availableProperties() & QMqttServerConnectionProperties::AssignedClientId, "Must contain client ID");

    QVERIFY2(!client.clientId().isEmpty(), "Client ID must not be empty");
}

void tst_QMqttConnectionProperties::userProperties()
{
    QMqttClient client;
    client.setProtocolVersion(QMqttClient::MQTT_5_0);
    client.setHostname(m_testBroker);
    client.setPort(m_port);
    QMqttConnectionProperties p;
    QMqttUserProperties userProperties;
    userProperties.append(QMqttStringPair(QLatin1String("TestKey"), QLatin1String("TestValue")));
    p.setUserProperties(userProperties);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    // ### TODO: We should a method to verify transmission of the user data
    // Probably in conjunction with authentication

    //QMqttServerConnectionProperties serverProperties = client.serverConnectionProperties();
    //if (serverProperties.availableProperties() & QMqttServerConnectionProperties::UserProperty) {
    //    auto serverUserProps = serverProperties.userProperties();
    //    qDebug() << serverUserProps;
    //} else
    //    qDebug() << "No user props available";
}

QTEST_MAIN(tst_QMqttConnectionProperties)

#include "tst_qmqttconnectionproperties.moc"
