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

#include "qmqttmessage.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMqttMessage

    \inmodule QtMqtt
    \brief The QMqttMessage class provides all required information about a message received
           from a message broker based on a subscription.

    A QMqttMessage only is created inside the module and returned via the
    \l QMqttSubscription::messageReceived() signal.
*/

/*!
    \property QMqttMessage::topic
    \brief The topic of a message.

    In case a wildcard has been used for a subscription, QMqttMessage::topic describes the
    topic matching this subscription. This property never contains any wildcard.
*/

/*!
    \property QMqttMessage::payload
    \brief The payload of a message.
*/

/*!
    \property QMqttMessage::id
    \brief The id of the message.

    Ids are used for messages with a QoS level above zero.
*/

/*!
    \property QMqttMessage::qos
    \brief The QoS level of a message.
*/

/*!
    \property QMqttMessage::duplicate
    \brief Specifies whether the message is a duplicate.

    Duplicate messages indicate that the message has been sent earlier, but has not been
    confirmed yet. Hence, the broker assumes that it needs to resend to verify transport of
    the message itself. Duplicate messages can only occur in the situation of a QoS level of
    one or two.
*/

/*!
    \property QMqttMessage::retain
    \brief Specify whether the message has been retained.

    A retained message is kept on the broker for future clients to subscribe. Consequently,
    a retained message has been created previously and is not a live update. A broker can only
    store one retained message per topic.
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

QString QMqttMessage::topic() const
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

QMqttMessage::QMqttMessage(const QString &topic, const QByteArray &content, quint16 id, quint8 qos, bool dup, bool retain)
    : m_topic(topic)
    , m_payload(content)
    , m_id(id)
    , m_qos(qos)
    , m_duplicate(dup)
    , m_retain(retain)
{
}

QT_END_NAMESPACE
