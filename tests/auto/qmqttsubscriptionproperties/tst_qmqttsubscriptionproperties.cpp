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
#include <QtMqtt/QMqttSubscriptionProperties>

class tst_QMqttSubscriptionProperties : public QObject
{
    Q_OBJECT

public:
    tst_QMqttSubscriptionProperties();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void getSet();
    void subscribe();

private:
    QProcess m_brokerProcess;
    QString m_testBroker;
    quint16 m_port{1883};
};

tst_QMqttSubscriptionProperties::tst_QMqttSubscriptionProperties()
{
}

void tst_QMqttSubscriptionProperties::initTestCase()
{
    m_testBroker = invokeOrInitializeBroker(&m_brokerProcess);
    if (m_testBroker.isEmpty())
        qFatal("No MQTT broker present to test against.");
}

void tst_QMqttSubscriptionProperties::cleanupTestCase()
{
}

void tst_QMqttSubscriptionProperties::getSet()
{
    QMqttSubscriptionProperties properties;

    const quint32 id = 123;
    properties.setSubscriptionIdentifier(id);
    QCOMPARE(properties.subscriptionIdentifier(), id);

    const QString userKey1 = QLatin1String("UserName1");
    const QString userValue1 = QLatin1String("SomeValue");
    const QString userKey2 = QLatin1String("UserName2");
    const QString userValue2 = QLatin1String("OtherValue");
    QMqttUserProperties userProperty;
    QCOMPARE(properties.userProperties(), userProperty);
    userProperty.append(QMqttStringPair(userKey1, userValue1));
    userProperty.append(QMqttStringPair(userKey2, userValue2));
    properties.setUserProperties(userProperty);
    QCOMPARE(properties.userProperties(), userProperty);
}

void tst_QMqttSubscriptionProperties::subscribe()
{
    QMqttClient client;
    client.setProtocolVersion(QMqttClient::MQTT_5_0);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    QMqttConnectionProperties conProperties;
    conProperties.setRequestResponseInformation(true);
    conProperties.setRequestProblemInformation(true);
    client.setConnectionProperties(conProperties);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    const QString topic = QLatin1String("sub/props");

    QMqttSubscriptionProperties properties;
    properties.setSubscriptionIdentifier(10);
    const QString userKey1 = QLatin1String("UserName1");
    const QString userValue1 = QLatin1String("SomeValue");
    const QString userKey2 = QLatin1String("UserName2");
    const QString userValue2 = QLatin1String("OtherValue");
    QMqttUserProperties userProperty;
    userProperty.append(QMqttStringPair(userKey1, userValue1));
    userProperty.append(QMqttStringPair(userKey2, userValue2));

    auto sub = client.subscribe(topic, properties, 1);
    QTRY_COMPARE(sub->state(), QMqttSubscription::Subscribed);

    // ### TODO: Try to create a subscription which generates a reason code
    // and/or user properties.
}

QTEST_MAIN(tst_QMqttSubscriptionProperties)

#include "tst_qmqttsubscriptionproperties.moc"
