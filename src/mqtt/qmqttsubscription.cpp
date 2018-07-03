/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtMqtt module.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#include "qmqttsubscription.h"
#include "qmqttsubscription_p.h"
#include "qmqttclient.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMqttSubscription

    \inmodule QtMqtt
    \brief The QMqttSubscription class receives notifications from an MQTT
    broker about a specified topic.
*/

/*!
    \property QMqttSubscription::state
    \brief This property holds the state of the subscription.
*/

/*!
    \property QMqttSubscription::qos
    \brief This property holds the QoS level at which the subscription has been
    made.

    The QoS level of the subscription specifies the \e maximum QoS level at
    which the client will receive messages. The publisher can still send
    messages at a lower level.
*/

/*!
    \property QMqttSubscription::topic
    \brief This property holds the topic of the subscription.
*/

/*!
    \enum QMqttSubscription::SubscriptionState

    This enum type describes the states a subscription can have.

    \value Unsubscribed
           The topic has been unsubscribed from.
    \value SubscriptionPending
           A request for a subscription has been sent, but is has not been
           confirmed by the broker yet.
    \value Subscribed
           The subscription was successful and messages will be received.
    \value UnsubscriptionPending
           A request to unsubscribe from a topic has been sent, but it has not
           been confirmed by the broker yet.
    \value Error
           An error occured.
*/

/*!
    \fn QMqttSubscription::messageReceived(QMqttMessage msg)

    This signal is emitted when the new message \a msg has been received.
*/

QMqttSubscription::QMqttSubscription(QObject *parent) : QObject(*(new QMqttSubscriptionPrivate), parent)
{

}

/*!
    Deletes a subscription. If the \l topic was not already unsubscribed from,
    it will be unsubscribed from automatically.
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

QMqttTopicFilter QMqttSubscription::topic() const
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
    Unsubscribes from \l topic.

    \note This might affect all shared pointer instances returned by
    \l QMqttClient::subscribe().
*/
void QMqttSubscription::unsubscribe()
{
    Q_D(QMqttSubscription);
    d->m_client->unsubscribe(d->m_topic);
    setState(Unsubscribed);
}

void QMqttSubscription::setTopic(const QMqttTopicFilter &topic)
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
