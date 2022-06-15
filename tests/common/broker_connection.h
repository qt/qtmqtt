// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QString>
#include <QtMqtt/QMqttClient>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QTest>

QString invokeOrInitializeBroker(QProcess *gBrokerProcess)
{
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

Q_DECLARE_METATYPE(QMqttClient::ProtocolVersion)
#define VersionClient(MQTTVERSION, CLIENTNAME) QMqttClient CLIENTNAME; CLIENTNAME.setProtocolVersion(MQTTVERSION)

#define DefaultVersionTestData(FUNCTION) \
void FUNCTION() \
{ \
    QTest::addColumn<QMqttClient::ProtocolVersion>("mqttVersion"); \
    QTest::newRow("V3.1.1") << QMqttClient::MQTT_3_1_1; \
    QTest::newRow("V5.0.0") << QMqttClient::MQTT_5_0; \
}
