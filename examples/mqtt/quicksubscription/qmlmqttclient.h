// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef QMLMQTTCLIENT_H
#define QMLMQTTCLIENT_H

#include <QtCore/QMap>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttSubscription>

#include <QtQml/qqml.h>

class QmlMqttClient;

class QmlMqttSubscription : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QMqttTopicFilter topic MEMBER m_topic NOTIFY topicChanged)
    QML_UNCREATABLE("Not intended to be creatable")
public:
    QmlMqttSubscription(QMqttSubscription *s, QmlMqttClient *c);
    ~QmlMqttSubscription();

Q_SIGNALS:
    void topicChanged(QString);
    void messageReceived(const QString &msg);

public slots:
    void handleMessage(const QMqttMessage &qmsg);

private:
    Q_DISABLE_COPY(QmlMqttSubscription)
    QMqttSubscription *sub;
    QmlMqttClient *client;
    QMqttTopicFilter m_topic;
};

class QmlMqttClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString hostname READ hostname WRITE setHostname NOTIFY hostnameChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QMqttClient::ClientState state READ state WRITE setState NOTIFY stateChanged)
    QML_NAMED_ELEMENT(MqttClient)
    QML_EXTENDED_NAMESPACE(QMqttClient)
public:

    QmlMqttClient(QObject *parent = nullptr);

    Q_INVOKABLE void connectToHost();
    Q_INVOKABLE void disconnectFromHost();
    Q_INVOKABLE QmlMqttSubscription *subscribe(const QString &topic);

    const QString hostname() const;
    void setHostname(const QString &newHostname);

    int port() const;
    void setPort(int newPort);

    const QMqttClient::ClientState state() const;
    void setState(const QMqttClient::ClientState &newState);

signals:
    void hostnameChanged();
    void portChanged();

    void stateChanged();

private:
    Q_DISABLE_COPY(QmlMqttClient)
    QMqttClient m_client;
};

#endif // QMLMQTTCLIENT_H
