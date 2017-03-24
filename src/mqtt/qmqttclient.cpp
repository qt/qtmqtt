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
    Q_D(QMqttClient);
    d->m_connection.setClient(this);
}

void QMqttClient::setTransport(QIODevice *device, QMqttClient::TransportType transport)
{
    Q_D(QMqttClient);

    if (d->m_state != Disconnected) {
        qWarning("Changing transport layer while connected is not possible");
        return;
    }
    d->m_connection.setTransport(device, transport);
}

QIODevice *QMqttClient::transport() const
{
    Q_D(const QMqttClient);
    return d->m_connection.transport();
}

QSharedPointer<QMqttSubscription> QMqttClient::subscribe(const QString &topic, quint8 qos)
{
    Q_D(QMqttClient);
    return d->m_connection.sendControlSubscribe(topic, qos);
}

void QMqttClient::unsubscribe(const QString &topic)
{
    Q_D(QMqttClient);
    d->m_connection.sendControlUnsubscribe(topic);
}

bool QMqttClient::publish(const QString &topic, const QByteArray &message, quint8 qos, bool retain)
{
    Q_D(QMqttClient);
    if (qos > 2)
        return false;
    return d->m_connection.sendControlPublish(topic, message, qos, retain);
}

bool QMqttClient::requestPing()
{
    Q_D(QMqttClient);
    return d->m_connection.sendControlPingRequest();
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

void QMqttClient::connectToHost()
{
    Q_D(QMqttClient);

    if (!d->m_connection.ensureTransport()) {
        qWarning("Could not ensure connection");
        setState(Disconnected);
        return;
    }
    setState(Connecting);

    if (!d->m_connection.ensureTransportOpen()) {
        qWarning("Could not ensure that connection is open");
        setState(Disconnected);
        return;
    }

    if (!d->m_connection.sendControlConnect()) {
        qWarning("Could not send CONNECT to broker");
        // ### Who disconnects now? Connection or client?
        setState(Disconnected);
        return;
    }
}

void QMqttClient::disconnectFromHost()
{
    Q_D(QMqttClient);

    if (d->m_connection.internalState() != QMqttConnection::BrokerConnected)
        return;

    d->m_connection.sendControlDisconnect();
}

QMqttClient::State QMqttClient::state() const
{
    Q_D(const QMqttClient);
    return d->m_state;
}

QString QMqttClient::username() const
{
    Q_D(const QMqttClient);
    return d->m_username;
}

QString QMqttClient::password() const
{
    Q_D(const QMqttClient);
    return d->m_password;
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

    if (state() != QMqttClient::Disconnected) {
        qWarning("Trying to modify keepAlive while connected.");
        return;
    }

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

void QMqttClient::setState(QMqttClient::State state)
{
    Q_D(QMqttClient);
    if (d->m_state == state)
        return;

    d->m_state = state;
    emit stateChanged(state);
    if (d->m_state == QMqttClient::Disconnected)
        emit disconnected();
    else if (d->m_state == QMqttClient::Connected)
        emit connected();
}

void QMqttClient::setUsername(QString username)
{
    Q_D(QMqttClient);
    if (d->m_username == username)
        return;

    d->m_username = username;
    emit usernameChanged(username);
}

void QMqttClient::setPassword(QString password)
{
    Q_D(QMqttClient);
    if (d->m_password == password)
        return;

    d->m_password = password;
    emit passwordChanged(password);
}

QMqttClientPrivate::QMqttClientPrivate()
    : QObjectPrivate()
{
    m_clientId = QUuid::createUuid().toString();
    m_clientId.remove(QLatin1Char('{'), Qt::CaseInsensitive);
    m_clientId.remove(QLatin1Char('}'), Qt::CaseInsensitive);
    m_clientId.remove(QLatin1Char('-'), Qt::CaseInsensitive);
}

QMqttClientPrivate::~QMqttClientPrivate()
{
}

QT_END_NAMESPACE
