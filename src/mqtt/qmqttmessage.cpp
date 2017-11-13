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

#include "qmqttmessage.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMqttMessage

    \inmodule QtMqtt
    \brief The QMqttMessage class provides information about a message received
    from a message broker based on a subscription.

    An MQTT message is created inside the module and returned via the
    \l QMqttSubscription::messageReceived() signal.
*/

/*!
    \property QMqttMessage::topic
    \brief This property holds the topic of a message.

    In case a wildcard has been used for a subscription, describes the topic
    matching this subscription. This property never contains wildcards.
*/

/*!
    \property QMqttMessage::payload
    \brief This property holds the payload of a message.
*/

/*!
    \property QMqttMessage::id
    \brief This property holds the ID of the message.

    IDs are used for messages with a QoS level above zero.
*/

/*!
    \property QMqttMessage::qos
    \brief This property holds the QoS level of a message.
*/

/*!
    \property QMqttMessage::duplicate
    \brief This property holds whether the message is a duplicate.

    Duplicate messages indicate that the message has been sent earlier, but it
    has not been confirmed yet. Hence, the broker assumes that it needs to
    resend to verify the transport of the message itself. Duplicate messages
    can only occur if the QoS level is one or two.
*/

/*!
    \property QMqttMessage::retain
    \brief This property holds whether the message has been retained.

    A retained message is kept on the broker for future clients to subscribe.
    Consequently, a retained message has been created previously and is not a
    live update. A broker can store only one retained message per topic.
*/

QByteArray QMqttMessage::payload() const
{
    return m_payload;
}

quint8 QMqttMessage::qos() const
{
    return m_qos;
}

quint16 QMqttMessage::id() const
{
    return m_id;
}

QMqttTopicName QMqttMessage::topic() const
{
    return m_topic;
}

bool QMqttMessage::duplicate() const
{
    return m_duplicate;
}

bool QMqttMessage::retain() const
{
    return m_retain;
}

QMqttMessage::QMqttMessage(const QMqttTopicName &topic, const QByteArray &content, quint16 id, quint8 qos, bool dup, bool retain)
    : m_topic(topic)
    , m_payload(content)
    , m_id(id)
    , m_qos(qos)
    , m_duplicate(dup)
    , m_retain(retain)
{
}

QT_END_NAMESPACE
