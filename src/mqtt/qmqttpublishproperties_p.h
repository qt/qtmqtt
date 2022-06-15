// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QMQTTPUBLISHPROPERTIES_P_H
#define QMQTTPUBLISHPROPERTIES_P_H

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

#include "qmqttglobal.h"
#include "qmqttpublishproperties.h"

#include <QtCore/QSharedData>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QMqttPublishPropertiesData : public QSharedData
{
public:
    QString responseTopic;
    QString contentType;
    QByteArray correlationData;
    quint32 messageExpiry{0};
    QList<quint32> subscriptionIdentifier;
    QMqttPublishProperties::PublishPropertyDetails details{QMqttPublishProperties::None};
    quint16 topicAlias{0};
    QMqtt::PayloadFormatIndicator payloadIndicator{QMqtt::PayloadFormatIndicator::Unspecified};
    QMqttUserProperties userProperties;
};

class QMqttMessageStatusPropertiesData : public QSharedData
{
public:
    QMqttUserProperties userProperties;
    QString reasonString;
    QMqtt::ReasonCode reasonCode{QMqtt::ReasonCode::Success};
};

QT_END_NAMESPACE

#endif // QMQTTPUBLISHPROPERTIES_P_H
