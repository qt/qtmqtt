/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Mqtt module of the Qt Toolkit.
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
****************************************************************************/
#include "qmqttclient.h"

#include <QTcpSocket>
#include <QUuid>
#include <QtEndian>
#include <QDebug>

QT_BEGIN_NAMESPACE

QMqttClient::QMqttClient(QObject *parent) : QObject(parent)
{
    if (m_clientId.isEmpty()) {
        m_clientId = QUuid::createUuid().toString();
        m_clientId.remove(QChar('{'), Qt::CaseInsensitive);
        m_clientId.remove(QChar('}'), Qt::CaseInsensitive);
        m_clientId.remove(QChar('-'), Qt::CaseInsensitive);
    }
}

bool QMqttClient::subscribe(const QString &topic)
{
    Q_UNIMPLEMENTED();
    Q_UNUSED(topic);
    return false;
}

void QMqttClient::unsubscribe(const QString &topic)
{
    Q_UNIMPLEMENTED();
    Q_UNUSED(topic);
}

QString QMqttClient::hostname() const
{
    return m_hostname;
}

quint16 QMqttClient::port() const
{
    return m_port;
}

inline void addInt(QByteArray &data, quint16 n)
{
    const quint16 msb = qToBigEndian<quint16>(n);
    const char * msb_c = reinterpret_cast<const char*>(&msb);
    data.append(msb_c[0]);
    data.append(msb_c[1]);
    return;
}

void QMqttClient::connectToHost()
{
    if (m_transport == nullptr) {
        // We are asked to create a transport layer
        if (m_hostname.isEmpty() || m_port == 0) {
            qWarning("No hostname specified");
            return;
        }
        auto socket = new QTcpSocket();
        m_transport = socket;
        m_ownTransport = true;
        m_transportType = TransportType::AbstractSocket;
    }

    // Check for open / connected
    if (!m_transport->isOpen()) {
        if (m_transportType == TransportType::IODevice) {
            if (m_transport->open(QIODevice::ReadWrite)) {
                qWarning("Could not open Transport IO device");
                return;
            }
        } else if (m_transportType == TransportType::AbstractSocket) {
            auto socket = dynamic_cast<QTcpSocket*>(m_transport);
            Q_ASSERT(socket);
            socket->connectToHost(m_hostname, m_port);
            if (!socket->waitForConnected()) {
                qWarning("Could not establish socket connection for transport");
                return;
            }
        }
    }

    QByteArray connectFrame;
    // Fixed header
    connectFrame += 0x10;

    connectFrame += char(0); // Length to be filled later
    // Variable header
    // 3.1.2.1 Protocol Name
    // 3.1.2.2 Protocol Level
    if (m_protocolVersion == 3) {
        const QByteArray other("MQIsdp");
        addInt(connectFrame, other.size());
        connectFrame += other.data();
        connectFrame += char(3); // Version 3.1
    } else if (m_protocolVersion == 4) {
        connectFrame += char(0); // Length MSB
        connectFrame += 4; // Length LSB
        connectFrame += "MQTT";
        connectFrame += char(4); // Version 3.1.1
    } else {
        qFatal("Illegal MQTT VERSION");
    }

    // 3.1.2.3 Connect Flags
    quint8 flags = 0;
    // Clean session
    flags |= 1;
    connectFrame += char(flags);

    // 3.1.2.10 Keep Alive
    addInt(connectFrame, m_keepAlive);

    // 3.1.3 Payload
    // 3.1.3.1 Client Identifier

    if (m_protocolVersion == 3) { // no username
        connectFrame += char(0);
        connectFrame += char(0);
    } else if (m_protocolVersion == 4) {
        // Client id maximum left is 23
        const QByteArray clientStringArray = m_clientId.left(23).toUtf8();
        if (clientStringArray.size()) {
            addInt(connectFrame, clientStringArray.size());
            connectFrame += clientStringArray;
        } else {
            connectFrame += char(0);
            connectFrame += char(0);
        }
    }

    const quint8 frameLength = connectFrame.size();
    connectFrame[1] = char(frameLength - 2); // The header bytes are not included

    m_transport->write(connectFrame.constData(), frameLength);
    if (!m_transport->waitForBytesWritten(30 * 1000)) {
        qWarning("Could not send data");
        qDebug() << "Error:" << m_transport->errorString();
        return;
    }

    if (!m_transport->waitForReadyRead(30 * 1000)) {
        qWarning("Did not get an ACK within time");
        qDebug() << "Error:" << m_transport->errorString();
        return;
    }

    QByteArray result = m_transport->readAll();
    qDebug() << "Result content:" << result;
}

void QMqttClient::disconnectFromHost()
{

}

quint8 QMqttClient::protocolVersion() const
{
    return m_protocolVersion;
}

QString QMqttClient::clientId() const
{
    return m_clientId;
}

quint16 QMqttClient::keepAlive() const
{
    return m_keepAlive;
}

void QMqttClient::setHostname(QString hostname)
{
    if (m_hostname == hostname)
        return;

    m_hostname = hostname;
    emit hostnameChanged(hostname);
}

void QMqttClient::setPort(quint16 port)
{
    if (m_port == port)
        return;

    m_port = port;
    emit portChanged(port);
}

void QMqttClient::setClientId(QString clientId)
{
    if (m_clientId == clientId)
        return;

    m_clientId = clientId;
    emit clientIdChanged(clientId);
}

void QMqttClient::setKeepAlive(quint16 keepAlive)
{
    if (m_keepAlive == keepAlive)
        return;

    m_keepAlive = keepAlive;
    emit keepAliveChanged(keepAlive);
}

void QMqttClient::setProtocolVersion(quint8 protocolVersion)
{
    if (m_protocolVersion == protocolVersion)
        return;

    m_protocolVersion = protocolVersion;
    emit protocolVersionChanged(protocolVersion);
}

QT_END_NAMESPACE
