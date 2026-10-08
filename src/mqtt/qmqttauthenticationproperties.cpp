// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qmqttauthenticationproperties.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMqttAuthenticationProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttAuthenticationProperties class represents configuration
    options during the authentication process.

    \note Authentication properties are part of the MQTT 5.0 specification and
    cannot be used when connecting with a lower protocol level. See
    QMqttClient::ProtocolVersion for more information.
*/

class QMqttAuthenticationPropertiesData : public QSharedData
{
public:
    QString authenticationMethod;
    QByteArray authenticationData;
    QString reason;
    QMqttUserProperties userProperties;
};

/*!
    \internal
*/
QMqttAuthenticationProperties::QMqttAuthenticationProperties() : data(new QMqttAuthenticationPropertiesData)
{

}

/*!
    \internal
*/
QMqttAuthenticationProperties::QMqttAuthenticationProperties(const QMqttAuthenticationProperties &) = default;

QMqttAuthenticationProperties &QMqttAuthenticationProperties::operator=(const QMqttAuthenticationProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttAuthenticationProperties::~QMqttAuthenticationProperties() = default;

/*!
    Returns the authentication method.
*/
QString QMqttAuthenticationProperties::authenticationMethod() const
{
    return data->authenticationMethod;
}

/*!
    Sets the authentication method to \a method.
*/
void QMqttAuthenticationProperties::setAuthenticationMethod(const QString &method)
{
    data->authenticationMethod = method;
}

/*!
  Returns the authentication data
*/
QByteArray QMqttAuthenticationProperties::authenticationData() const
{
    return data->authenticationData;
}

/*!
    Sets the authentication data to \a adata.

    Authentication data can only be used if an authentication method has
    been specified.

    \sa authenticationMethod()
*/
void QMqttAuthenticationProperties::setAuthenticationData(const QByteArray &adata)
{
    data->authenticationData = adata;
}

/*!
    Returns the reason string. The reason string specifies the reason for
    a disconnect.
*/
QString QMqttAuthenticationProperties::reason() const
{
    return data->reason;
}

/*!
    Sets the reason string to \a r.
*/
void QMqttAuthenticationProperties::setReason(const QString &r)
{
    data->reason = r;
}

/*!
    Returns the user properties.
*/
QMqttUserProperties QMqttAuthenticationProperties::userProperties() const
{
    return data->userProperties;
}

/*!
    Sets the user properties to \a user.
*/
void QMqttAuthenticationProperties::setUserProperties(const QMqttUserProperties &user)
{
    data->userProperties = user;
}

QT_END_NAMESPACE
