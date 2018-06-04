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

#include "qmqttpublishproperties.h"
#include "qmqtttype.h"

QT_BEGIN_NAMESPACE

class QMqttPublishPropertiesData : public QSharedData
{
public:
    QString responseTopic;
    QString contentType;
    QByteArray correlationData;
    quint32 messageExpiry{0};
    quint32 subscriptionIdentifier{0}; // Variable Integer
    quint16 topicAlias{0};
    QMqttPublishProperties::PayloadIndicatorOption payloadIndicator{QMqttPublishProperties::Unspecified};
    QMqttPublishProperties::PublishPropertyDetails details{0};
    QMqttUserProperties userProperties;
};

QMqttPublishProperties::QMqttPublishProperties() : data(new QMqttPublishPropertiesData)
{

}

QMqttPublishProperties::QMqttPublishProperties(const QMqttPublishProperties &rhs) : data(rhs.data)
{

}

QMqttPublishProperties &QMqttPublishProperties::operator=(const QMqttPublishProperties &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

QMqttPublishProperties::~QMqttPublishProperties()
{

}

QMqttPublishProperties::PublishPropertyDetails QMqttPublishProperties::availableProperties() const
{
    return data->details;
}

quint8 QMqttPublishProperties::payloadIndicator() const
{
    return data->payloadIndicator;
}

void QMqttPublishProperties::setPayloadIndicator(PayloadIndicatorOption indicator)
{
    if (indicator > 1) {
        qWarning("Invalid payload indicator specified");
        return;
    }
    data->details |= QMqttPublishProperties::PayloadFormatIndicator;
    data->payloadIndicator = indicator;
}

quint32 QMqttPublishProperties::messageExpiryInterval() const
{
    return data->messageExpiry;
}

void QMqttPublishProperties::setMessageExpiryInterval(quint32 interval)
{
    data->details |= QMqttPublishProperties::MessageExpiryInterval;
    data->messageExpiry = interval;
}

quint16 QMqttPublishProperties::topicAlias() const
{
    return data->topicAlias;
}

void QMqttPublishProperties::setTopicAlias(quint16 alias)
{
    if (alias == 0) {
        qWarning("A topic alias with value 0 is not allowed.");
        return;
    }
    data->details |= QMqttPublishProperties::TopicAlias;
    data->topicAlias = alias;
}

QString QMqttPublishProperties::responseTopic() const
{
    return data->responseTopic;
}

void QMqttPublishProperties::setResponseTopic(const QString &topic)
{
    data->details |= QMqttPublishProperties::ResponseTopic;
    data->responseTopic = topic;
}

QByteArray QMqttPublishProperties::correlationData() const
{
    return data->correlationData;
}

void QMqttPublishProperties::setCorrelationData(const QByteArray &correlation)
{
    data->details |= QMqttPublishProperties::CorrelationData;
    data->correlationData = correlation;
}

QMqttUserProperties QMqttPublishProperties::userProperties() const
{
    return data->userProperties;
}

void QMqttPublishProperties::setUserProperties(const QMqttUserProperties &properties)
{
    data->details |= QMqttPublishProperties::UserProperty;
    data->userProperties = properties;
}

quint32 QMqttPublishProperties::subscriptionIdentifier() const
{
    return data->subscriptionIdentifier;
}

void QMqttPublishProperties::setSubscriptionIdentifier(quint32 id)
{
    if (id == 0) {
        qWarning("A subscription identifier with value 0 is not allowed.");
        return;
    }
    data->details |= QMqttPublishProperties::SubscriptionIdentifier;
    data->subscriptionIdentifier = id;
}

QString QMqttPublishProperties::contentType() const
{
    return data->contentType;
}

void QMqttPublishProperties::setContentType(const QString &type)
{
    data->details |= QMqttPublishProperties::ContentType;
    data->contentType = type;
}

QT_END_NAMESPACE
