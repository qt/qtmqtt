// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QCoreApplication>
#include <QtMqtt>
#include <QtNetwork>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    const bool support = QSslSocket::supportsSsl();
    if (!support) {
        qWarning() << "This test requires a Qt build with SSL support.";
        return -1;
    }

    QSslConfiguration sslConfig;

    // You must download the current certificate from the mosquitto test page
    // http://test.mosquitto.org and place it next to the binary.
    const auto certs = QSslCertificate::fromPath("mosquitto.org.crt");

    if (certs.isEmpty()) {
        qWarning() << "Could not load certificates";
        return -2;
    }

    sslConfig.setCaCertificates(certs);

    QMqttClient client;

    client.setHostname("test.mosquitto.org");
    client.setPort(8883);

    a.connect(&client, &QMqttClient::connected, [&client]() {
        qDebug() << "MQTT Client connected, subscribing";
        auto sub = client.subscribe(QLatin1String("some/Topic/foo"), 1);
        client.connect(sub, &QMqttSubscription::stateChanged, [] (QMqttSubscription::SubscriptionState s) {
            qDebug() << "MQTT Subscription new state:" << s;
            if (s == QMqttSubscription::Subscribed)
                qInfo() << "Connection and Subscription succeeded, test done.";
        });
    });

    a.connect(&client, &QMqttClient::stateChanged, [](QMqttClient::ClientState s) {
        qDebug() << "MQTT State:" << s;
    });
    a.connect(&client, &QMqttClient::errorChanged, [](QMqttClient::ClientError e) {
        qDebug() << "MQTT Error:" << e;
    });

    client.connectToHostEncrypted(sslConfig);

    return a.exec();
}
