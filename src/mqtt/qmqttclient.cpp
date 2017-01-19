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
#include "qmqttclient_p.h"

#include <QTcpSocket>
#include <QUuid>
#include <QtEndian>
#include <QDebug>

QT_BEGIN_NAMESPACE

QMqttClient::QMqttClient(QObject *parent) : QObject(*(new QMqttClientPrivate), parent)
{
}

void QMqttClient::setTransport(QIODevice *device, QMqttClient::TransportType transport)
{
    Q_D(QMqttClient);
    if (d->m_transport && d->m_ownTransport)
        delete d->m_transport;

    d->m_transport = device;
    d->m_transportType = transport;
    d->m_ownTransport = false;
}

QIODevice *QMqttClient::transport() const
{
    Q_D(const QMqttClient);
    return d->m_transport;
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
    Q_D(const QMqttClient);
    return d->m_hostname;
}

quint16 QMqttClient::port() const
{
    Q_D(const QMqttClient);
    return d->m_port;
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
    Q_D(QMqttClient);
    if (d->m_transport == nullptr) {
        // We are asked to create a transport layer
        if (d->m_hostname.isEmpty() || d->m_port == 0) {
            qWarning("No hostname specified");
            return;
        }
        auto socket = new QTcpSocket();
        d->m_transport = socket;
        d->m_ownTransport = true;
        d->m_transportType = TransportType::AbstractSocket;
    }

    // Check for open / connected
    if (!d->m_transport->isOpen()) {
        if (d->m_transportType == TransportType::IODevice) {
            if (d->m_transport->open(QIODevice::ReadWrite)) {
                qWarning("Could not open Transport IO device");
                return;
            }
        } else if (d->m_transportType == TransportType::AbstractSocket) {
            auto socket = dynamic_cast<QTcpSocket*>(d->m_transport);
            Q_ASSERT(socket);
            socket->connectToHost(d->m_hostname, d->m_port);
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
    if (d->m_protocolVersion == 3) {
        const QByteArray other("MQIsdp");
        addInt(connectFrame, other.size());
        connectFrame += other.data();
        connectFrame += char(3); // Version 3.1
    } else if (d->m_protocolVersion == 4) {
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
    addInt(connectFrame, d->m_keepAlive);

    // 3.1.3 Payload
    // 3.1.3.1 Client Identifier

    if (d->m_protocolVersion == 3) { // no username
        connectFrame += char(0);
        connectFrame += char(0);
    } else if (d->m_protocolVersion == 4) {
        // Client id maximum left is 23
        const QByteArray clientStringArray = d->m_clientId.left(23).toUtf8();
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

    d->m_transport->write(connectFrame.constData(), frameLength);
    if (!d->m_transport->waitForBytesWritten(30 * 1000)) {
        qWarning("Could not send data");
        qDebug() << "Error:" << d->m_transport->errorString();
        return;
    }

    if (!d->m_transport->waitForReadyRead(30 * 1000)) {
        qWarning("Did not get an ACK within time");
        qDebug() << "Error:" << d->m_transport->errorString();
        return;
    }

    QByteArray result = d->m_transport->readAll();
    qDebug() << "Result content:" << result;
}

void QMqttClient::disconnectFromHost()
{

}

quint8 QMqttClient::protocolVersion() const
{
    Q_D(const QMqttClient);
    return d->m_protocolVersion;
}

QString QMqttClient::clientId() const
{
    Q_D(const QMqttClient);
    return d->m_clientId;
}

quint16 QMqttClient::keepAlive() const
{
    Q_D(const QMqttClient);
    return d->m_keepAlive;
}

void QMqttClient::setHostname(QString hostname)
{
    Q_D(QMqttClient);
    if (d->m_hostname == hostname)
        return;

    d->m_hostname = hostname;
    emit hostnameChanged(hostname);
}

void QMqttClient::setPort(quint16 port)
{
    Q_D(QMqttClient);
    if (d->m_port == port)
        return;

    d->m_port = port;
    emit portChanged(port);
}

void QMqttClient::setClientId(QString clientId)
{
    Q_D(QMqttClient);
    if (d->m_clientId == clientId)
        return;

    d->m_clientId = clientId;
    emit clientIdChanged(clientId);
}

void QMqttClient::setKeepAlive(quint16 keepAlive)
{
    Q_D(QMqttClient);
    if (d->m_keepAlive == keepAlive)
        return;

    d->m_keepAlive = keepAlive;
    emit keepAliveChanged(keepAlive);
}

void QMqttClient::setProtocolVersion(quint8 protocolVersion)
{
    Q_D(QMqttClient);
    if (d->m_protocolVersion == protocolVersion)
        return;

    d->m_protocolVersion = protocolVersion;
    emit protocolVersionChanged(protocolVersion);
}

QMqttClientPrivate::QMqttClientPrivate()
    : QObjectPrivate()
{
    m_clientId = QUuid::createUuid().toString();
    m_clientId.remove(QChar('{'), Qt::CaseInsensitive);
    m_clientId.remove(QChar('}'), Qt::CaseInsensitive);
    m_clientId.remove(QChar('-'), Qt::CaseInsensitive);
}

QMqttClientPrivate::~QMqttClientPrivate()
{
    // ### TODO: check for open connection and quit gracefully
    if (m_ownTransport && m_transport)
        delete m_transport;
}

QT_END_NAMESPACE
