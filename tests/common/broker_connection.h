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

#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QString>
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
    if (python.isEmpty()) {
        const QString usrPython = QFile::decodeName("/usr/bin/python3");
        if (QFile::exists(usrPython))
            python = usrPython;
        else // We assume it to be in the path
            python = QLatin1String("python3");
    }
    const QStringList arguments = QStringList() << brokerLocation;
    gBrokerProcess->start(python, arguments);
    if (!gBrokerProcess->waitForStarted())
        qFatal("Could not start MQTT test broker.");

    // Give the server some time to initialize, not only launch
    QTcpSocket socket;
    socket.connectToHost(QLatin1String("localhost"), 1883);

    if (socket.waitForConnected(5000))
        return QLatin1String("localhost");

    qWarning("Could not launch MQTT test broker.");
    return QString();
}
