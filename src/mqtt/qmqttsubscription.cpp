/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Mqtt module of the Qt Toolkit.
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
****************************************************************************/
#include "qmqttsubscription.h"
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
    \fn QMqttSubscription::messageReceived(QByteArray msg)

    This signal is emitted when a new message \a msg has been received.
*/

QMqttSubscription::QMqttSubscription(QObject *parent) : QObject(parent)
{

}

/*!
    Deletes a subscription. If the subscription did not unsubscribe from topic(), then it
    will automatically do so.
*/
QMqttSubscription::~QMqttSubscription()
{
    if (m_state == Subscribed)
        unsubscribe();
}

QMqttSubscription::SubscriptionState QMqttSubscription::state() const
{
    return m_state;
}

QString QMqttSubscription::topic() const
{
    return m_topic;
}

quint8 QMqttSubscription::qos() const
{
    return m_qos;
}

void QMqttSubscription::setState(QMqttSubscription::SubscriptionState state)
{
    if (m_state == state)
        return;

    m_state = state;
    emit stateChanged(m_state);
}

/*!
    Unsubscribes from \l topic. Note that this might affect all shared pointer instance
    returned by \l QMqttClient::subscribe()
*/
void QMqttSubscription::unsubscribe()
{
    m_client->unsubscribe(m_topic);
    setState(Unsubscribed);
}

QT_END_NAMESPACE
