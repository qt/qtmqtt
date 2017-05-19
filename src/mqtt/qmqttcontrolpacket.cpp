/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
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
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

#include "qmqttcontrolpacket_p.h"
#include <QtCore/QtEndian>

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
    m_payload.append(msb_c[0]);
    m_payload.append(msb_c[1]);
}

void QMqttControlPacket::append(const QByteArray &data)
{
    append(static_cast<quint16>(data.size()));
    m_payload.append(data.constData());
}

void QMqttControlPacket::appendRaw(const QByteArray &data)
{
    m_payload.append(data.constData());
}

QByteArray QMqttControlPacket::serialize() const
{
    // Create ByteArray
    QByteArray data;
    // Add Header
    data.append(char(m_header));
    // Add length
    quint32 msgSize = m_payload.size();
    if (msgSize > 268435455) // 0xFFFFFF7F
        qWarning("Publishing a message bigger than maximum size!");
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

