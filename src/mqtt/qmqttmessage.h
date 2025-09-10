// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QMQTTMESSAGE_H
#define QMQTTMESSAGE_H

#include <QtMqtt/qmqttglobal.h>
#include <QtMqtt/qmqttpublishproperties.h>
#include <QtMqtt/qmqtttopicname.h>

#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>

QT_BEGIN_NAMESPACE

class QMqttMessagePrivate;

class Q_MQTT_EXPORT QMqttMessage
{
    Q_GADGET
    Q_PROPERTY(QMqttTopicName topic READ topic CONSTANT)
    Q_PROPERTY(QByteArray payload READ payload CONSTANT)
    Q_PROPERTY(quint16 id READ id CONSTANT)
    Q_PROPERTY(quint8 qos READ qos CONSTANT)
    Q_PROPERTY(bool duplicate READ duplicate CONSTANT)
    Q_PROPERTY(bool retain READ retain CONSTANT)

public:
    QMqttMessage();
    QMqttMessage(const QMqttMessage& other);
    ~QMqttMessage();

    QMqttMessage& operator=(const QMqttMessage &other);
    bool operator==(const QMqttMessage &other) const;
    inline bool operator!=(const QMqttMessage &other) const;

    const QByteArray &payload() const;
    quint8 qos() const;
    quint16 id() const;
    QMqttTopicName topic() const;
    bool duplicate() const;
    bool retain() const;

    QMqttPublishProperties publishProperties() const;
private:
    friend class QMqttConnection;
    QMqttMessage(const QMqttTopicName &topic, const QByteArray &payload,
                          quint16 id, quint8 qos,
                          bool dup, bool retain);
    QExplicitlySharedDataPointer<QMqttMessagePrivate> d;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QMqttMessage)

#endif // QMQTTMESSAGE_H
