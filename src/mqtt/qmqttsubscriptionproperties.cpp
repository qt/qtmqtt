/******************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
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

#include "qmqttsubscriptionproperties.h"

QT_BEGIN_NAMESPACE

class QMqttSubscriptionPropertiesData : public QSharedData
{
public:
    quint32 subscriptionIdentifier{0};
    QMqttUserProperties userProperties;
};

class QMqttUnsubscriptionPropertiesData : public QSharedData
{
public:
    QMqttUserProperties userProperties;
};

QMqttSubscriptionProperties::QMqttSubscriptionProperties() : data(new QMqttSubscriptionPropertiesData)
{

}

QMqttSubscriptionProperties::QMqttSubscriptionProperties(const QMqttSubscriptionProperties &) = default;

QMqttSubscriptionProperties &QMqttSubscriptionProperties::operator=(const QMqttSubscriptionProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttSubscriptionProperties::~QMqttSubscriptionProperties() = default;

QMqttUserProperties QMqttSubscriptionProperties::userProperties() const
{
    return data->userProperties;
}

void QMqttSubscriptionProperties::setUserProperties(const QMqttUserProperties &user)
{
    data->userProperties = user;
}

quint32 QMqttSubscriptionProperties::subscriptionIdentifier() const
{
    return data->subscriptionIdentifier;
}

void QMqttSubscriptionProperties::setSubscriptionIdentifier(quint32 id)
{
    data->subscriptionIdentifier = id;
}

QMqttUnsubscriptionProperties::QMqttUnsubscriptionProperties() : data(new QMqttUnsubscriptionPropertiesData)
{
}

QMqttUnsubscriptionProperties &QMqttUnsubscriptionProperties::operator=(const QMqttUnsubscriptionProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttUserProperties QMqttUnsubscriptionProperties::userProperties() const
{
    return data->userProperties;
}

void QMqttUnsubscriptionProperties::setUserProperties(const QMqttUserProperties &user)
{
    data->userProperties = user;
}

QMqttUnsubscriptionProperties::~QMqttUnsubscriptionProperties() = default;

QMqttUnsubscriptionProperties::QMqttUnsubscriptionProperties(const QMqttUnsubscriptionProperties &) = default;

QT_END_NAMESPACE
