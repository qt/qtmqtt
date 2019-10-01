/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "configuration.h"

#include <QCoreApplication>
#include <QMqttClient>
#include <QSslSocket>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("qtmqtt_sub"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    // Create the client
    Configuration description;
    auto *client = createClientWithConfiguration(&a, &description, false);

    if (!client)
        return -1;

    a.connect(client, &QMqttClient::errorChanged, [&client](const QMqttClient::ClientError e) {
        if (e == QMqttClient::NoError)
            return;

        qWarning() << "Error Occurred:" << e << " Client state:" << client->state();
        client->disconnectFromHost();
    });

    a.connect(client, &QMqttClient::stateChanged, [&client] (QMqttClient::ClientState s) {
        if (s == QMqttClient::Disconnected) {
            client->deleteLater();
            qApp->quit();
        }
    });

    a.connect(client, &QMqttClient::connected, [&client, description]() {
        auto sub = client->subscribe(description.topic, description.qos);
        client->connect(sub, &QMqttSubscription::stateChanged, [&client](QMqttSubscription::SubscriptionState s) {
            qInfo() << "Subscription state:" << s;
            if (s == QMqttSubscription::Unsubscribed)
                client->disconnectFromHost();
        });

        client->connect(sub, &QMqttSubscription::messageReceived, [](const QMqttMessage &msg) {
            qInfo() << "ID:" << msg.id()
                    << "Topic:" << msg.topic().name()
                    << "QoS:" << msg.qos()
                    << "Retain:" << msg.retain()
                    << "Duplicate:" << msg.duplicate()
                    << "Payload:" << msg.payload().left(50) << (msg.payload().size() > 50 ? "..." : "");
        });
    });

#ifndef QT_NO_SSL
    if (description.useEncryption)
        client->connectToHostEncrypted(description.sslConfiguration);
    else
#endif
        client->connectToHost();

    return a.exec();
}
