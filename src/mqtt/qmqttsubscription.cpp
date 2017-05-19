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

#include "qmqttsubscription.h"
#include "qmqttsubscription_p.h"
#include <QtMqtt/QMqttClient>

QT_BEGIN_NAMESPACE

/*!
    \class QMqttSubscription

    \inmodule QtMqtt
    \brief The QMqttSubscription class receives notifications from a MQTT broker about a
           specified topic.
*/

/*!
    \property QMqttSubscription::state
    \brief The state of the subscription
*/

/*!
    \property QMqttSubscription::qos
    \brief The QoS Level the subscription has been made with.

    The QoS Level of the subscription specifies the \b maximum QoS level the subscription
    will receive messages. The publisher can still send messages with a lower level.
*/

/*!
    \property QMqttSubscription::topic
    \brief The topic of the subscription.
*/

/*!
    \enum QMqttSubscription::SubscriptionState

    Describes the states a subscription can have.

    \value Unsubscribed
           The QMqttSubscription has unsubcribed from this topic.
    \value SubscriptionPending
           A request for a subscription has been sent, but is not confirmed by the broker yet.
    \value Subscribed
           The subscription has been successful and messages will be received.
    \value UnsubscriptionPending
           A requestion to unsubscribe to a topic has been sent, but is not confirmed by the broker
           yet.
    \value Error
           An error occured.
*/

/*!
    \fn QMqttSubscription::messageReceived(QByteArray msg, QString topic)

    This signal is emitted when a new message \a msg has been received. When a subscription
    contains a wildcard, \a topic provides the exact topic of the message matching the criteria
    for the subscription.
*/

QMqttSubscription::QMqttSubscription(QObject *parent) : QObject(*(new QMqttSubscriptionPrivate), parent)
{

}

/*!
    Deletes a subscription. If the subscription did not unsubscribe from topic(), then it
    will automatically do so.
*/
QMqttSubscription::~QMqttSubscription()
{
    Q_D(const QMqttSubscription);
    if (d->m_state == Subscribed)
        unsubscribe();
}

QMqttSubscription::SubscriptionState QMqttSubscription::state() const
{
    Q_D(const QMqttSubscription);
    return d->m_state;
}

QString QMqttSubscription::topic() const
{
    Q_D(const QMqttSubscription);
    return d->m_topic;
}

quint8 QMqttSubscription::qos() const
{
    Q_D(const QMqttSubscription);
    return d->m_qos;
}

void QMqttSubscription::setState(QMqttSubscription::SubscriptionState state)
{
    Q_D(QMqttSubscription);
    if (d->m_state == state)
        return;

    d->m_state = state;
    emit stateChanged(d->m_state);
}

/*!
    Unsubscribes from \l topic. Note that this might affect all shared pointer instance
    returned by \l QMqttClient::subscribe()
*/
void QMqttSubscription::unsubscribe()
{
    Q_D(QMqttSubscription);
    d->m_client->unsubscribe(d->m_topic);
    setState(Unsubscribed);
}

void QMqttSubscription::setTopic(const QString &topic)
{
    Q_D(QMqttSubscription);
    d->m_topic = topic;
}

void QMqttSubscription::setClient(QMqttClient *client)
{
    Q_D(QMqttSubscription);
    d->m_client = client;
}

void QMqttSubscription::setQos(quint8 qos)
{
    Q_D(QMqttSubscription);
    d->m_qos = qos;
}

QMqttSubscriptionPrivate::QMqttSubscriptionPrivate()
    : QObjectPrivate()
{

}

QT_END_NAMESPACE
