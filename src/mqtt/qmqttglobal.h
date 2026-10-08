// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTQMQTTGLOBAL_H
#define QTQMQTTGLOBAL_H

#include <QtCore/qglobal.h>
#include <QtMqtt/qtmqttexports.h>

QT_BEGIN_NAMESPACE

namespace QMqtt
{
enum class PayloadFormatIndicator : quint8 {
    Unspecified = 0,
    UTF8Encoded = 1
};

enum class MessageStatus : quint8 {
    Unknown = 0,
    Published,
    Acknowledged,
    Received,
    Released,
    Completed
};

enum class ReasonCode : quint8 {
    Success = 0,
    SubscriptionQoSLevel0 = 0,
    SubscriptionQoSLevel1 = 0x01,
    SubscriptionQoSLevel2 = 0x02,
    NoMatchingSubscriber = 0x10,
    NoSubscriptionExisted = 0x11,
    ContinueAuthentication = 0x18,
    ReAuthenticate = 0x19,
    UnspecifiedError = 0x80,
    MalformedPacket = 0x81,
    ProtocolError = 0x82,
    ImplementationSpecificError = 0x83,
    UnsupportedProtocolVersion = 0x84,
    InvalidClientId = 0x85,
    InvalidUserNameOrPassword = 0x86,
    NotAuthorized = 0x87,
    ServerNotAvailable = 0x88,
    ServerBusy = 0x89,
    ClientBanned = 0x8A,
    InvalidAuthenticationMethod = 0x8C,
    InvalidTopicFilter = 0x8F,
    InvalidTopicName = 0x90,
    MessageIdInUse = 0x91,
    MessageIdNotFound = 0x92,
    PacketTooLarge = 0x95,
    QuotaExceeded = 0x97,
    InvalidPayloadFormat = 0x99,
    RetainNotSupported = 0x9A,
    QoSNotSupported = 0x9B,
    UseAnotherServer = 0x9C,
    ServerMoved = 0x9D,
    SharedSubscriptionsNotSupported = 0x9E,
    ExceededConnectionRate = 0x9F,
    SubscriptionIdsNotSupported = 0xA1,
    WildCardSubscriptionsNotSupported = 0xA2
};
}
QT_END_NAMESPACE

#endif //QTQMQTTGLOBAL_H
