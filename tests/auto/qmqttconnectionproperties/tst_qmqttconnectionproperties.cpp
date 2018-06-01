/******************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

    QCOMPARE(p.sessionExpiryInterval(), 0);
    p.setSessionExpiryInterval(1000);
    QCOMPARE(p.sessionExpiryInterval(), 1000);

    QCOMPARE(p.maximumPacketSize(), std::numeric_limits<quint32>::max());
    p.setMaximumPacketSize(0);
    QVERIFY(p.maximumPacketSize() != 0);
    p.setMaximumPacketSize(500);
    QCOMPARE(p.maximumPacketSize(), 500);

    QCOMPARE(p.maximumReceive(), 65535);
    p.setMaximumReceive(0);
    QVERIFY(p.maximumReceive() != 0);
    p.setMaximumReceive(30);
    QCOMPARE(p.maximumReceive(), 30);

    QCOMPARE(p.maximumTopicAlias(), 0);
    p.setMaximumTopicAlias(5);
    QCOMPARE(p.maximumTopicAlias(), 5);

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
    QVERIFY(properties != 0);

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
        qDebug() << "  ReasonString:" << server.reasonString();
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
            qWarning("Server accepts less data than required for this test.");
        }
    } else {
        qDebug() << "Server has no max packet size specified";
    }

    const QString topic = QLatin1String("some/Topic/maxSize");

    auto sub = client.subscribe(topic, 1);
    QTRY_COMPARE(sub->state(), QMqttSubscription::Subscribed);

    QSignalSpy subscribeSpy(sub, SIGNAL(messageReceived(QMqttMessage)));

    QByteArray shortData(100, 'd');
    QByteArray overFlowData(1000, 'o');

    QSignalSpy publishSpy(&client, SIGNAL(messageSent(qint32)));
    client.publish(topic, shortData, 1);
    QTRY_COMPARE(publishSpy.count(), 1);

    QTRY_COMPARE(subscribeSpy.count(), 1);

    client.publish(topic, overFlowData, 1);
    QTRY_COMPARE(publishSpy.count(), 1);

    // We defined maximum size to receive is 500, hence the message should not be sent back
    // to us. Wait for some time and verify no message got sent to subscriber
    QTest::qWait(3000);
    QTRY_COMPARE(subscribeSpy.count(), 1);
}

void tst_QMqttConnectionProperties::maximumTopicAlias()
{
    // ### TODO: implement
    QSKIP("Needs to be implemented.");
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
