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

QByteArray QMqttMessage::content() const
{
    return m_content;
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
    , m_content(content)
    , m_id(id)
    , m_qos(qos)
    , m_duplicate(dup)
    , m_retain(retain)
{
}
