/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtMqtt module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#ifndef QMQTTSUBSCRIPTION_H
#define QMQTTSUBSCRIPTION_H

#include "qmqttmessage.h"

#include <QtMqtt/qmqttglobal.h>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QMqttClient;
class QMqttSubscriptionPrivate;

class Q_MQTT_EXPORT QMqttSubscription : public QObject
{
    Q_OBJECT
    Q_ENUMS(SubscriptionState)
    Q_PROPERTY(SubscriptionState state READ state NOTIFY stateChanged)
    Q_PROPERTY(quint8 qos READ qos NOTIFY qosChanged)
    Q_PROPERTY(QString topic READ topic)
public:
    ~QMqttSubscription() override;
    enum SubscriptionState {
        Unsubscribed = 0,
        SubscriptionPending,
        Subscribed,
        UnsubscriptionPending,
        Error
    };

    SubscriptionState state() const;
    QString topic() const;
    quint8 qos() const;

Q_SIGNALS:
    void stateChanged(SubscriptionState state);
    void qosChanged(quint8); // only emitted when broker provides different QoS than requested
    void messageReceived(QMqttMessage msg);

public Q_SLOTS:
    void unsubscribe();

private:
    Q_DECLARE_PRIVATE(QMqttSubscription)
    Q_DISABLE_COPY(QMqttSubscription)
    void setState(SubscriptionState state);
    void setTopic(const QString &topic);
    void setClient(QMqttClient *client);
    void setQos(quint8 qos);
    friend class QMqttConnection;
    friend class QMqttClient;
    explicit QMqttSubscription(QObject *parent = nullptr);
};

QT_END_NAMESPACE

#endif // QMQTTSUBSCRIPTION_H
