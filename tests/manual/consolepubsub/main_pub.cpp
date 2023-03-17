// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "configuration.h"

#include <QCoreApplication>
#include <QMqttClient>
#include <QSslSocket>
#include <QTimer>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("qtmqtt_pub"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0"));

    // Create the client
    Configuration description;
    auto *client = createClientWithConfiguration(&a, &description, true);

    if (!client)
        return -1;

    a.connect(client, &QMqttClient::errorChanged, [&client](const QMqttClient::ClientError e) {
        if (e == QMqttClient::NoError)
            return;

        qWarning() << "Error Occurred:" << e << " Client state:" << client->state();
        client->disconnectFromHost();
    });

    a.connect(client, &QMqttClient::messageSent, [&client] (quint32 id) {
        qInfo() << "Message with ID:" << id << " sent";
        client->disconnectFromHost();
    });

    a.connect(client, &QMqttClient::stateChanged, [&client] (QMqttClient::ClientState s) {
        if (s == QMqttClient::Disconnected) {
            client->deleteLater();
            qApp->quit();
        }
    });

    a.connect(client, &QMqttClient::connected, [&client, description]() {
        qInfo() << "Message:";
        qInfo() << "  Topic:" << description.topic << " QoS:" << description.qos
                << " Retain:" << description.retain;
        qInfo() << "  Content: " << description.content.left(50);
        client->publish(description.topic,
                        description.content,
                        description.qos,
                        description.retain);
        if (description.qos == 0)// 0 has no acknowledgment
            QTimer::singleShot(500, client, &QMqttClient::disconnectFromHost);
    });

#ifndef QT_NO_SSL
    if (description.useEncryption)
        client->connectToHostEncrypted(description.sslConfiguration);
    else
#endif
        client->connectToHost();

    return a.exec();
}
