// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "clientsubscription.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QLoggingCategory>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription(QLatin1String("Websocket MQTT subscription tool"));
    auto help = parser.addHelpOption();

    // Use http://www.hivemq.com/demos/websocket-client/ in browser to publish
    QCommandLineOption urlOption(QStringList() << "host" << "url" << "broker",
                                 QStringLiteral("Host to connect to, eg ws://broker.hivemq.com:8000/mqtt"),
                                 "host");
    parser.addOption(urlOption);

    QCommandLineOption subscriptionOption(QStringList() << "t" << "topic",
                                          QStringLiteral("Topic to subscribe to"), "topic");
    parser.addOption(subscriptionOption);

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                   QStringLiteral("Enable Debug mode"));
    parser.addOption(debugOption);

    QCommandLineOption versionOption(QStringList() << "v" << "version",
                                     QStringLiteral("MQTT protocol version.\n3: MQTT 3.1\n4: MQTT 3.1.1"),
                                     "version", "3");
    parser.addOption(versionOption);

    parser.process(a.arguments());

    const QString debugLog = QString::fromLatin1("qtdemo.websocket.mqtt*=%1").arg(
                                parser.isSet(debugOption) ? "true" : "false");
    QLoggingCategory::setFilterRules(debugLog);

    ClientSubscription clientsub;
    clientsub.setUrl(QUrl(parser.value(urlOption)));
    clientsub.setTopic(parser.value(subscriptionOption));

    const QString versionString = parser.value(versionOption);

    if (versionString == "4") {
        clientsub.setVersion(4);
    } else if (versionString == "3") {
        clientsub.setVersion(3);
    } else {
        qInfo() << "Unknown MQTT version";
        return -2;
    }

    clientsub.connectAndSubscribe();
    return a.exec();
}
