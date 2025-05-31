// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
    void reconnect_data();
    void reconnect();
    void sharedConnection();
    void sharedNonShared_data();
    void sharedNonShared();
    void noLocal_data();
    void noLocal();
    void qtbug_106203();
    void qtbug_104478();
    void qtbug_135653_connected();
    void qtbug_135653_not_connected();
    void qtbug_135653_not_unsubscribed();
private:
    template<typename Client>
    void createAndSubscribe(Client *c, QMqttSubscription **sub, const QString &topic);
    QProcess m_brokerProcess;
    QString m_testBroker;
    quint16 m_port{0};
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
    QTest::addColumn<QMqttClient::ProtocolVersion>("mqttVersion");
    QTest::addColumn<QString>("subscription");
    QTest::addColumn<int>("expectedReceival");

    QList<QMqttClient::ProtocolVersion> versions{QMqttClient::MQTT_3_1_1, QMqttClient::MQTT_5_0};

    for (int i = 0; i < 2; ++i) {
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/#")) << versions[i] << "Qt/subscription/#" << 6;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/a/b/c/d/e/f")) << versions[i] << "Qt/subscription/a/b/c/d/e/f" << 1;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/+/b/c/d/e/f")) << versions[i] << "Qt/subscription/+/b/c/d/e/f" << 1;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/a/+/c/d/e/f")) << versions[i] << "Qt/subscription/a/+/c/d/e/f" << 1;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/a/b/+/d/e/f")) << versions[i] << "Qt/subscription/a/b/+/d/e/f" << 1;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/a/b/c/+/e/f")) << versions[i] << "Qt/subscription/a/b/c/+/e/f" << 1;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/a/b/c/d/+/f")) << versions[i] << "Qt/subscription/a/b/c/d/+/f" << 1;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/a/b/c/d/e/+")) << versions[i] << "Qt/subscription/a/b/c/d/e/+" << 1;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/+/b/+/d/e/+")) << versions[i] << "Qt/subscription/+/b/+/d/e/+" << 1;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/a/+")) << versions[i] << "Qt/subscription/a/+" << 1;
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":Qt/subscription/a/+/c")) << versions[i] << "Qt/subscription/a/+/c" << 1;
    }
}

void Tst_QMqttSubscription::wildCards()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);
    QFETCH(QString, subscription);
    QFETCH(int, expectedReceival);

    VersionClient(mqttVersion, client);

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    VersionClient(mqttVersion, publisher);
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);
    publisher.connectToHost();
    QTRY_VERIFY2(publisher.state() == QMqttClient::Connected, "Could not connect to broker.");

    auto sub = client.subscribe(subscription, 1);
    QTRY_VERIFY2(sub->state() == QMqttSubscription::Subscribed, "Could not subscribe to topic.");

    QSignalSpy receivalSpy(sub, SIGNAL(messageReceived(QMqttMessage)));

    QStringList topics;
    topics << "Qt/subscription/a"
           << "Qt/subscription/a/b"
           << "Qt/subscription/a/b/c"
           << "Qt/subscription/a/b/c/d"
           << "Qt/subscription/a/b/c/d/e"
           << "Qt/subscription/a/b/c/d/e/f";

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

DefaultVersionTestData(Tst_QMqttSubscription::reconnect_data)

void Tst_QMqttSubscription::reconnect()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    // QTBUG-64042
    //    - Connect with clean session
    VersionClient(mqttVersion, client);

    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.setCleanSession(true);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    if (!client.isTcp() && !client.isSsl())
        QSKIP("This test only runs for tcp and ssl");

    //    - Subscribe to topic A
    const QString subscription("Qt/subscription/topics/resub");
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
    if (restoredSpy.size() > 0) {
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

template<typename Client>
void Tst_QMqttSubscription::createAndSubscribe(Client *c, QMqttSubscription **sub, const QString &topic)
{
    c->setProtocolVersion(QMqttClient::MQTT_5_0);
    c->setHostname(m_testBroker);
    c->setPort(m_port);

    c->connectToHost();
    QTRY_VERIFY2(c->state() == QMqttClient::Connected, "Could not connect to broker.");

    *sub = c->subscribe(topic, 1);
    QTRY_VERIFY2((*sub)->state() == QMqttSubscription::Subscribed, "Could not subscribe.");
}

void Tst_QMqttSubscription::sharedConnection()
{
    // Create / Connect publisher
    VersionClient(QMqttClient::MQTT_5_0, sender);
    sender.setHostname(m_testBroker);
    sender.setPort(m_port);
    sender.connectToHost();
    QTRY_VERIFY2(sender.state() == QMqttClient::Connected, "Could not connect to broker.");

    // Create GroupA
    const int groupSizeA = 2;
    const QString groupTopicA{QLatin1String("$share/groupA/shared/sub")};

    VersionClient(QMqttClient::MQTT_5_0, listenersA0);
    VersionClient(QMqttClient::MQTT_5_0, listenersA1);
    std::vector listenersA = {&listenersA0, &listenersA1};
    QMqttSubscription *subsA[groupSizeA];
    int messageCounterA[groupSizeA] = {0};
    int messageSumA = 0;
    // listenerAx: $share/groupA/Qt/Subscription/shared_check/#
    for (int i = 0; i < groupSizeA; ++i) {
        createAndSubscribe(listenersA[i], &subsA[i], groupTopicA);
        QCOMPARE(subsA[i]->isSharedSubscription(), true);
        QCOMPARE(subsA[i]->sharedSubscriptionName(), QLatin1String("groupA"));
        connect(subsA[i], &QMqttSubscription::messageReceived, [i, &messageCounterA, &messageSumA]() {
            messageCounterA[i]++;
            messageSumA++;
            //qDebug() << "A Got message:" << i << ":" << messageCounterA[i];
        });
    }

    // Create GroupB
    const int groupSizeB = 5;
    const QString groupTopicB{QLatin1String("$share/groupB/shared/#")};
    VersionClient(QMqttClient::MQTT_5_0, listenersB0);
    VersionClient(QMqttClient::MQTT_5_0, listenersB1);
    VersionClient(QMqttClient::MQTT_5_0, listenersB2);
    VersionClient(QMqttClient::MQTT_5_0, listenersB3);
    VersionClient(QMqttClient::MQTT_5_0, listenersB4);
    std::vector listenersB ={&listenersB0, &listenersB1, &listenersB2, &listenersB3, &listenersB4};
    QMqttSubscription *subsB[groupSizeB];
    int messageCounterB[groupSizeB] = {0};
    int messageSumB = 0;
    // listenerBx: $share/groupB/Qt/Subscription/shared_check/#
    for (int i = 0; i < groupSizeB; ++i) {
        createAndSubscribe(listenersB[i], &subsB[i], groupTopicB);
        QCOMPARE(subsB[i]->isSharedSubscription(), true);
        QCOMPARE(subsB[i]->sharedSubscriptionName(), QLatin1String("groupB"));
        connect(subsB[i], &QMqttSubscription::messageReceived, [i, &messageCounterB, &messageSumB]() {
            messageCounterB[i]++;
            messageSumB++;
            //qDebug() << "B Got message:" << i << ":" << messageCounterB[i];
        });
    }

    const int publishedMessages = 10;
    for (int i = 0; i < publishedMessages; ++i) {
        QSignalSpy publishSpy(&sender, SIGNAL(messageSent(qint32)));
        sender.publish(QLatin1String("shared/sub"), QByteArray("Foobidoo"), 1);
        QTRY_VERIFY2(publishSpy.size() == 1, "Could not publish message.");
    }

    QTRY_VERIFY2(messageSumA == publishedMessages, "Group A did not receive enough messages.");
    QTRY_VERIFY2(messageSumB == publishedMessages, "Group A did not receive enough messages.");

    QString formatString;
    for (int i = 0; i < groupSizeA; ++i) {
        formatString.append(QString::number(messageCounterA[i]));
        formatString.append(QLatin1Char(' '));
    }
    qDebug() << "Statistics GroupA : " << formatString;
    formatString.clear();
    for (int i = 0; i < groupSizeB; ++i) {
        formatString.append(QString::number(messageCounterB[i]));
        formatString.append(QLatin1Char(' '));
    }
    qDebug() << "Statistics GroupB : " << formatString;
}

void Tst_QMqttSubscription::sharedNonShared_data()
{
    const QString topic(QLatin1String("Qt/Subscription/SharedNonShared"));
    const QString groupTopic = QString::fromLatin1("$share/somegroup/") + topic;

    QTest::addColumn<QString>("topic1");
    QTest::addColumn<bool>("shared1");
    QTest::addColumn<QString>("topic2");
    QTest::addColumn<bool>("shared2");
    QTest::addColumn<bool>("expected");

    QTest::newRow("non - non") << topic << false << topic << false << true;
    QTest::newRow("non - yes") << topic << false << groupTopic << true << false;
    QTest::newRow("yes - non") << groupTopic << true << topic << false << false;
    QTest::newRow("yes - yes") << groupTopic << true << groupTopic << true << true;
}

void Tst_QMqttSubscription::sharedNonShared()
{
    QFETCH(QString, topic1);
    QFETCH(bool, shared1);
    QFETCH(QString, topic2);
    QFETCH(bool, shared2);
    QFETCH(bool, expected);

    // Create / Connect publisher
    VersionClient(QMqttClient::MQTT_5_0, client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    QMqttSubscription *sub1 = client.subscribe(topic1, 1);
    QCOMPARE(sub1->isSharedSubscription(), shared1);
    QVERIFY(sub1->sharedSubscriptionName().isEmpty() == !shared1);

    QMqttSubscription *sub2 = client.subscribe(topic2, 1);
    QCOMPARE(sub2->isSharedSubscription(), shared2);
    QVERIFY(sub2->sharedSubscriptionName().isEmpty() == !shared2);

    // Verify that a subscription is reused / not reused
    QCOMPARE(sub1 == sub2, expected);

    // Depending on the broker, it may decide to not send a message twice due to overlapping
    // subscriptions on the same client (eg. paho).
    // Using two different clients would make the receival work, but is not part of this test.

//    QSignalSpy receivalSpy1(sub1, SIGNAL(messageReceived(QMqttMessage)));
//    QSignalSpy receivalSpy2(sub2, SIGNAL(messageReceived(QMqttMessage)));
//    QSignalSpy publishSpy(&client, SIGNAL(messageSent(qint32)));

//    client.publish(QLatin1String("Qt/Subscription/SharedNonShared"), QByteArray(), 1);
//    QTRY_VERIFY(publishSpy.count() == 1);

//    // Verify both subscriptions receive the message
//    QTRY_VERIFY(receivalSpy1.count() == 1);
//    QTRY_VERIFY(receivalSpy2.count() == 1);
}

void Tst_QMqttSubscription::noLocal_data()
{
    QTest::addColumn<QMqttClient::ProtocolVersion>("version");
    QTest::addColumn<bool>("non");
    QTest::newRow("3.1.1 - false") << QMqttClient::MQTT_3_1_1 << false;
    QTest::newRow("3.1.1 - true") << QMqttClient::MQTT_3_1_1 << true;
    QTest::newRow("5.0 - false") << QMqttClient::MQTT_5_0 << false;
    QTest::newRow("5.0 - true") << QMqttClient::MQTT_5_0 << true;
}

void Tst_QMqttSubscription::noLocal()
{
    QFETCH(QMqttClient::ProtocolVersion, version);
    QFETCH(bool, non);

    VersionClient(version, client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.connectToHost();

    const auto clientVersion = client.protocolVersion();

    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    QMqttSubscriptionProperties subProps;
    subProps.setNoLocal(non);

    const QString topic(QLatin1String("Qt/Subscription/NonLoc"));

    auto sub = client.subscribe(topic, subProps, 1);
    QTRY_VERIFY2(sub->state() == QMqttSubscription::Subscribed, "Could not subscribe to topic.");

    QSignalSpy publishSpy(&client, SIGNAL(messageSent(qint32)));
    QSignalSpy receivalSpy(sub, SIGNAL(messageReceived(QMqttMessage)));

    client.publish(topic, "content", 1);
    QTRY_VERIFY(publishSpy.size() == 1);

    if (clientVersion == QMqttClient::MQTT_3_1_1 || !non) { // 3.1.1 does not know NoLocal and sends to subscription
        QTRY_VERIFY(receivalSpy.size() == 1);
    } else {
        QTest::qWait(3000);
        QCOMPARE(receivalSpy.size(), 0);
    }
}

void Tst_QMqttSubscription::qtbug_106203()
{
    const QString topic(QLatin1String("Qt/qtbug106203/Identity"));

    // Fill up sub-Topics with retained messages (ie connection state of devices)
    VersionClient(QMqttClient::MQTT_3_1_1, retainer);
    retainer.setHostname(m_testBroker);
    retainer.setPort(m_port);
    retainer.connectToHost();
    const int messageCount = 200;
    QTRY_VERIFY2(retainer.state() == QMqttClient::Connected, "Could not connect to broker.");
    for (int i = 0; i < messageCount; ++i) {
        QSignalSpy publishSpy(&retainer, SIGNAL(messageSent(qint32)));
        const QByteArray content = QString::fromLatin1("Content: %1").arg(i).toLocal8Bit();

        retainer.publish(topic + QString("/msg%1").arg(i), content, 1, true);
        QTRY_VERIFY(publishSpy.size() == 1);
    }

    retainer.disconnectFromHost();
    QTRY_VERIFY2(retainer.state() == QMqttClient::Disconnected, "Could not disconnect from broker.");

    VersionClient(QMqttClient::MQTT_3_1_1, client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    auto sub = client.subscribe(topic + QLatin1String("/#"), 1);
    QSignalSpy receiveSpy(sub, SIGNAL(messageReceived(QMqttMessage)));

    connect(sub, &QMqttSubscription::messageReceived, sub, [&client, topic](QMqttMessage msg) {
        // This can potentially cause relayout of internal structures
        auto subsub = client.subscribe(QString("/%1").arg(msg.payload()) + topic, 1);
        connect(subsub, &QMqttSubscription::messageReceived, &client, [](QMqttMessage) {
            QVERIFY2(false, "Second sub should never be reached");
        });
    });

    // We cannot use QTRY_ here as the bug is about receiving too many messages
    QTest::qWait(3000);
    QVERIFY2(receiveSpy.size() == messageCount, "Received invalid amount of messages.");

    sub->unsubscribe();
    QTRY_VERIFY2(sub->state() == QMqttSubscription::Unsubscribed, "Client could not unsubscribe.");

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect from broker.");
}

void Tst_QMqttSubscription::qtbug_104478()
{
    VersionClient(QMqttClient::MQTT_3_1_1, client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    auto singleWildcardSub = client.subscribe(QLatin1String("test/+/+/test2"));
    QTRY_VERIFY2(singleWildcardSub->state() == QMqttSubscription::Subscribed, "Could not subscribe to topic.");
    QSignalSpy singleSpy(singleWildcardSub, SIGNAL(messageReceived(QMqttMessage)));

    auto mixedWildcardSub = client.subscribe(QLatin1String("test/+/#"));
    QTRY_VERIFY2(mixedWildcardSub->state() == QMqttSubscription::Subscribed, "Could not subscribe to topic.");
    QSignalSpy mixedSpy(mixedWildcardSub, SIGNAL(messageReceived(QMqttMessage)));


    const QString topic(QLatin1String("test/foo/bar/test2"));

    QSignalSpy pubSpy(&client, SIGNAL(messageSent(qint32)));
    client.publish(topic, "Wildcard checks", 1);
    QTRY_VERIFY2(pubSpy.size() == 1, "Could not publish message.");

    QTRY_VERIFY2(singleSpy.count() >= 1, "Did not receive message");
    QTRY_VERIFY2(mixedSpy.count() >= 1, "Did not receive message");

    client.disconnectFromHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect from broker.");
}

void Tst_QMqttSubscription::qtbug_135653_connected()
{
    VersionClient(QMqttClient::ProtocolVersion(0), client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    const QString topic(QLatin1String("test/foo/bar/test3"));

    QMqttSubscription *subscriptions[10] = { 0 };
    for (int i = 0; i < 10; ++i) {
        bool received = false;
        subscriptions[i] = client.subscribe(topic);
        connect(subscriptions[i], &QMqttSubscription::messageReceived, subscriptions[i], [&client, topic, &received](QMqttMessage msg) {
            received = (msg.payload() == "Hello World");
        });
        client.publish(topic, QString(QLatin1String("Hello World")).toLocal8Bit(), 1, true);
        QTRY_VERIFY2(received, "Could not receive message.");
        client.unsubscribe(topic);
        client.unsubscribe(topic);
    }

    for (int i = 0; i < 10; ++i) {
        QVERIFY(subscriptions[i]);
        for (int j = i + 1; j < 10; ++j)
            QVERIFY(subscriptions[i] != subscriptions[j]);
    }
}

void Tst_QMqttSubscription::qtbug_135653_not_connected()
{
    VersionClient(QMqttClient::ProtocolVersion(0), client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    const QString topic(QLatin1String("test/foo/bar/test4"));

    QMqttSubscription *subscriptions[10] = { 0 };
    for (int i = 0; i < 10; ++i) {
        bool received = false;
        client.connectToHost();
        QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");
        subscriptions[i] = client.subscribe(topic);
        connect(subscriptions[i], &QMqttSubscription::messageReceived, subscriptions[i], [&client, topic, &received](QMqttMessage msg) {
            received = (msg.payload() == "Hello World");
        });
        client.publish(topic, QString(QLatin1String("Hello World")).toLocal8Bit(), 1, true);
        QTRY_VERIFY2(received, "Could not receive message.");
        client.disconnectFromHost();
        QTRY_VERIFY2(client.state() == QMqttClient::Disconnected, "Could not disconnect from broker.");
        client.unsubscribe(topic);
        client.unsubscribe(topic);
    }
    for (int i = 0; i < 10; ++i) {
        QVERIFY(subscriptions[i]);
        for (int j = i + 1; j < 10; ++j)
            QVERIFY(subscriptions[i] != subscriptions[j]);
    }
}

void Tst_QMqttSubscription::qtbug_135653_not_unsubscribed()
{

    const QString topic(QLatin1String("test/foo/bar/test5"));

    QMqttSubscription *subscriptions[10] = { 0 };
    for (int i = 0; i < 10; ++i) {
        bool received = false;
        VersionClient(QMqttClient::ProtocolVersion(0), client);
        client.setHostname(m_testBroker);
        client.setPort(m_port);
        client.connectToHost();
        QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");
        subscriptions[i] = client.subscribe(topic);
        connect(subscriptions[i], &QMqttSubscription::messageReceived, subscriptions[i], [&client, topic, &received](QMqttMessage msg) {
            received = (msg.payload() == "Hello World");
        });
        client.publish(topic, QString(QLatin1String("Hello World")).toLocal8Bit(), 1, true);
        QTRY_VERIFY2(received, "Could not receive message.");
    }
}

QTEST_MAIN(Tst_QMqttSubscription)

#include "tst_qmqttsubscription.moc"
