// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qmqttsubscriptionproperties.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMqttSubscriptionProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttSubscriptionProperties class represents configuration
    options a QMqttClient can pass to the server when subscribing to a
    topic filter.

    \note Subscription properties are part of the MQTT 5.0 specification and
    cannot be used when connecting with a lower protocol level. See
    QMqttClient::ProtocolVersion for more information.
*/

/*!
    \class QMqttUnsubscriptionProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttUnsubscriptionProperties class represents configuration
    options a QMqttClient can pass to the server when unsubscribing from a
    topic filter.

    \note Unsubscription properties are part of the MQTT 5.0 specification and
    cannot be used when connecting with a lower protocol level. See
    QMqttClient::ProtocolVersion for more information.
*/

class QMqttSubscriptionPropertiesData : public QSharedData
{
public:
    quint32 subscriptionIdentifier{0};
    QMqttUserProperties userProperties;
    bool noLocal{false};
};

class QMqttUnsubscriptionPropertiesData : public QSharedData
{
public:
    QMqttUserProperties userProperties;
};

/*!
    \internal
*/
QMqttSubscriptionProperties::QMqttSubscriptionProperties() : data(new QMqttSubscriptionPropertiesData)
{

}

/*!
    \internal
*/
QMqttSubscriptionProperties::QMqttSubscriptionProperties(const QMqttSubscriptionProperties &) = default;

QMqttSubscriptionProperties &QMqttSubscriptionProperties::operator=(const QMqttSubscriptionProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttSubscriptionProperties::~QMqttSubscriptionProperties() = default;

/*!
    Returns the user specified properties.
*/
QMqttUserProperties QMqttSubscriptionProperties::userProperties() const
{
    return data->userProperties;
}

/*!
    Sets the user properties to \a user.
*/
void QMqttSubscriptionProperties::setUserProperties(const QMqttUserProperties &user)
{
    data->userProperties = user;
}

/*!
    Returns the subscription identifier used to describe this subscription.
*/
quint32 QMqttSubscriptionProperties::subscriptionIdentifier() const
{
    return data->subscriptionIdentifier;
}

/*!
    Sets the subscription identifier to \a id.
*/
void QMqttSubscriptionProperties::setSubscriptionIdentifier(quint32 id)
{
    data->subscriptionIdentifier = id;
}

/*!
 * \since 6.4
 * Returns true if the subscription shall not receive local messages on the same topic.
 */
bool QMqttSubscriptionProperties::noLocal() const
{
    return data->noLocal;
}

/*!
 * \since 6.4
 * Sets the subscription option to not receive local message.
 * When a client publishes a message with the same topic as an existing local subscription
 * the server by default sends the message back to the client. If \a noloc is set to true
 * the broker will not send any message the same client has published.
 */
void QMqttSubscriptionProperties::setNoLocal(bool noloc)
{
    data->noLocal = noloc;
}

/*!
    \internal
*/
QMqttUnsubscriptionProperties::QMqttUnsubscriptionProperties() : data(new QMqttUnsubscriptionPropertiesData)
{
}

/*!
    \internal
*/
QMqttUnsubscriptionProperties &QMqttUnsubscriptionProperties::operator=(const QMqttUnsubscriptionProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

/*!
    Returns the user specified properties.
*/
QMqttUserProperties QMqttUnsubscriptionProperties::userProperties() const
{
    return data->userProperties;
}

/*!
    Sets the user properties to \a user.
*/
void QMqttUnsubscriptionProperties::setUserProperties(const QMqttUserProperties &user)
{
    data->userProperties = user;
}

QMqttUnsubscriptionProperties::~QMqttUnsubscriptionProperties() = default;

QMqttUnsubscriptionProperties::QMqttUnsubscriptionProperties(const QMqttUnsubscriptionProperties &) = default;

QT_END_NAMESPACE
