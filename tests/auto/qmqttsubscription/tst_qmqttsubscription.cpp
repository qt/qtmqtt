/******************************************************************************
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
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttSubscription>

class Tst_QMqttSubscription : public QObject
{
    Q_OBJECT

public:
    Tst_QMqttSubscription();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void getSetCheck();
    void wildCards_data();
    void wildCards();
    void reconnect();
private:
    QProcess m_brokerProcess;
    QString m_testBroker;
    quint16 m_port{1883};
};

Tst_QMqttSubscription::Tst_QMqttSubscription()
{
}

void Tst_QMqttSubscription::initTestCase()
{
    m_testBroker = invokeOrInitializeBroker(&m_brokerProcess);
    if (m_testBroker.isEmpty())
        qFatal("No MQTT broker present to test against.");
}

void Tst_QMqttSubscription::cleanupTestCase()
{
}

void Tst_QMqttSubscription::getSetCheck()
{
}

void Tst_QMqttSubscription::wildCards_data()
{
    QTest::addColumn<QString>("subscription");
    QTest::addColumn<int>("expectedReceival");

    QTest::newRow("#") << "Qt/#" << 6;
    QTest::newRow("Qt/a/b/c/d/e/f") << "Qt/a/b/c/d/e/f" << 1;
    QTest::newRow("Qt/+/b/c/d/e/f") << "Qt/+/b/c/d/e/f" << 1;
    QTest::newRow("Qt/a/+/c/d/e/f") << "Qt/a/+/c/d/e/f" << 1;
    QTest::newRow("Qt/a/b/+/d/e/f") << "Qt/a/b/+/d/e/f" << 1;
    QTest::newRow("Qt/a/b/c/+/e/f") << "Qt/a/b/c/+/e/f" << 1;
    QTest::newRow("Qt/a/b/c/d/+/f") << "Qt/a/b/c/d/+/f" << 1;
    QTest::newRow("Qt/a/b/c/d/e/+") << "Qt/a/b/c/d/e/+" << 1;
    QTest::newRow("Qt/+/b/+/d/e/+") << "Qt/+/b/+/d/e/+" << 1;
    QTest::newRow("Qt/a/+") << "Qt/a/+" << 1;
    QTest::newRow("Qt/a/+/c") << "Qt/a/+/c" << 1;
}

void Tst_QMqttSubscription::wildCards()
{
    QFETCH(QString, subscription);
    QFETCH(int, expectedReceival);

    QMqttClient client;

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    QMqttClient publisher;
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);
    publisher.connectToHost();
    QTRY_VERIFY2(publisher.state() == QMqttClient::Connected, "Could not connect to broker.");

    auto sub = client.subscribe(subscription, 1);
    QTRY_VERIFY2(sub->state() == QMqttSubscription::Subscribed, "Could not subscribe to topic.");

    QSignalSpy receivalSpy(sub, SIGNAL(messageReceived(QMqttMessage)));

    QStringList topics;
    topics << "Qt/a"
           << "Qt/a/b"
           << "Qt/a/b/c"
           << "Qt/a/b/c/d"
           << "Qt/a/b/c/d/e"
           << "Qt/a/b/c/d/e/f";

    for (auto t : topics) {
        QSignalSpy spy(&publisher, SIGNAL(messageSent(qint32)));
        publisher.publish(t, "Some arbitrary message", 1);
        QTRY_VERIFY2(spy.size() == 1, "Could not publish message.");
    }

    if (expectedReceival > 0) {
        QTRY_VERIFY2(receivalSpy.size() == expectedReceival, "Did not receive sufficient messages.");
        // So far we only tested we got enough, now verify we do not get too many
        QTest::qWait(1000);
        QVERIFY2(receivalSpy.size() == expectedReceival, "Received too many messages.");
    }

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect.");

    publisher.disconnectFromHost();
    QTRY_VERIFY2(publisher.state() == QMqttClient::Disconnected, "Could not disconnect.");
}

void Tst_QMqttSubscription::reconnect()
{
    // QTBUG-64042
    //    - Connect with clean session
    QMqttClient client;

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.setCleanSession(true);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    //    - Subscribe to topic A
    const QString subscription("topics/resub");
    auto sub = client.subscribe(subscription, 1);
    QTRY_VERIFY2(sub->state() == QMqttSubscription::Subscribed, "Could not subscribe to topic.");

    //    - Loose connection / connection drop
    QAbstractSocket *transport = qobject_cast<QAbstractSocket *>(client.transport());
    QVERIFY2(transport, "Transport has to be QAbstractSocket-based.");
    transport->disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "State not correctly switched.");

    //    - Reconnect (keeping cleansession)
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");
    // old subs should get updated / invalidated
    QCOMPARE(sub->state(), QMqttSubscription::Unsubscribed);

    //    - Resubscribe
    auto reSub = client.subscribe(subscription, 1);
    QTRY_VERIFY2(reSub->state() == QMqttSubscription::Subscribed, "Could not re-subscribe to topic.");
    QSignalSpy receivalSpy(reSub, SIGNAL(messageReceived(QMqttMessage)));

    QSignalSpy pubSpy(&client, SIGNAL(messageSent(qint32)));
    client.publish(subscription, "Sending after reconnect 1", 1);
    QTRY_VERIFY2(pubSpy.size() == 1, "Could not publish message.");

    QTRY_VERIFY2(receivalSpy.size() == 1, "Did not receive message on re-subscribe.");

    //    - Loose connection / connection drop
    transport = qobject_cast<QAbstractSocket *>(client.transport());
    QVERIFY2(transport, "Transport has to be QAbstractSocket-based.");
    transport->disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "State not correctly switched.");

    //    - Reconnect (no cleansession)
    QSignalSpy restoredSpy(&client, SIGNAL(brokerSessionRestored()));

    client.setCleanSession(false);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");
    // Could not identify a broker doing this. The specs states:
    // [MQTT-4.1.0-1] The Client and Server MUST store Session state for the entire
    //                duration of the Session.
    // [MQTT-4.1.0-2] A Session MUST last at least as long it has an active Network Connection.
    // All testbrokers delete the session at transport disconnect, regardless of DISCONNECT been
    // send before or not.
    if (restoredSpy.count() > 0) {
        QCOMPARE(reSub->state(), QMqttSubscription::Subscribed);
        pubSpy.clear();
        receivalSpy.clear();
        client.publish(subscription, "Sending after reconnect 2", 1);
        QTRY_VERIFY2(pubSpy.size() == 1, "Could not publish message.");
        QTRY_VERIFY2(receivalSpy.size() == 1, "Did not receive message on re-subscribe.");
    } else {
        // No need to test this
        qDebug() << "Test broker does not support long-livety sessions.";
    }
    //    - Old subscription is still active

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect.");
}

QTEST_MAIN(Tst_QMqttSubscription)

#include "tst_qmqttsubscription.moc"
