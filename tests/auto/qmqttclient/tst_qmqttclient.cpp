#include <QtCore/QString>
#include <QtTest/QtTest>
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
private:
    QString m_testBroker;
    quint16 m_port{1883};
};

Tst_QMqttClient::Tst_QMqttClient()
{
}

void Tst_QMqttClient::initTestCase()
{
    m_testBroker = qgetenv("MQTT_TEST_BROKER");
    if (m_testBroker.isEmpty()) {
        QFAIL("No test server given.");
        return;
    }
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
    QSharedPointer<QMqttSubscription> subscription;
    const QString testTopic = QLatin1String("test/topic");

    connect(&subscriber, &QMqttClient::connected, [&](){
        subscription = subscriber.subscribe(testTopic);
        if (!subscription)
            qFatal("Failed to create subscription");

        connect(subscription.data(), &QMqttSubscription::messageReceived, [&](QByteArray msg) {
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

QTEST_MAIN(Tst_QMqttClient)

#include "tst_qmqttclient.moc"
