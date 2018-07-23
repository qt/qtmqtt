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

#include "qmqttconnectionproperties.h"

#include "qmqttconnectionproperties_p.h"

#include <limits>

QT_BEGIN_NAMESPACE

QMqttLastWillProperties::QMqttLastWillProperties() : data(new QMqttLastWillPropertiesData)
{
}

QMqttLastWillProperties::QMqttLastWillProperties(const QMqttLastWillProperties &rhs) : data(rhs.data)
{
}

QMqttLastWillProperties &QMqttLastWillProperties::operator=(const QMqttLastWillProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttLastWillProperties::~QMqttLastWillProperties()
{
}

quint32 QMqttLastWillProperties::willDelayInterval() const
{
    return data->willDelayInterval;
}

QMqtt::PayloadFormatIndicator QMqttLastWillProperties::payloadFormatIndicator() const
{
    return data->formatIndicator;
}

quint32 QMqttLastWillProperties::messageExpiryInterval() const
{
    return data->messageExpiryInterval;
}

QString QMqttLastWillProperties::contentType() const
{
    return data->contentType;
}

QString QMqttLastWillProperties::responseTopic() const
{
    return data->responseTopic;
}

QByteArray QMqttLastWillProperties::correlationData() const
{
    return data->correlationData;
}

QMqttUserProperties QMqttLastWillProperties::userProperties() const
{
    return data->userProperties;
}

void QMqttLastWillProperties::setWillDelayInterval(quint32 delay)
{
    data->willDelayInterval = delay;
}

void QMqttLastWillProperties::setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator p)
{
    data->formatIndicator = p;
}

void QMqttLastWillProperties::setMessageExpiryInterval(quint32 expiry)
{
    data->messageExpiryInterval = expiry;
}

void QMqttLastWillProperties::setContentType(const QString &content)
{
    data->contentType = content;
}

void QMqttLastWillProperties::setResponseTopic(const QString &response)
{
    data->responseTopic = response;
}

void QMqttLastWillProperties::setCorrelationData(const QByteArray &correlation)
{
    data->correlationData = correlation;
}

void QMqttLastWillProperties::setUserProperties(const QMqttUserProperties &properties)
{
    data->userProperties = properties;
}

QMqttConnectionProperties::QMqttConnectionProperties() : data(new QMqttConnectionPropertiesData)
{

}

QMqttConnectionProperties::QMqttConnectionProperties(const QMqttConnectionProperties &rhs) : data(rhs.data)
{

}

QMqttConnectionProperties &QMqttConnectionProperties::operator=(const QMqttConnectionProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttConnectionProperties::~QMqttConnectionProperties()
{

}

void QMqttConnectionProperties::setSessionExpiryInterval(quint32 expiry)
{
    data->sessionExpiryInterval = expiry;
}

void QMqttConnectionProperties::setMaximumReceive(quint16 qos)
{
    if (qos == 0) {
        qWarning("Maximum Receive is not allowed to be 0.");
        return;
    }
    data->maximumReceive = qos;
}

void QMqttConnectionProperties::setMaximumPacketSize(quint32 packetSize)
{
    if (packetSize == 0) {
        qWarning("Packet size is not allowed to be 0.");
        return;
    }
    data->maximumPacketSize = packetSize;
}

void QMqttConnectionProperties::setMaximumTopicAlias(quint16 alias)
{
    data->maximumTopicAlias = alias;
}

void QMqttConnectionProperties::setRequestResponseInformation(bool response)
{
    data->requestResponseInformation = response;
}

void QMqttConnectionProperties::setRequestProblemInformation(bool problem)
{
    data->requestProblemInformation = problem;
}

void QMqttConnectionProperties::setUserProperties(const QMqttUserProperties &properties)
{
    data->userProperties = properties;
}

void QMqttConnectionProperties::setAuthenticationMethod(const QString &authMethod)
{
    data->authenticationMethod = authMethod;
}

void QMqttConnectionProperties::setAuthenticationData(const QByteArray &authData)
{
    data->authenticationData = authData;
}

quint32 QMqttConnectionProperties::sessionExpiryInterval() const
{
    return data->sessionExpiryInterval;
}

quint16 QMqttConnectionProperties::maximumReceive() const
{
    return data->maximumReceive;
}

quint32 QMqttConnectionProperties::maximumPacketSize() const
{
    return data->maximumPacketSize;
}

quint16 QMqttConnectionProperties::maximumTopicAlias() const
{
    return data->maximumTopicAlias;
}

bool QMqttConnectionProperties::requestResponseInformation() const
{
    return data->requestResponseInformation;
}

bool QMqttConnectionProperties::requestProblemInformation() const
{
    return data->requestProblemInformation;
}

QMqttUserProperties QMqttConnectionProperties::userProperties() const
{
    return data->userProperties;
}

QString QMqttConnectionProperties::authenticationMethod() const
{
    return data->authenticationMethod;
}

QByteArray QMqttConnectionProperties::authenticationData() const
{
    return data->authenticationData;
}

QMqttServerConnectionProperties::QMqttServerConnectionProperties()
    : QMqttConnectionProperties()
    , serverData(new QMqttServerConnectionPropertiesData)
{

}

QMqttServerConnectionProperties::QMqttServerConnectionProperties(const QMqttServerConnectionProperties &rhs)
    : QMqttConnectionProperties(rhs)
    , serverData(rhs.serverData)
{

}

QMqttServerConnectionProperties &QMqttServerConnectionProperties::operator=(const QMqttServerConnectionProperties &rhs)
{
    if (this != &rhs) {
        serverData.operator=(rhs.serverData);
        QMqttConnectionProperties::operator=(rhs);
    }
    return *this;
}

QMqttServerConnectionProperties::~QMqttServerConnectionProperties()
{

}

QMqttServerConnectionProperties::ServerPropertyDetails QMqttServerConnectionProperties::availableProperties() const
{
    return serverData->details;
}

bool QMqttServerConnectionProperties::isValid() const
{
    return serverData->valid;
}

quint8 QMqttServerConnectionProperties::maximumQoS() const
{
    return serverData->maximumQoS;
}

bool QMqttServerConnectionProperties::retainAvailable() const
{
    return serverData->retainAvailable;
}

bool QMqttServerConnectionProperties::clientIdAssigned() const
{
    return serverData->details & QMqttServerConnectionProperties::AssignedClientId;
}

QString QMqttServerConnectionProperties::reasonString() const
{
    return serverData->reasonString;
}

bool QMqttServerConnectionProperties::wildcardSupported() const
{
    return serverData->wildcardSupported;
}

bool QMqttServerConnectionProperties::subscriptionIdentifierSupported() const
{
    return serverData->subscriptionIdentifierSupported;
}

bool QMqttServerConnectionProperties::sharedSubscriptionSupported() const
{
    return serverData->sharedSubscriptionSupported;
}

quint16 QMqttServerConnectionProperties::serverKeepAlive() const
{
    return serverData->serverKeepAlive;
}

QString QMqttServerConnectionProperties::responseInformation() const
{
    return serverData->responseInformation;
}

QString QMqttServerConnectionProperties::serverReference() const
{
    return serverData->serverReference;
}

QT_END_NAMESPACE
