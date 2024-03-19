// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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
