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

#include "qmqttauthenticationproperties.h"

QT_BEGIN_NAMESPACE

class QMqttAuthenticationPropertiesData : public QSharedData
{
public:
    QString authenticationMethod;
    QByteArray authenticationData;
    QString reason;
    QMqttUserProperties userProperties;
};

QMqttAuthenticationProperties::QMqttAuthenticationProperties() : data(new QMqttAuthenticationPropertiesData)
{

}

QMqttAuthenticationProperties::QMqttAuthenticationProperties(const QMqttAuthenticationProperties &) = default;

QMqttAuthenticationProperties &QMqttAuthenticationProperties::operator=(const QMqttAuthenticationProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttAuthenticationProperties::~QMqttAuthenticationProperties() = default;

QString QMqttAuthenticationProperties::authenticationMethod() const
{
    return data->authenticationMethod;
}

void QMqttAuthenticationProperties::setAuthenticationMethod(const QString &method)
{
    data->authenticationMethod = method;
}

QByteArray QMqttAuthenticationProperties::authenticationData() const
{
    return data->authenticationData;
}

void QMqttAuthenticationProperties::setAuthenticationData(const QByteArray &adata)
{
    data->authenticationData = adata;
}

QString QMqttAuthenticationProperties::reason() const
{
    return data->reason;
}

void QMqttAuthenticationProperties::setReason(const QString &r)
{
    data->reason = r;
}

QMqttUserProperties QMqttAuthenticationProperties::userProperties() const
{
    return data->userProperties;
}

void QMqttAuthenticationProperties::setUserProperties(const QMqttUserProperties &user)
{
    data->userProperties = user;
}

QT_END_NAMESPACE
