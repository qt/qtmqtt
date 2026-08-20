// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "broker_connection.h"

#include <QtCore/QString>
#include <QtNetwork/QTcpServer>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtMqtt/QMqttClient>

#include <limits>

class Tst_QMqttClient : public QObject
{
    Q_OBJECT

public:
    Tst_QMqttClient();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void getSetCheck_data();
    void getSetCheck();
    void sendReceive_data();
    void sendReceive();
    void retainMessage_data();
    void retainMessage();
    void willMessage_data();
    void willMessage();
    void compliantTopic_data();
    void compliantTopic();
    void subscribeLongTopic_data();
    void subscribeLongTopic();
    void dataIncludingZero_data();
    void dataIncludingZero();
    void publishLongTopic_data();
    void publishLongTopic();
    void reconnect_QTBUG65726_data();
    void reconnect_QTBUG65726();
    void openIODevice_QTBUG66955_data();
    void openIODevice_QTBUG66955();
    void subackLongRemainingLength();
    void pubackLongRemainingLength();
    void pubrelWithReasonCode();
    void shortRemainingLength_data();
    void shortRemainingLength();
    void staticProperties_QTBUG_67176_data();
    void staticProperties_QTBUG_67176();
    void authentication();
    void messageStatus_data();
    void messageStatus();
    void messageStatusReceive_data();
    void messageStatusReceive();
    void subscriptionIdsOverlap();
    void keepAlive_data();
    void keepAlive();
    void reconnect();
private:
    QProcess m_brokerProcess;
    QString m_testBroker;
    quint16 m_port{0};
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

DefaultVersionTestData(Tst_QMqttClient::getSetCheck_data)

void Tst_QMqttClient::getSetCheck()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);
    VersionClient(mqttVersion, client);

    QVERIFY(client.clientId().size() > 0);
    const QString clientId = QLatin1String("testclient123");
    client.setClientId(clientId);
    QCOMPARE(client.clientId(), clientId);

    QCOMPARE(client.hostname(), QString());
    const QString hostname = QLatin1String("qt.io");
    client.setHostname(hostname);
    QCOMPARE(client.hostname(), hostname);

    QCOMPARE(client.port(), quint16(0));
    client.setPort(1883);
    QCOMPARE(client.port(), quint16(1883));

    QCOMPARE(client.keepAlive(), quint16(60));
    client.setKeepAlive(10);
    QCOMPARE(client.keepAlive(), quint16(10));

    // Available protocol versions
    QMqttClient client2;
    QCOMPARE(client2.protocolVersion(), QMqttClient::MQTT_3_1_1);
    client2.setProtocolVersion(QMqttClient::ProtocolVersion(0));
    QCOMPARE(client2.protocolVersion(), QMqttClient::MQTT_3_1_1);
    client2.setProtocolVersion(QMqttClient::ProtocolVersion(6));
    QCOMPARE(client2.protocolVersion(), QMqttClient::MQTT_3_1_1);
    client2.setProtocolVersion(QMqttClient::MQTT_3_1);
    QCOMPARE(client2.protocolVersion(), QMqttClient::MQTT_3_1);
    client2.setProtocolVersion(QMqttClient::MQTT_5_0);
    QCOMPARE(client2.protocolVersion(), QMqttClient::MQTT_5_0);

#ifdef QT_BUILD_INTERNAL
    if (qEnvironmentVariableIsSet("QT_MQTT_TEST_USERNAME"))
        QEXPECT_FAIL("", "Default username has been overwritten.", Continue);
#endif
    QCOMPARE(client.username(), QString());
#ifdef QT_BUILD_INTERNAL
    if (qEnvironmentVariableIsSet("QT_MQTT_TEST_PASSWORD"))
        QEXPECT_FAIL("", "Default username has been overwritten.", Continue);
#endif
    QCOMPARE(client.password(), QString());
    QCOMPARE(client.cleanSession(), true);
    QCOMPARE(client.willTopic(), QString());
    QCOMPARE(client.willMessage(), QByteArray());
    QCOMPARE(client.willQoS(), quint8(0));
    QCOMPARE(client.willRetain(), false);
    QCOMPARE(client.autoKeepAlive(), true);
    client.setAutoKeepAlive(false);
    QCOMPARE(client.autoKeepAlive(), false);
}

void Tst_QMqttClient::sendReceive_data()
{
    QTest::addColumn<QMqttClient::ProtocolVersion>("mqttVersion");
    QTest::addColumn<QByteArray>("data");

    QList<QMqttClient::ProtocolVersion> versions{QMqttClient::MQTT_3_1_1, QMqttClient::MQTT_5_0};

    for (int i = 0; i < 2; ++i) {
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":empty")) << versions[i] << QByteArray();
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":simple")) << versions[i] << QByteArray("This is a test message");
        QByteArray d;
        d.fill('A', 500);
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":big")) << versions[i] << d;
        d.fill('B', (128 * 128 * 128) + 4);
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":huge")) << versions[i] << d;
    }
}

void Tst_QMqttClient::sendReceive()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);
    QFETCH(QByteArray, data);
    const QString testTopic = QLatin1String("Qt/QMqttClient/Topic");

    VersionClient(mqttVersion, publisher);
    publisher.setClientId(QLatin1String("publisher"));
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    publisher.connectToHost();
    QTRY_COMPARE(publisher.state(), QMqttClient::Connected);

    VersionClient(mqttVersion, subscriber);
    subscriber.setClientId(QLatin1String("subscriber"));
    subscriber.setHostname(m_testBroker);
    subscriber.setPort(m_port);

    subscriber.connectToHost();
    QTRY_COMPARE(subscriber.state(), QMqttClient::Connected);

    bool received = false;
    bool verified = false;
    auto sub = subscriber.subscribe(testTopic, 1);
    QVERIFY(sub);
    connect(sub, &QMqttSubscription::messageReceived, sub, [&](QMqttMessage msg) {
        verified = msg.payload() == data;
        received = true;
    });

    QTRY_COMPARE(sub->state(), QMqttSubscription::Subscribed);

    if (subscriber.protocolVersion() == QMqttClient::MQTT_5_0 &&
            (quint32)data.size() > subscriber.serverConnectionProperties().maximumPacketSize())
        QSKIP("The MQTT 5 test broker does not support huge packages.", SkipOnce);
    publisher.publish(testTopic, data, 1);

    QTRY_VERIFY2_WITH_TIMEOUT(received, "Subscriber did not receive message", std::chrono::seconds(180));
    QVERIFY2(verified, "Subscriber received different message");
}

DefaultVersionTestData(Tst_QMqttClient::retainMessage_data)

void Tst_QMqttClient::retainMessage()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    const QString testTopic = QLatin1String("Qt/QMqttClient/Topic2");
    const QByteArray testMessage("retainedMessage");

    // Publisher
    VersionClient(mqttVersion, publisher);
    publisher.setClientId(QLatin1String("publisher"));
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    publisher.connectToHost();
    QTRY_COMPARE(publisher.state(), QMqttClient::Connected);

    // [MQTT-3.3.1-7]
    // If the Server receives a QoS 0 message with the RETAIN flag set to 1 it MUST
    // discard any message previously retained for that topic. It SHOULD store the
    // new QoS 0 message
    publisher.publish(testTopic, QByteArray(), 0, true);
    // We cannot wait as 0 QoS does not send confirmation
    QTest::qWait(500);

    for (int i = 0; i < 2; i++) {
        int msgCount = 0;

        QSignalSpy publishSpy(&publisher, SIGNAL(messageSent(qint32)));
        publisher.publish(testTopic, testMessage, 1, i == 1 ? true : false);
        QTRY_COMPARE(publishSpy.size(), 1);

        VersionClient(mqttVersion, sub);
        sub.setClientId(QLatin1String("SubA"));
        sub.setHostname(m_testBroker);
        sub.setPort(m_port);
        connect(&sub, &QMqttClient::messageReceived, &sub,
                [&msgCount, testMessage](const QByteArray &msg) {
            if (msg == testMessage)
                msgCount++;
        });

        QSignalSpy messageSpy(&sub, SIGNAL(messageReceived(QByteArray,QMqttTopicName)));
        sub.connectToHost();
        QTRY_COMPARE(sub.state(), QMqttClient::Connected);

        auto subscription = sub.subscribe(testTopic);
        QTRY_COMPARE(subscription->state(), QMqttSubscription::Subscribed);

        QTRY_VERIFY(msgCount == i);
    }
    publisher.disconnect();
}

void Tst_QMqttClient::willMessage_data()
{
    QTest::addColumn<QMqttClient::ProtocolVersion>("mqttVersion");
    QTest::addColumn<QByteArray>("willMessage");

    QList<QMqttClient::ProtocolVersion> versions{QMqttClient::MQTT_3_1_1, QMqttClient::MQTT_5_0};

    for (int i = 0; i < 2; ++i) {
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":simple")) << versions[i] << QByteArray("The client connection is gone.");
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":empty")) << versions[i] << QByteArray();
    }
}

void Tst_QMqttClient::willMessage()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);
    QFETCH(QByteArray, willMessage);

    const QString willTopic = QLatin1String("Qt/QMqttClient/will/topic");

    // Client A connects
    VersionClient(mqttVersion, client1);
    client1.setHostname(m_testBroker);
    client1.setPort(m_port);
    client1.connectToHost();
    QTRY_COMPARE(client1.state(), QMqttClient::Connected);

    if (!client1.isTcp())
        QSKIP("This test is only for tcp");

    auto client1Sub = client1.subscribe(willTopic, 1);
    connect(client1Sub, &QMqttSubscription::messageReceived,
            client1Sub, [=](QMqttMessage message) {
        Q_UNUSED(message);
        // Just debug purposes
        //qDebug() << "Got something:" << message;
    });
    QTRY_COMPARE(client1Sub->state(), QMqttSubscription::Subscribed);

    QSignalSpy messageSpy(client1Sub, SIGNAL(messageReceived(QMqttMessage)));

    // Client B connects (via TcpSocket)
    QTcpSocket sock;
    sock.connectToHost(client1.hostname(), client1.port());
    QVERIFY(sock.waitForConnected());

    for (int i = 1; i > 0; --i) {
        VersionClient(mqttVersion, willClient);
        if (i == 1)
            willClient.setTransport(&sock, QMqttClient::AbstractSocket);
        else {
            willClient.setHostname(m_testBroker);
            willClient.setPort(m_port);
        }
        willClient.setWillQoS(1);
        willClient.setWillTopic(willTopic);
        willClient.setWillMessage(willMessage);
        willClient.connectToHost();
        QTRY_COMPARE(willClient.state(), QMqttClient::Connected);

        willClient.publish(QLatin1String("noninteresting"), "just something");

        // Be evil and kill the connection without DISCONNECT
        // Should send will message to client1.
        // When you manually disconnect (send the DISCONNECT command) no will message
        // is sent
        if (i == 1)
            sock.disconnectFromHost();
        else
            willClient.disconnectFromHost();
        QTest::qWait(500);
        QTRY_COMPARE(messageSpy.size(), i);
    }
}

void Tst_QMqttClient::compliantTopic_data()
{
    QTest::addColumn<QMqttClient::ProtocolVersion>("mqttVersion");
    QTest::addColumn<QString>("topic");

    QList<QMqttClient::ProtocolVersion> versions{QMqttClient::MQTT_3_1_1, QMqttClient::MQTT_5_0};

    for (int i = 0; i < 2; ++i) {
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":simple")) << versions[i] << QString::fromLatin1("Qt/QMqttClient/topic");
        QTest::newRow(qPrintable(QString::number(versions[i]) + ":subPath")) << versions[i] << QString::fromLatin1("Qt/QMqttClient/topic/subtopic");

        if (versions[i] != QMqttClient::MQTT_5_0) {
            QString l;
            l.fill(QLatin1Char('T'), std::numeric_limits<std::uint16_t>::max());
            QTest::newRow(qPrintable(QString::number(versions[i]) + ":maxSize")) << versions[i] << l;
        }
    }
}

void Tst_QMqttClient::compliantTopic()
{
    QFETCH(QString, topic);
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    QString truncTopic = topic;
    truncTopic.truncate(std::numeric_limits<std::uint16_t>::max());

    VersionClient(mqttVersion, publisher);
    publisher.setClientId(QLatin1String("publisher"));
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    publisher.connectToHost();
    QTRY_COMPARE(publisher.state(), QMqttClient::Connected);

    VersionClient(mqttVersion, subscriber);
    subscriber.setClientId(QLatin1String("subscriber"));
    subscriber.setHostname(m_testBroker);
    subscriber.setPort(m_port);

    subscriber.connectToHost();
    QTRY_COMPARE(subscriber.state(), QMqttClient::Connected);

    bool received = false;
    bool verified = false;
    connect(&subscriber, &QMqttClient::messageReceived,
            &subscriber, [&](const QByteArray &, const QMqttTopicName &t) {
        received = true;
        verified = t == truncTopic;
    });

    auto client1Sub = subscriber.subscribe(truncTopic, 1);
    QTRY_COMPARE(client1Sub->state(), QMqttSubscription::Subscribed);

    publisher.publish(topic, QByteArray("Msgs"), 1);

    QTRY_VERIFY2(received, "Subscriber did not receive message");
    QVERIFY2(verified, "Subscriber received different message");
}

DefaultVersionTestData(Tst_QMqttClient::subscribeLongTopic_data)

void Tst_QMqttClient::subscribeLongTopic()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    VersionClient(mqttVersion, subscriber);
    subscriber.setClientId(QLatin1String("subscriber"));
    subscriber.setHostname(m_testBroker);
    subscriber.setPort(m_port);

    subscriber.connectToHost();
    QTRY_COMPARE(subscriber.state(), QMqttClient::Connected);

    QString topic;
    topic.fill(QLatin1Char('s'), 2 * std::numeric_limits<std::uint16_t>::max());
    auto sub = subscriber.subscribe(topic);
    QCOMPARE(sub, nullptr);
}

DefaultVersionTestData(Tst_QMqttClient::dataIncludingZero_data)

void Tst_QMqttClient::dataIncludingZero()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    QByteArray data;
    const int dataSize = 200;
    data.fill('A', dataSize);
    data[100] = '\0';

    VersionClient(mqttVersion, client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    bool received = false;
    bool verified = false;
    bool correctSize = false;
    const QString testTopic(QLatin1String("Qt/QMqttClient/some/topic"));
    auto sub = client.subscribe(testTopic, 1);
    QVERIFY(sub);
    connect(sub, &QMqttSubscription::messageReceived,
            sub, [&](QMqttMessage msg) {
        verified = msg.payload() == data;
        correctSize = msg.payload().size() == dataSize;
        received = true;
    });

    QTRY_COMPARE(sub->state(), QMqttSubscription::Subscribed);

    client.publish(testTopic, data, 1);

    QTRY_VERIFY2(received, "Subscriber did not receive message");
    QVERIFY2(verified, "Subscriber received different message");
    QVERIFY2(correctSize, "Subscriber received message of different size");
}

DefaultVersionTestData(Tst_QMqttClient::publishLongTopic_data)

void Tst_QMqttClient::publishLongTopic()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    VersionClient(mqttVersion, publisher);
    publisher.setClientId(QLatin1String("publisher"));
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    publisher.connectToHost();
    QTRY_COMPARE(publisher.state(), QMqttClient::Connected);

    QString topic;
    topic.fill(QLatin1Char('s'), 2 * std::numeric_limits<std::uint16_t>::max());
    auto pub = publisher.publish(topic);
    QCOMPARE(pub, -1);
}

class FakeServer : public QObject
{
    Q_OBJECT
public:
    FakeServer() {
        server = new QTcpServer(this);
        connect(server, &QTcpServer::newConnection, this, &FakeServer::createSocket);
        server->listen(QHostAddress::Any, 5726);
    }
    // Sends raw bytes to the client, so a test can drive the parser with an exact
    // byte sequence.
    void send(const QByteArray &data) { socket->write(data); }
public slots:
    void createSocket() {
        socket = server->nextPendingConnection();
        connectionAcknowledged = false;
        lastRequest.clear();
        connect(socket, &QTcpSocket::readyRead, this, &FakeServer::connectionRequested);
    }

    void connectionRequested() {
        // We assume the first packet is always a connect statement, so no verification
        // is done. Everything after that is recorded for the test to answer itself.
        if (connectionAcknowledged) {
            lastRequest.append(socket->readAll());
            return;
        }
        connectionAcknowledged = true;
        socket->readAll();
        QByteArray response;
        response += 0x20;
        response += quint8(2); // Payload size
        if (!connectionSuccess) {
            response += quint8(255); // Causes ProtocolViolation
            response += quint8(13);
        } else {
            response += char(0); // ackFlags
            response += char(0); // result
            if (version == QMqttClient::MQTT_5_0) {
                response += char(0); // No properties
                response[1] = response[1] + 1; // Adjust payload size
            }
        }
        qDebug() << "Fake server response:" << connectionSuccess;
        socket->write(response);
    }
public:
    QTcpServer *server;
    QTcpSocket *socket;
    QMqttClient::ProtocolVersion version{QMqttClient::MQTT_3_1_1};
    bool connectionSuccess{false};
    bool connectionAcknowledged{false};
    QByteArray lastRequest;
};

DefaultVersionTestData(Tst_QMqttClient::reconnect_QTBUG65726_data)

void Tst_QMqttClient::reconnect_QTBUG65726()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    FakeServer server;

    VersionClient(mqttVersion, client);
    client.setClientId(QLatin1String("bugclient"));
    client.setHostname(QLatin1String("localhost"));
    client.setPort(5726);

    if (!client.isTcp())
        QSKIP("This test is only for tcp");

    server.version = client.protocolVersion();

    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Disconnected);
    QTRY_COMPARE(client.error(), QMqttClient::ProtocolViolation);

    server.connectionSuccess = true;

    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);
    QTRY_COMPARE(client.error(), QMqttClient::NoError);
}

class IOTransport : public QIODevice
{
public:
    bool open(OpenMode mode) override {
        return QIODevice::open(mode);
    }
    qint64 readData(char *data, qint64 maxlen) override {
        Q_UNUSED(data);
        Q_UNUSED(maxlen);
        return 0;
    }
    qint64 writeData(const char *data, qint64 len) override {
        Q_UNUSED(data);
        Q_UNUSED(len);
        if (data[0] == 0x10)
            written = 1;
        else
            qDebug() << "Received unknown/invalid data";
        return len;
    }
    QAtomicInt written{0};
};

DefaultVersionTestData(Tst_QMqttClient::openIODevice_QTBUG66955_data)

void Tst_QMqttClient::openIODevice_QTBUG66955()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    IOTransport trans;
    trans.open(QIODevice::ReadWrite);

    VersionClient(mqttVersion, client);
    client.setTransport(&trans, QMqttClient::IODevice);
    client.connectToHost();

    QTRY_COMPARE(trans.written, 1);
}

// A reason string property (0x1f) of the requested content length.
static QByteArray reasonStringProperty(quint16 length)
{
    QByteArray property;
    property.append(char(0x1f));
    property.append(char(length >> 8));
    property.append(char(length & 0xff));
    property.append(QByteArray(length, 'r'));
    return property;
}

// The packet identifier of a packet the client wrote, skipping the packet type and
// its variable byte remaining length.
static quint16 packetIdentifier(const QByteArray &packet)
{
    qsizetype pos = 1;
    while (pos < packet.size() && (packet.at(pos) & 128) != 0)
        ++pos;
    Q_ASSERT(pos < packet.size() - 2);
    ++pos;
    return quint16(quint8(packet.at(pos)) << 8 | quint8(packet.at(pos + 1)));
}

static void appendPacketIdentifier(QByteArray *payload, quint16 id)
{
    payload->append(char(id >> 8));
    payload->append(char(id & 0xff));
}

// MQTT-2.2.3 (3.1.1) / MQTT-2.1.4 (5.0) The remaining length is a variable byte
// integer, so a packet longer than 127 bytes carries a two byte length whose first
// byte has the continuation bit set. Reading only that first byte consumes one byte
// too few and leaves the rest of the payload to be parsed as the next packet's
// fixed header.
void Tst_QMqttClient::subackLongRemainingLength()
{
    FakeServer server;
    server.version = QMqttClient::MQTT_5_0;
    server.connectionSuccess = true;

    VersionClient(QMqttClient::MQTT_5_0, client);
    if (!client.isTcp())
        QSKIP("This test is only for tcp");
    client.setHostname(QLatin1String("localhost"));
    client.setPort(5726);
    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    auto sub = client.subscribe(QMqttTopicFilter(QLatin1String("a/b")), 1);
    QVERIFY(sub);
    QTRY_VERIFY(server.lastRequest.size() >= 4); // The SUBSCRIBE arrived

    // SUBACK with a reason string, which pushes the remaining length past 127.
    QByteArray payload;
    appendPacketIdentifier(&payload, packetIdentifier(server.lastRequest));
    const QByteArray properties = reasonStringProperty(123);
    payload.append(char(properties.size())); // Property length, 126
    payload.append(properties);
    payload.append(char(0x01)); // Reason code: maximum QoS 1
    QCOMPARE(payload.size(), 130);

    QByteArray packet;
    packet.append(char(0x90));
    packet.append(char(0x82)); // Remaining length 130, low seven bits plus continuation
    packet.append(char(0x01)); // Remaining length 130, remaining bits
    packet.append(payload);
    server.send(packet);

    QTRY_COMPARE(sub->state(), QMqttSubscription::Subscribed);
    QCOMPARE(client.state(), QMqttClient::Connected);
    QCOMPARE(client.error(), QMqttClient::NoError);
}

void Tst_QMqttClient::pubackLongRemainingLength()
{
    FakeServer server;
    server.version = QMqttClient::MQTT_5_0;
    server.connectionSuccess = true;

    VersionClient(QMqttClient::MQTT_5_0, client);
    if (!client.isTcp())
        QSKIP("This test is only for tcp");
    client.setHostname(QLatin1String("localhost"));
    client.setPort(5726);
    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    const qint32 id = client.publish(QMqttTopicName(QLatin1String("a/b")), QByteArray("x"), 1);
    QVERIFY(id > 0);
    QTRY_VERIFY(server.lastRequest.size() >= 4); // The PUBLISH arrived

    QSignalSpy sentSpy(&client, &QMqttClient::messageSent);

    // PUBACK with a reason string, which pushes the remaining length past 127.
    QByteArray payload;
    appendPacketIdentifier(&payload, quint16(id));
    payload.append(char(0x00)); // Reason code: Success
    const QByteArray properties = reasonStringProperty(123);
    payload.append(char(properties.size())); // Property length, 126
    payload.append(properties);
    QCOMPARE(payload.size(), 130);

    QByteArray packet;
    packet.append(char(0x40));
    packet.append(char(0x82)); // Remaining length 130, low seven bits plus continuation
    packet.append(char(0x01)); // Remaining length 130, remaining bits
    packet.append(payload);
    server.send(packet);

    QTRY_COMPARE(sentSpy.count(), 1);
    QCOMPARE(sentSpy.at(0).at(0).toInt(), id);
    QCOMPARE(client.state(), QMqttClient::Connected);
    QCOMPARE(client.error(), QMqttClient::NoError);
}

// MQTT-3.6.2, MQTT-3.6.2.1 In MQTT 5.0 the PUBREL variable header holds a reason
// code and properties in addition to the packet identifier. A remaining length of 2
// only occurs when both are omitted, so a longer PUBREL must be accepted.
void Tst_QMqttClient::pubrelWithReasonCode()
{
    FakeServer server;
    server.version = QMqttClient::MQTT_5_0;
    server.connectionSuccess = true;

    VersionClient(QMqttClient::MQTT_5_0, client);
    if (!client.isTcp())
        QSKIP("This test is only for tcp");
    client.setHostname(QLatin1String("localhost"));
    client.setPort(5726);
    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    QMqtt::MessageStatus status = QMqtt::MessageStatus::Unknown;
    QString reasonString;
    connect(&client, &QMqttClient::messageStatusChanged, &client,
            [&](qint32, QMqtt::MessageStatus s, const QMqttMessageStatusProperties &properties)
    {
        status = s;
        reasonString = properties.reason();
    });

    // PUBLISH, QoS 2, topic "a/b", packet identifier 7, no properties, payload "x".
    // The client answers with a PUBREC, which puts it in the middle of the QoS 2
    // exchange and makes it expect a PUBREL.
    server.send(QByteArray::fromHex("34090003612f6200070078"));
    QTRY_VERIFY(server.lastRequest.size() >= 4); // The PUBREC arrived

    // PUBREL, packet identifier 7, reason code Success, and a reason string. Remaining
    // length 12: 2 identifier + 1 reason code + 1 property length + 8 property bytes.
    server.send(QByteArray::fromHex("620c000700081f000568656c6c6f"));

    QTRY_COMPARE(status, QMqtt::MessageStatus::Released);
    QCOMPARE(reasonString, QLatin1String("hello"));
    QCOMPARE(client.state(), QMqttClient::Connected);
    QCOMPARE(client.error(), QMqttClient::NoError);
}

// The remaining length counts the bytes that follow the fixed header, so a packet
// declaring less than the fields the specification requires is malformed. Accepting
// one leaves the parser reading past the end of the packet: the reason code loops in
// finalize_suback() and finalize_unsuback() read at least once regardless of what is
// left, and finalize_pubAckRecRelComp() always reads a packet identifier. Each of
// these packets must be rejected as a protocol violation instead.
void Tst_QMqttClient::shortRemainingLength_data()
{
    QTest::addColumn<QMqttClient::ProtocolVersion>("mqttVersion");
    QTest::addColumn<QByteArray>("packet");

    // SUBACK, packet identifier 7, and nothing else. MQTT-3.9.3 requires at least one
    // reason code, so the shortest legal remaining length is 3.
    QTest::newRow("SUBACK 3.1.1, no reason code")
            << QMqttClient::MQTT_3_1_1 << QByteArray::fromHex("90020007");
    // The same, plus an empty property block. MQTT-3.9.2.1 makes the property length
    // mandatory in MQTT 5.0, so the shortest legal remaining length is 4.
    QTest::newRow("SUBACK 5.0, no reason code")
            << QMqttClient::MQTT_5_0 << QByteArray::fromHex("9003000700");
    // UNSUBACK, packet identifier 7, empty property block, no reason code. The
    // property length is mandatory (MQTT-3.11.2.1) and at least one reason code is
    // required (MQTT-3.11.3), so the shortest legal remaining length is 4.
    QTest::newRow("UNSUBACK 5.0, no reason code")
            << QMqttClient::MQTT_5_0 << QByteArray::fromHex("b003000700");
    // PUBACK with half a packet identifier. The reason code and the property length may
    // both be omitted (MQTT-3.4.2.1), but the identifier may not, so the shortest legal
    // remaining length is 2.
    QTest::newRow("PUBACK 5.0, truncated packet identifier")
            << QMqttClient::MQTT_5_0 << QByteArray::fromHex("400100");
    // The same for PUBREL, MQTT-3.6.2.1.
    QTest::newRow("PUBREL 5.0, truncated packet identifier")
            << QMqttClient::MQTT_5_0 << QByteArray::fromHex("620100");
}

void Tst_QMqttClient::shortRemainingLength()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);
    QFETCH(QByteArray, packet);

    FakeServer server;
    server.version = mqttVersion;
    server.connectionSuccess = true;

    VersionClient(mqttVersion, client);
    if (!client.isTcp())
        QSKIP("This test is only for tcp");
    client.setHostname(QLatin1String("localhost"));
    client.setPort(5726);
    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    server.send(packet);

    QTRY_COMPARE(client.state(), QMqttClient::Disconnected);
    QCOMPARE(client.error(), QMqttClient::ProtocolViolation);
}

DefaultVersionTestData(Tst_QMqttClient::staticProperties_QTBUG_67176_data)

void Tst_QMqttClient::staticProperties_QTBUG_67176()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);

    VersionClient(mqttVersion, client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    auto clientVersion = client.protocolVersion();
    const QString hostname = client.hostname();
    const int port = client.port();
    const QString clientId = client.clientId();
    const quint16 keepAlive = client.keepAlive();
    const bool clean = client.cleanSession();

    client.connectToHost();
    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    client.setClientId(QLatin1String("someclient"));
    QCOMPARE(client.clientId(), clientId);

    client.setHostname(QLatin1String("some.domain.foo"));
    QCOMPARE(client.hostname(), hostname);

    client.setPort(1234);
    QCOMPARE(client.port(), port);

    client.setKeepAlive(keepAlive + 10);
    QCOMPARE(client.keepAlive(), keepAlive);

    client.setProtocolVersion(QMqttClient::MQTT_3_1);
    QCOMPARE(client.protocolVersion(), clientVersion);

    client.setUsername(QLatin1String("someUser"));
    QCOMPARE(client.username(), QLatin1String());

    client.setPassword(QLatin1String("somePassword"));
    QCOMPARE(client.password(), QLatin1String());

    client.setCleanSession(!clean);
    QCOMPARE(client.cleanSession(), clean);
}

void Tst_QMqttClient::authentication()
{
    VersionClient(QMqttClient::MQTT_5_0, client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    QMqttConnectionProperties connectionProperties;
    connectionProperties.setAuthenticationMethod(QLatin1String("SCRAM-SHA-1"));
    client.setConnectionProperties(connectionProperties);

    connect(&client, &QMqttClient::authenticationRequested, &client,
            [](const QMqttAuthenticationProperties &prop)
    {
        qDebug() << "Authentication requested:" << prop.authenticationMethod();
    });

    connect(&client, &QMqttClient::authenticationFinished, &client,
            [](const QMqttAuthenticationProperties &prop)
    {
        qDebug() << "Authentication finished:" << prop.authenticationMethod();
    });

    // ### FIXME : There is no public test broker yet able to handle authentication methods
    // Theoretically the broker should send an AUTH request, followed by AUTH call including
    // authentication data. See 4.12 of MQTT v5 specs.
    QSKIP("No broker available with enhanced authentication.");
    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);
}

Q_DECLARE_METATYPE(QMqtt::MessageStatus)

void Tst_QMqttClient::messageStatus_data()
{
    QTest::addColumn<int>("qos");
    QTest::addColumn<QList<QMqtt::MessageStatus>>("expectedStatus");

    QTest::newRow("QoS1") << 1 << (QList<QMqtt::MessageStatus>() << QMqtt::MessageStatus::Acknowledged);
    QTest::newRow("QoS2") << 2 << (QList<QMqtt::MessageStatus>() << QMqtt::MessageStatus::Received
                                   << QMqtt::MessageStatus::Completed);
}

void Tst_QMqttClient::messageStatus()
{
    QFETCH(int, qos);
    QFETCH(QList<QMqtt::MessageStatus>, expectedStatus);

    VersionClient(QMqttClient::MQTT_5_0, client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.connectToHost();

    QTRY_VERIFY2(client.state() == QMqttClient::Connected, "Could not connect to broker.");

    const QString topic = QLatin1String("Qt/client/statusCheck");

    connect(&client, &QMqttClient::messageStatusChanged,
            &client, [&expectedStatus](qint32,
                QMqtt::MessageStatus s,
                const QMqttMessageStatusProperties &)
    {
        QCOMPARE(s, expectedStatus.first());
        expectedStatus.takeFirst();
    });

    QSignalSpy publishSpy(&client, &QMqttClient::messageSent);
    client.publish(topic, QByteArray("someContent"), quint8(qos));
    QTRY_VERIFY2(publishSpy.size() == 1, "Could not publish message");
    QTRY_VERIFY2(expectedStatus.isEmpty(), "Did not receive all status updates.");
}

void Tst_QMqttClient::messageStatusReceive_data()
{
    QTest::addColumn<int>("qos");
    QTest::addColumn<QList<QMqtt::MessageStatus>>("expectedStatus");

    QTest::newRow("QoS1") << 1 << (QList<QMqtt::MessageStatus>() << QMqtt::MessageStatus::Published);
    QTest::newRow("QoS2") << 2 << (QList<QMqtt::MessageStatus>() << QMqtt::MessageStatus::Published
                                   << QMqtt::MessageStatus::Released);
}

void Tst_QMqttClient::messageStatusReceive()
{
    QFETCH(int, qos);
    QFETCH(QList<QMqtt::MessageStatus>, expectedStatus);

    VersionClient(QMqttClient::MQTT_5_0, publisher);
    publisher.setHostname(m_testBroker);
    publisher.setPort(m_port);

    publisher.connectToHost();
    QTRY_VERIFY2(publisher.state() == QMqttClient::Connected, "Could not connect to broker.");

    VersionClient(QMqttClient::MQTT_5_0, subscriber);
    subscriber.setHostname(m_testBroker);
    subscriber.setPort(m_port);

    subscriber.connectToHost();
    QTRY_VERIFY2(subscriber.state() == QMqttClient::Connected, "Could not connect to broker.");

    const QString topic = QLatin1String("Qt/client/statusCheckReceive");

    auto subscription = subscriber.subscribe(topic, quint8(qos));
    QTRY_VERIFY2(subscription->state() == QMqttSubscription::Subscribed, "Could not subscribe to topic");
    QVERIFY(subscription->qos() >= qos);

    connect(&subscriber, &QMqttClient::messageStatusChanged, &subscriber,
            [&expectedStatus](qint32, QMqtt::MessageStatus s, const QMqttMessageStatusProperties &)
    {
        QCOMPARE(s, expectedStatus.first());
        expectedStatus.takeFirst();
    });

    QSignalSpy publishSpy(&publisher, &QMqttClient::messageSent);
    QSignalSpy receiveSpy(&subscriber, &QMqttClient::messageReceived);

    publisher.publish(topic, QByteArray("someContent"), quint8(qos));
    QTRY_VERIFY2(publishSpy.size() == 1, "Could not publish message");
    QTRY_VERIFY2(receiveSpy.size() == 1, "Did not receive message");

    QTRY_VERIFY2(expectedStatus.isEmpty(), "Did not receive all status updates.");
}

void Tst_QMqttClient::subscriptionIdsOverlap()
{

    // If the Server sends a single copy of the message it MUST include in the
    // PUBLISH packet the Subscription Identifiers for all matching
    // subscriptions which have a Subscription Identifiers, their order is not
    // significant [MQTT-3.3.4-4].
    // If the Server sends multiple PUBLISH packets it MUST send, in each of
    // them, the Subscription Identifier of the matching subscription if it has
    // a Subscription Identifier [MQTT-3.3.4-5].

    const QString topic = QLatin1String("Qt/client/idcheck");
    // Connect publisher
    VersionClient(QMqttClient::MQTT_5_0, pub);
    pub.setHostname(m_testBroker);
    pub.setPort(m_port);

    pub.connectToHost();
    QTRY_VERIFY2(pub.state() == QMqttClient::Connected, "Could not connect publisher.");

    // Connect subA
    VersionClient(QMqttClient::MQTT_5_0, subClientA);
    subClientA.setHostname(m_testBroker);
    subClientA.setPort(m_port);

    subClientA.connectToHost();
    QTRY_VERIFY2(subClientA.state() == QMqttClient::Connected, "Could not connect subscriber A.");

    QMqttSubscriptionProperties subAProp;
    subAProp.setSubscriptionIdentifier(8);
    auto subA = subClientA.subscribe(topic, subAProp, 1);
    QTRY_VERIFY2(subA->state() == QMqttSubscription::Subscribed, "Could not subscibe A.");

    int receiveACounter = 0;
    connect(subA, &QMqttSubscription::messageReceived, subA,
            [&receiveACounter](QMqttMessage msg) {
        qDebug() << "Sub A received:" << msg.publishProperties().subscriptionIdentifiers();
        // ### TODO: Wait for fix at https://github.com/eclipse/paho.mqtt.testing/issues/56
        //QVERIFY(msg.publishProperties().subscriptionIdentifiers().size() == 1);
        //QVERIFY(msg.publishProperties().subscriptionIdentifiers().at(0) == 8); // Use sub->id();
        receiveACounter++;
    });

    // Connect subB
    VersionClient(QMqttClient::MQTT_5_0, subClientB);
    subClientB.setHostname(m_testBroker);
    subClientB.setPort(m_port);

    subClientB.connectToHost();
    QTRY_VERIFY2(subClientB.state() == QMqttClient::Connected, "Could not connect subscriber A.");

    QMqttSubscriptionProperties subBProp;
    subBProp.setSubscriptionIdentifier(9);
    auto subB = subClientB.subscribe(topic, subBProp, 1);
    QTRY_VERIFY2(subB->state() == QMqttSubscription::Subscribed, "Could not subscibe A.");

    int receiveBCounter = 2;
    connect(subB, &QMqttSubscription::messageReceived,
            subB, [&receiveBCounter](QMqttMessage msg) {
        qDebug() << "Sub B received:" << msg.publishProperties().subscriptionIdentifiers();
        QVERIFY(msg.publishProperties().subscriptionIdentifiers().size() > 0);
        receiveBCounter -= msg.publishProperties().subscriptionIdentifiers().size();
    });

    QMqttSubscriptionProperties subB2Prop;
    subB2Prop.setSubscriptionIdentifier(14);
    auto subB2 = subClientB.subscribe(topic + "/#", subB2Prop, 1);
    QTRY_VERIFY2(subB2->state() == QMqttSubscription::Subscribed, "Could not subscibe A.");

    int receiveB2Counter = 2;
    connect(subB2, &QMqttSubscription::messageReceived,
            subB2, [&receiveB2Counter](QMqttMessage msg) {
        qDebug() << "Sub B2 received:" << msg.publishProperties().subscriptionIdentifiers();
        QVERIFY(msg.publishProperties().subscriptionIdentifiers().size() > 0);
        receiveB2Counter -= msg.publishProperties().subscriptionIdentifiers().size();
    });

    QSignalSpy publishSpy(&pub, &QMqttClient::messageSent);
    pub.publish(topic, "SomeData", 1);
    QTRY_VERIFY2(publishSpy.size() == 1, "Could not finalize publication.");
    QTRY_VERIFY2(receiveBCounter == 0, "Did not receive both messages.");
    QTRY_VERIFY2(receiveB2Counter == 0, "Did not receive both messages.");
    QTRY_VERIFY2(receiveACounter == 1, "Did not receive non-overlapping message.");
}

DefaultVersionTestData(Tst_QMqttClient::keepAlive_data)

void Tst_QMqttClient::keepAlive()
{
    QFETCH(QMqttClient::ProtocolVersion, mqttVersion);
    const quint16 keepAlive = 1;

    VersionClient(mqttVersion, client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);

    client.setKeepAlive(keepAlive);
    QCOMPARE(client.keepAlive(), keepAlive);
    QCOMPARE(client.autoKeepAlive(), true); // default

    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    if (client.protocolVersion() == QMqttClient::MQTT_5_0) {
        const auto serverProps = client.serverConnectionProperties();
        if (serverProps.isValid() && serverProps.availableProperties() & QMqttServerConnectionProperties::ServerKeepAlive) {
            if (serverProps.serverKeepAlive() == 0 || serverProps.serverKeepAlive() > keepAlive) {
                qDebug() << "Server specifies keepAlive which cannot be used for this test";
                return;
            }
        }
    }

    // Changing autoKeepAlive is not possible while connected
    client.setAutoKeepAlive(false);
    QCOMPARE(client.autoKeepAlive(), true);

    // Sending a manual ping request with autoKeepAlive is not allowed
    // and will be suppressed.
    const bool sent = client.requestPing();
    QCOMPARE(sent, false);

    // Verify auto keep alive works
    QSignalSpy spy(&client, &QMqttClient::pingResponseReceived);
    QTRY_VERIFY_WITH_TIMEOUT(spy.size() == 5, keepAlive * 1000 * 5 * 2);
    spy.clear();

    client.disconnectFromHost();
    QTRY_COMPARE(client.state(), QMqttClient::Disconnected);

    // Set manual keepAlive
    client.setAutoKeepAlive(false);
    QCOMPARE(client.autoKeepAlive(), false);
    client.setKeepAlive(keepAlive);

    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    // Check for timeout / disconnect when not ping sent
    QTRY_COMPARE_WITH_TIMEOUT(client.state(), QMqttClient::Disconnected, keepAlive * 1000 * 5 * 2);

    // Check manual ping
    QTimer t;
    t.setInterval(keepAlive * 1000);
    t.setSingleShot(false);
    t.connect(&t, &QTimer::timeout, &client, [&client]() {
        client.requestPing();
    });

    connect(&client, &QMqttClient::connected, &t, [&t]() {
        t.start();
    });

    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    QTRY_VERIFY_WITH_TIMEOUT(spy.size() == 5, keepAlive * 1000 * 5 * 2);

    client.disconnectFromHost();
    QTRY_COMPARE(client.state(), QMqttClient::Disconnected);
}

void Tst_QMqttClient::reconnect()
{
    VersionClient(QMqttClient::ProtocolVersion(0), client);
    client.setHostname(m_testBroker);
    client.setPort(m_port);
    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    client.disconnectFromHost();
    QTRY_COMPARE(client.state(), QMqttClient::Disconnected);

    /* The rest requires a mosquitto installation that runs
       on multiple ports.
    */
#if defined(MQTT_TEST_BROKER)
    client.setPort(client.port() + 1000);
    client.connectToHost();
    QTRY_COMPARE(client.state(), QMqttClient::Connected);

    client.disconnectFromHost();
    QTRY_COMPARE(client.state(), QMqttClient::Disconnected);

    QMqttClient client2;
    client2.setHostname(client.hostname());

    for (int i = 0; i < 2; ++i) {
#if defined(QT_MQTT_WITH_WEBSOCKETS)
        client2.setPort(9080);
        client2.connectToHostWebSocket();
        QTRY_COMPARE(client2.state(), QMqttClient::Connected);

        client2.disconnectFromHost();
        QTRY_COMPARE(client2.state(), QMqttClient::Disconnected);
#endif

#if defined(QT_MQTT_WITH_WEBSOCKETS) && !defined(QT_NO_SSL)
        client2.setPort(9081);
        QUrl url(QString(u"wss://") + client2.hostname() + QString(u":") + QString::number(client2.port()));
        QWebSocketHandshakeOptions options;
        options.setSubprotocols(QStringList{ QString::fromUtf8("mqttv3.1") });
        QWebSocket socket;
        socket.setSslConfiguration(makeConfig(QString::fromUtf8(getenv("MQTT_TEST_BROKER_CERTIFICATE"))));
        socket.open(url, options);

        client2.connectToHostWebSocketEncrypted(&socket);
        QTRY_COMPARE(client2.state(), QMqttClient::Connected);

        client2.disconnectFromHost();
        QTRY_COMPARE(client2.state(), QMqttClient::Disconnected);
#endif

#if !defined(QT_NO_SSL)
        client2.setPort(9883);
        client2.connectToHostEncrypted(makeConfig(QString::fromUtf8(getenv("MQTT_TEST_BROKER_CERTIFICATE"))));
        QTRY_COMPARE(client2.state(), QMqttClient::Connected);

        client2.disconnectFromHost();
        QTRY_COMPARE(client2.state(), QMqttClient::Disconnected);
#endif

        client2.setPort(2883);
        client2.connectToHost();
        QTRY_COMPARE(client2.state(), QMqttClient::Connected);

        client2.disconnectFromHost();
        QTRY_COMPARE(client2.state(), QMqttClient::Disconnected);
    }
#endif
}

QTEST_MAIN(Tst_QMqttClient)

#include "tst_qmqttclient.moc"
