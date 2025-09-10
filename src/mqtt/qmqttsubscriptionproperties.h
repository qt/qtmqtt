// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

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

    bool noLocal() const;
    void setNoLocal(bool noloc);
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
