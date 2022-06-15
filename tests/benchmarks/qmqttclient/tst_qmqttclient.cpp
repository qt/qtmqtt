// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "broker_connection.h"

#include <QtCore/QString>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtMqtt/QMqttClient>

class Tst_QMqttClient : public QObject
{
    Q_OBJECT

public:
    Tst_QMqttClient();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void stressTest_data();
    void stressTest();
    void stressTest2_data();
    void stressTest2();
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

void Tst_QMqttClient::stressTest_data()
{
    QTest::addColumn<int>("qos");
    QTest::newRow("qos0") << 0;
    QTest::newRow("qos1") << 1;
    QTest::newRow("qos2") << 2;
}

void Tst_QMqttClient::stressTest()
{
    QFETCH(int, qos);

    QMqttClient subscriber;
    QMqttClient publisher;

    subscriber.setHostname(m_testBroker);
    subscriber.setPort(m_port);
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    quint64 messageCount = 0;
    QMqttSubscription *subscription;
    const QString testTopic = QLatin1String("Qt/benchmark/test/topic");

    connect(&subscriber, &QMqttClient::connected, [&](){
        subscription = subscriber.subscribe(testTopic);
        if (!subscription)
            qFatal("Failed to create subscription");

        connect(subscription, &QMqttSubscription::messageReceived, [&](QMqttMessage) {
            messageCount++;
            publisher.publish(testTopic, QByteArray("some message"), qos);
        });
        publisher.connectToHost();
    });
    subscriber.connectToHost();

    connect(&publisher, &QMqttClient::connected, [&]() {
        publisher.publish(testTopic, QByteArray("initial message"), qos);
    });

    QTest::qWait(30000);
    qDebug() << "Stress test result for QoS " << qos << ":" << messageCount;
}

void Tst_QMqttClient::stressTest2_data()
{
    QTest::addColumn<int>("qos");
    QTest::addColumn<int>("msgCount");
    QTest::newRow("1/100") << 1 << 100;
    QTest::newRow("1/1000") << 1 << 1000;
    // Disable to avoid timeout
    //QTest::newRow("1/10000") << 1 << 10000;
    QTest::newRow("2/100") << 2 << 100;
    QTest::newRow("2/1000") << 2 << 1000;
    // Disabled as mosquitto is not able to handle this many message
    // QTest::newRow("2/10000") << 2 << 10000;
}

void Tst_QMqttClient::stressTest2()
{
    QFETCH(int, qos);
    QFETCH(int, msgCount);

    QSet<qint32> msgIds;
    msgIds.reserve(msgCount);

    QMqttClient publisher;
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    connect(&publisher, &QMqttClient::messageSent, [&msgIds](qint32 id) {
        QVERIFY2(msgIds.contains(id), "Received messageSent for unknown id");
        msgIds.remove(id);
    });

    QSignalSpy spy(&publisher, SIGNAL(connected()));
    publisher.connectToHost();
    QTRY_COMPARE(spy.count(), 1);

    const QString topic = QLatin1String("Qt/benchmark/SomeTopic/Sub");
    const QByteArray message("messageContent");

    for (qint32 i = 0; i < msgCount; ++i) {
        QSignalSpy writeSpy(publisher.transport(), SIGNAL(bytesWritten(qint64)));
        const qint32 id = publisher.publish(topic, message, qos);
        QTRY_VERIFY2(id != -1, "Could not publish message");
        msgIds.insert(id);
        QTRY_VERIFY(writeSpy.count() >= 1);
    }

    // Give some extra time depending on connection
    if (msgCount > 1000)
        QTest::qWait(5000);
    QTRY_COMPARE(msgIds.count(), 0);

    publisher.disconnectFromHost();
}

QTEST_MAIN(Tst_QMqttClient)

#include "tst_qmqttclient.moc"
