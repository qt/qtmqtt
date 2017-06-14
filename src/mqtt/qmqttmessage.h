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

#ifndef QMQTTMESSAGE_H
#define QMQTTMESSAGE_H

#include "qmqttglobal.h"

#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class Q_MQTT_EXPORT QMqttMessage
{
    Q_GADGET
    Q_PROPERTY(QString topic READ topic CONSTANT)
    Q_PROPERTY(QByteArray payload READ payload CONSTANT)
    Q_PROPERTY(quint16 id READ id CONSTANT)
    Q_PROPERTY(quint8 qos READ qos CONSTANT)
    Q_PROPERTY(bool duplicate READ duplicate CONSTANT)
    Q_PROPERTY(bool retain READ retain CONSTANT)
public:
    QByteArray payload() const;
    quint8 qos() const;
    quint16 id() const;
    QString topic() const;
    bool duplicate() const;
    bool retain() const;

private:
    friend class QMqttConnection;
    explicit QMqttMessage(const QString &topic, const QByteArray &payload,
                          quint16 id, quint8 qos,
                          bool dup, bool retain);
    QString m_topic;
    QByteArray m_payload;
    quint16 m_id;
    quint8 m_qos;
    bool m_duplicate;
    bool m_retain;
};

QT_END_NAMESPACE

#endif // QMQTTMESSAGE_H
