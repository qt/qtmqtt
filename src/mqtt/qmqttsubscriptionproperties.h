/******************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
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
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

#ifndef QMQTTSUBSCRIPTIONPROPERTIES_H
#define QMQTTSUBSCRIPTIONPROPERTIES_H

#include <QtMqtt/qmqttglobal.h>
#include <QtMqtt/qmqtttype.h>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QMqttSubscriptionPropertiesData;
class QMqttUnsubscriptionPropertiesData;

class Q_MQTT_EXPORT QMqttSubscriptionProperties
{
public:
    QMqttSubscriptionProperties();
    QMqttSubscriptionProperties(const QMqttSubscriptionProperties &);
    QMqttSubscriptionProperties &operator=(const QMqttSubscriptionProperties &);
    ~QMqttSubscriptionProperties();

    QMqttUserProperties userProperties() const;
    void setUserProperties(const QMqttUserProperties &user);

    quint32 subscriptionIdentifier() const;
    void setSubscriptionIdentifier(quint32 id);
private:
    QSharedDataPointer<QMqttSubscriptionPropertiesData> data;
};

class Q_MQTT_EXPORT QMqttUnsubscriptionProperties
{
public:
    QMqttUnsubscriptionProperties();
    QMqttUnsubscriptionProperties(const QMqttUnsubscriptionProperties &);
    QMqttUnsubscriptionProperties &operator=(const QMqttUnsubscriptionProperties &rhs);
    ~QMqttUnsubscriptionProperties();

    QMqttUserProperties userProperties() const;
    void setUserProperties(const QMqttUserProperties &user);

private:
    QSharedDataPointer<QMqttUnsubscriptionPropertiesData> data;
};

QT_END_NAMESPACE

#endif // QMQTTSUBSCRIPTIONPROPERTIES_H
