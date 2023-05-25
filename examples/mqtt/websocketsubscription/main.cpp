// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "clientsubscription.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QLoggingCategory>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Websocket MQTT subscription tool"_s);
    auto help = parser.addHelpOption();

    // Use http://www.hivemq.com/demos/websocket-client/ in browser to publish
    QCommandLineOption urlOption(QStringList{ u"host"_s, u"url"_s, u"broker"_s },
                                 u"Host to connect to, eg ws://broker.hivemq.com:8000/mqtt"_s,
                                 u"host"_s);
    parser.addOption(urlOption);

    QCommandLineOption subscriptionOption(QStringList{ u"t"_s, u"topic"_s },
                                          u"Topic to subscribe to"_s, u"topic"_s);
    parser.addOption(subscriptionOption);

    QCommandLineOption debugOption(QStringList{ u"d"_s, u"debug"_s }, u"Enable Debug mode"_s);
    parser.addOption(debugOption);

    QCommandLineOption versionOption(QStringList{ u"v"_s, u"version"_s },
                                     u"MQTT protocol version.\n3: MQTT 3.1\n4: MQTT 3.1.1"_s,
                                     u"version"_s, u"3"_s);
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
