// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtMqtt/QMqttClient>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QTest>

#ifdef QT_MQTT_WITH_WEBSOCKETS
#include <QWebSocket>
#include <QWebSocketHandshakeOptions>
#endif

QString invokeOrInitializeBroker(QProcess *gBrokerProcess)
{
#if defined(MQTT_TEST_BROKER)
    return QString::fromLatin1(MQTT_TEST_BROKER);
#endif
    const QString broker = qgetenv("MQTT_TEST_BROKER");
    if (broker.size()) {
        qDebug("Overwriting default test broker");
        return broker;
    }

    const QString brokerLocation = QFile::decodeName(qgetenv("MQTT_TEST_BROKER_LOCATION"));
    if (brokerLocation.isEmpty())
        qFatal("Unknown location to start MQTT test broker.");

    // Start the paho test broker
    QString python = QFile::decodeName(qgetenv("PYTHON3_PATH"));
    if (!python.isEmpty()) {
#ifdef Q_OS_WIN
        python += QLatin1String("/python.exe");
#else
        python += QLatin1String("/python3");
#endif
        if (!QFileInfo::exists(python)) {
            qWarning() << "Could not find Python at:" << python << ". Assuming it in PATH.";
            python.clear();
        }
    }

    if (python.isEmpty()) {
#ifdef Q_OS_WIN
        python = QLatin1String("python.exe");
#else
        python = QLatin1String("python3");
#endif
    }

    QStringList arguments = {brokerLocation};

    qDebug() << "Launching broker:" << python << arguments;
    gBrokerProcess->start(python, arguments);
    if (!gBrokerProcess->waitForStarted())
        qFatal("Could not start MQTT test broker.");

    const int maxTries = 6;
    // Give the server some time to initialize, not only launch.
    // Cannot use QTRY_*
    for (int tryCounter = 0; tryCounter < maxTries; ++tryCounter) {
        QTcpSocket socket;
        socket.connectToHost(QLatin1String("localhost"), 1883);

        if (socket.waitForConnected(5000))
            return QLatin1String("localhost");
        QTest::qWait(5000);
    }

    qWarning() << "Could not launch MQTT test broker.";
    return QString();
}

#if !defined(QT_NO_SSL)
QSslConfiguration makeConfig(QString certificateFile)
{
    if (certificateFile.isEmpty())
        certificateFile=QString(u":/certificate/cert.crt");

    QSslConfiguration config;
    QFile file(certificateFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << " Could not open certificate file " << certificateFile;
    } else {
        const auto bytes = file.readAll();
        const QSslCertificate certificate(bytes);

        config.addCaCertificate(certificate);
    }
    return config;
}
#endif /* !QT_NO_SSL */

int getBrokerPort(int defaultPort)
{
    if (getenv("MQTT_TEST_BROKER_PORT"))
        return atoi(getenv("MQTT_TEST_BROKER_PORT"));

#if defined (MQTT_TEST_PROTOCOL)
    return defaultPort;
#else
    return defaultPort - 1000; /* evaluates to 1883 for the old system */
#endif
}

QMqttClient::ProtocolVersion getProtocolVersion(int version)
{
#if defined (MQTT_TEST_PROTOCOL)
    if (version == 0)
        version=MQTT_TEST_PROTOCOL;
#else
    if ((version == 0) && getenv("MQTT_TEST_PROTOCOL_VERSION")) {
        switch (atoi(getenv("MQTT_TEST_PROTOCOL_VERSION"))) {
        case 3: version = QMqttClient::MQTT_3_1; break;
        case 4: version = QMqttClient::MQTT_3_1_1; break;
        case 5: version = QMqttClient::MQTT_5_0; break;
        }
    }
    if (version == 0)
        version = QMqttClient::MQTT_5_0;
#endif
    return (QMqttClient::ProtocolVersion)version;
}

class MqTTTestClientBase : public QMqttClient
{
public:
    virtual bool isTcp() const { return false; }
    virtual bool isSsl() const { return false; }
    virtual bool isWs() const { return false; }
    virtual bool isWss() const { return false; }
public:
    virtual void setPort(quint16 port) = 0;
    virtual void connectToHost() = 0;

    void setTransport(QIODevice *device, TransportType transport)
    {
        m_transportSet = true;
        QMqttClient::setTransport(device, transport);
    }
    void disconnectFromHost()
    {
        m_transportSet = false;
        QMqttClient::disconnectFromHost();
    }
protected:
    bool m_transportSet = false;
};

class MqTTClientTcp : public MqTTTestClientBase
{
public:
    MqTTClientTcp() = default;

    virtual bool isTcp() const override { return true; }
    void setPort(quint16 port) override
    {
        if (port == 0)
            port = getBrokerPort(2883);
        QMqttClient::setPort(port);
    }
    void connectToHost() override
    {
        QMqttClient::connectToHost();
    }
};

#ifdef QT_MQTT_WITH_WEBSOCKETS
class MqTTClientWs : public MqTTTestClientBase
{
public:
    MqTTClientWs() = default;

    virtual bool isWs() const override { return true; }

    void setPort(quint16 port) override
    {
        if (port == 0)
            port = getBrokerPort(9080);
        QMqttClient::setPort(port);
    }

    void connectToHost() override
    {
        if (m_transportSet)
            QMqttClient::connectToHost();
        else
            QMqttClient::connectToHostWebSocket();
    }
};
#endif

#ifdef QT_MQTT_WITH_WEBSOCKETS
class MqTTClientWss : public MqTTTestClientBase
{
public:
    MqTTClientWss()
    {
#ifndef QT_NO_SSL
        const QString origin;
        const QWebSocketProtocol::Version version = QWebSocketProtocol::VersionLatest;
        socket = new QWebSocket(origin, version, this);
#endif
    }

    virtual bool isWss() const override { return true; }

    void setPort(quint16 port) override
    {
        if (port == 0)
            port = getBrokerPort(9081);
        QMqttClient::setPort(port);
    }

    void connectToHost() override
    {
        if (m_transportSet)
            QMqttClient::connectToHost();
        else {
#ifndef QT_NO_SSL
            QUrl url(QString(u"wss://") + QMqttClient::hostname() + QString(u":") + QString::number(QMqttClient::port()));
            QWebSocketHandshakeOptions options;
            options.setSubprotocols(QStringList{ QString::fromUtf8("mqttv3.1") });
            socket->setSslConfiguration(makeConfig(QString::fromUtf8(getenv("MQTT_TEST_BROKER_CERTIFICATE"))));
            socket->open(url, options);
#endif
            QMqttClient::connectToHostWebSocketEncrypted(socket);
        }
    }

private:
    QWebSocket *socket = nullptr;
};
#endif

#ifndef QT_NO_SSL
class MqTTClientSsl : public MqTTTestClientBase
{
public:
    MqTTClientSsl() = default;

    virtual bool isSsl() const override { return true; }

    void setPort(quint16 port) override
    {
        if (port == 0)
            port = getBrokerPort(9883);
        QMqttClient::setPort(port);
    }

    void connectToHost() override
    {
        if (m_transportSet)
            QMqttClient::connectToHost();
        else {
            QMqttClient::connectToHostEncrypted(makeConfig(QString::fromUtf8(getenv("MQTT_TEST_BROKER_CERTIFICATE"))));
        }
    }
};
#endif /* !QT_NO_SSL */

Q_DECLARE_METATYPE(QMqttClient::ProtocolVersion)

#if !defined(QT_NO_SSL) && defined(QT_MQTT_WITH_WEBSOCKETS)
#define VersionClientBase(MQTTVERSION, CLIENTNAME, TestType)  \
  std::shared_ptr<MqTTTestClientBase> p##CLIENTNAME; \
  if (TestType == QString(u"Tcp")) p##CLIENTNAME = std::make_shared<MqTTClientTcp>(); \
  if (TestType == QString(u"Ws"))  p##CLIENTNAME = std::make_shared<MqTTClientWs>(); \
  if (TestType == QString(u"Wss")) p##CLIENTNAME = std::make_shared<MqTTClientWss>(); \
  if (TestType == QString(u"Ssl")) p##CLIENTNAME = std::make_shared<MqTTClientSsl>(); \
  if (!p##CLIENTNAME) p##CLIENTNAME = std::make_shared<MqTTClientTcp>(); \
  MqTTTestClientBase &CLIENTNAME = *p##CLIENTNAME; \
  CLIENTNAME.setProtocolVersion(getProtocolVersion(MQTTVERSION))
#endif

#if defined(QT_NO_SSL) && defined(QT_MQTT_WITH_WEBSOCKETS)
#define VersionClientBase(MQTTVERSION, CLIENTNAME, TestType)  \
  std::shared_ptr<MqTTTestClientBase> p##CLIENTNAME; \
  if (TestType == QString(u"Tcp")) p##CLIENTNAME = std::make_shared<MqTTClientTcp>(); \
  if (TestType == QString(u"Ws"))  p##CLIENTNAME = std::make_shared<MqTTClientWs>(); \
  if (TestType == QString(u"Wss")) p##CLIENTNAME = std::make_shared<MqTTClientWss>(); \
  if (!p##CLIENTNAME) p##CLIENTNAME = std::make_shared<MqTTClientTcp>(); \
  MqTTTestClientBase &CLIENTNAME = *p##CLIENTNAME; \
  CLIENTNAME.setProtocolVersion(getProtocolVersion(MQTTVERSION))
#endif

#if !defined(QT_NO_SSL) && !defined(QT_MQTT_WITH_WEBSOCKETS)
#define VersionClientBase(MQTTVERSION, CLIENTNAME, TestType)  \
  std::shared_ptr<MqTTTestClientBase> p##CLIENTNAME; \
  if (TestType == QString(u"Tcp")) p##CLIENTNAME = std::make_shared<MqTTClientTcp>(); \
  if (TestType == QString(u"Ssl")) p##CLIENTNAME = std::make_shared<MqTTClientSsl>(); \
  if (!p##CLIENTNAME) p##CLIENTNAME = std::make_shared<MqTTClientTcp>(); \
  MqTTTestClientBase &CLIENTNAME = *p##CLIENTNAME; \
  CLIENTNAME.setProtocolVersion(getProtocolVersion(MQTTVERSION))
#endif

#if defined(QT_NO_SSL) && !defined(QT_MQTT_WITH_WEBSOCKETS)
#define VersionClientBase(MQTTVERSION, CLIENTNAME, TestType)  \
  std::shared_ptr<MqTTTestClientBase> p##CLIENTNAME; \
  if (TestType == QString(u"Tcp")) p##CLIENTNAME = std::make_shared<MqTTClientTcp>(); \
  if (!p##CLIENTNAME) p##CLIENTNAME = std::make_shared<MqTTClientTcp>(); \
  MqTTTestClientBase &CLIENTNAME = *p##CLIENTNAME; \
  CLIENTNAME.setProtocolVersion(getProtocolVersion(MQTTVERSION))
#endif


#if defined(MQTT_TEST_TYPE)
#define VersionClient(MQTTVERSION, CLIENTNAME)  \
  VersionClientBase(MQTTVERSION, CLIENTNAME, MQTT_TEST_TYPE)
#else
#define VersionClient(MQTTVERSION, CLIENTNAME)  \
  VersionClientBase(MQTTVERSION, CLIENTNAME, qgetenv("MQTT_TEST_BROKER_TYPE"))
#endif

#ifdef MQTT_TEST_PROTOCOL
#define DefaultVersionTestData(FUNCTION) \
void FUNCTION() \
{ \
    QTest::addColumn<QMqttClient::ProtocolVersion>("mqttVersion"); \
    QTest::newRow("V0") << QMqttClient::ProtocolVersion(0); \
}
#else
#define DefaultVersionTestData(FUNCTION) \
void FUNCTION() \
{ \
    QTest::addColumn<QMqttClient::ProtocolVersion>("mqttVersion"); \
    if (getenv("MQTT_TEST_PROTOCOL_VERSION"))\
        QTest::newRow("V0") << QMqttClient::ProtocolVersion(0); \
    else { \
        QTest::newRow("V3.1.1") << QMqttClient::MQTT_3_1_1; \
        QTest::newRow("V5.0.0") << QMqttClient::MQTT_5_0; \
    } \
}
#endif
