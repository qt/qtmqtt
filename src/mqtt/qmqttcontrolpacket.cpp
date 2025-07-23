// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:network-protocol

#include "qmqttcontrolpacket_p.h"

#include <QtCore/QtEndian>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

QMqttControlPacket::QMqttControlPacket()
{

}

QMqttControlPacket::QMqttControlPacket(quint8 header)
    : m_header(header)
{

}

QMqttControlPacket::QMqttControlPacket(quint8 header, const QByteArray &pay)
    : m_header(header)
    , m_payload(pay)
{

}

void QMqttControlPacket::clear()
{
    m_header = 0;
    m_payload.clear();
}

void QMqttControlPacket::setHeader(quint8 h)
{
    if (h < QMqttControlPacket::CONNECT || h > DISCONNECT || h & 0x0F)
        m_header = QMqttControlPacket::UNKNOWN;
    else
        m_header = h;
}

void QMqttControlPacket::append(char value)
{
    m_payload.append(value);
}

void QMqttControlPacket::append(quint16 value)
{
    const quint16 msb = qToBigEndian<quint16>(value);
    const char * msb_c = reinterpret_cast<const char*>(&msb);
    m_payload.append(msb_c, 2);
}

void QMqttControlPacket::append(quint32 value)
{
    const quint32 msb = qToBigEndian<quint32>(value);
    const char * msb_c = reinterpret_cast<const char*>(&msb);
    m_payload.append(msb_c, 4);
}

void QMqttControlPacket::append(const QByteArray &data)
{
    append(static_cast<quint16>(data.size()));
    m_payload.append(data);
}

void QMqttControlPacket::appendRaw(const QByteArray &data)
{
    m_payload.append(data);
}

void QMqttControlPacket::appendRawVariableInteger(quint32 value)
{
    QByteArray data;
    // Add length
    if (value > 268435455)
        qCDebug(lcMqttClient) << "Attempting to write variable integer overflow.";
    do {
        quint8 b = value % 128;
        value /= 128;
        if (value > 0)
            b |= 0x80;
        data.append(char(b));
    } while (value > 0);
    appendRaw(data);
}

QByteArray QMqttControlPacket::serialize() const
{
    // Create ByteArray
    QByteArray data;
    // Add Header
    data.append(char(m_header));
    data.append(serializePayload());

    return data;
}

QByteArray QMqttControlPacket::serializePayload() const
{
    QByteArray data;
    // Add length
    quint32 msgSize = quint32(m_payload.size());
    if (msgSize > 268435455)
        qCDebug(lcMqttClient) << "Publishing a message bigger than maximum size.";
    do {
        quint8 b = msgSize % 128;
        msgSize /= 128;
        if (msgSize > 0)
            b |= 0x80;
        data.append(char(b));
    } while (msgSize > 0);
    // Add payload
    data.append(m_payload);
    return data;
}

QT_END_NAMESPACE

