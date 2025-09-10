// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QMQTTSUBSCRIPTION_H
#define QMQTTSUBSCRIPTION_H

#include <QtMqtt/qmqttglobal.h>
#include <QtMqtt/qmqttmessage.h>
#include <QtMqtt/qmqtttopicfilter.h>

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
    Q_PROPERTY(QMqttTopicFilter topic READ topic)
    Q_PROPERTY(QString reason READ reason)
    Q_PROPERTY(QMqtt::ReasonCode reasonCode READ reasonCode)
    Q_PROPERTY(bool sharedSubscription READ isSharedSubscription)
    Q_PROPERTY(QString sharedSubscriptionName READ sharedSubscriptionName)
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
    QMqttTopicFilter topic() const;
    quint8 qos() const;
    QString reason() const;
    QMqtt::ReasonCode reasonCode() const;
    QMqttUserProperties userProperties() const;

    bool isSharedSubscription() const;
    QString sharedSubscriptionName() const;

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
    void setTopic(const QMqttTopicFilter &topic);
    void setClient(QMqttClient *client);
    void setQos(quint8 qos);
    void setSharedSubscription(bool s);
    void setSharedSubscriptionName(const QString &name);
    friend class QMqttConnection;
    friend class QMqttClient;
    explicit QMqttSubscription(QObject *parent = nullptr);
};

QT_END_NAMESPACE

#endif // QMQTTSUBSCRIPTION_H
