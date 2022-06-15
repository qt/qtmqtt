// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "clientsubscription.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(lcWebSocketMqtt, "qtdemo.websocket.mqtt")

ClientSubscription::ClientSubscription(QObject *parent) : QObject(parent)
{
    connect(this, &ClientSubscription::errorOccured, qApp, &QCoreApplication::quit);
}

void ClientSubscription::setUrl(const QUrl &url)
{
    m_url = url;
}

void ClientSubscription::setTopic(const QString &topic)
{
    m_topic = topic;
}

void ClientSubscription::setVersion(int v)
{
    m_version = v;
}

void ClientSubscription::connectAndSubscribe()
{
    qCDebug(lcWebSocketMqtt) << "Connecting to broker at " << m_url;

    m_device.setUrl(m_url);
    m_device.setProtocol(m_version == 3 ? "mqttv3.1" : "mqtt");

    connect(&m_device, &WebSocketIODevice::socketConnected, this, [this]() {
        qCDebug(lcWebSocketMqtt) << "WebSocket connected, initializing MQTT connection.";

        m_client.setProtocolVersion(m_version == 3 ? QMqttClient::MQTT_3_1 : QMqttClient::MQTT_3_1_1);
        m_client.setTransport(&m_device, QMqttClient::IODevice);

        connect(&m_client, &QMqttClient::connected, this, [this]() {
            qCDebug(lcWebSocketMqtt) << "MQTT connection established";

            m_subscription = m_client.subscribe(m_topic);
            if (!m_subscription) {
                qDebug() << "Failed to subscribe to " << m_topic;
                emit errorOccured();
            }

            connect(m_subscription, &QMqttSubscription::stateChanged,
                    [](QMqttSubscription::SubscriptionState s) {
                qCDebug(lcWebSocketMqtt) << "Subscription state changed:" << s;
            });

            connect(m_subscription, &QMqttSubscription::messageReceived,
                    [this](QMqttMessage msg) {
                handleMessage(msg.payload());
            });
        });

        m_client.connectToHost();
    });
    if (!m_device.open(QIODevice::ReadWrite))
        qDebug() << "Could not open socket device";
}

void ClientSubscription::handleMessage(const QByteArray &msgContent)
{
    // Should happen when the internal device has ready read?
    qInfo() << "New message:" << msgContent;
}
