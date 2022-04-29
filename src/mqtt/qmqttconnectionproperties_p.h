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

#ifndef QMQTTCONNECTIONPROPERTIES_P_H
#define QMQTTCONNECTIONPROPERTIES_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qmqttconnectionproperties.h"

QT_BEGIN_NAMESPACE

class QMqttLastWillPropertiesData : public QSharedData
{
public:
    QString contentType;
    QString responseTopic;
    QByteArray correlationData;
    QMqttUserProperties userProperties;
    quint32 willDelayInterval{0};
    quint32 messageExpiryInterval{0};
    QMqtt::PayloadFormatIndicator formatIndicator{QMqtt::PayloadFormatIndicator::Unspecified};
};

class QMqttConnectionPropertiesData : public QSharedData
{
public:
    QMqttUserProperties userProperties;
    QString authenticationMethod;
    QByteArray authenticationData;
    quint32 sessionExpiryInterval{0};
    quint32 maximumPacketSize{std::numeric_limits<quint32>::max()};
    quint16 maximumReceive{65535};
    quint16 maximumTopicAlias{0};
    bool requestResponseInformation{false};
    bool requestProblemInformation{true};
};

class QMqttServerConnectionPropertiesData : public QSharedData
{
public:
    QMqttServerConnectionProperties::ServerPropertyDetails details{QMqttServerConnectionProperties::None};
    QString reasonString;
    QString responseInformation;
    QString serverReference;
    quint16 serverKeepAlive{0};
    quint8 maximumQoS{2};
    QMqtt::ReasonCode reasonCode{QMqtt::ReasonCode::Success};
    bool valid{false}; // Only set to true after CONNACK
    bool retainAvailable{true};
    bool wildcardSupported{true};
    bool subscriptionIdentifierSupported{true};
    bool sharedSubscriptionSupported{true};
};

QT_END_NAMESPACE

#endif // QMQTTCONNECTIONPROPERTIES_P_H
