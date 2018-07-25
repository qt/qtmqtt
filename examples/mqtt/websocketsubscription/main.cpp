/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
