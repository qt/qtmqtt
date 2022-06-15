// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CLIENTSUBSCRIPTION_H
#define CLIENTSUBSCRIPTION_H

#include "websocketiodevice.h"

#include <QtCore/QObject>
#include <QtMqtt/QMqttClient>
#include <QtWebSockets/QWebSocket>

class ClientSubscription : public QObject
{
    Q_OBJECT
public:
    ClientSubscription(QObject *parent = nullptr);

    void setUrl(const QUrl &url); // ie ws://broker.hivemq.com:8000/mqtt
    void setTopic(const QString &topic);
    void setVersion(int v);
signals:
    void messageReceived(QByteArray);
    void errorOccured();

public slots:
    void connectAndSubscribe();
    void handleMessage(const QByteArray &msgContent);

private:
    QMqttClient m_client;
    QMqttSubscription *m_subscription;
    QUrl m_url;
    QString m_topic;
    WebSocketIODevice m_device;
    int m_version;
};

#endif // CLIENTSUBSCRIPTION_H
