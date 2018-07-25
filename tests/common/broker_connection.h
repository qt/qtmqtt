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

    // MQTT5 tests use the same configuration as mosquitto
    const QString configuration = QLatin1String("localhost_testing.conf");
    const QDir brokerDir = QFileInfo(brokerLocation).absoluteDir();
    if (brokerDir.exists(configuration)) {
        arguments << QLatin1String("-c") << QDir::toNativeSeparators(brokerDir.absoluteFilePath(configuration));
        // Configuration files use relative paths, hence the working directory of the broker
        // process needs to be set correctly
        gBrokerProcess->setWorkingDirectory(brokerDir.absolutePath());
    }

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
