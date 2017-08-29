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

    QSignalSpy receivalSpy(sub.data(), SIGNAL(messageReceived(QMqttMessage)));

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

QTEST_MAIN(Tst_QMqttSubscription)

#include "tst_qmqttsubscription.moc"
