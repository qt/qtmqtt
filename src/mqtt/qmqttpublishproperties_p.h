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
