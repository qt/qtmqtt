/******************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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
