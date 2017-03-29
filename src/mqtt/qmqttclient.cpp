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

QT_BEGIN_NAMESPACE

/*!
    \class QMqttClient

    \inmodule QtMqtt
    \brief The QMqttClient class represents the central access communicating with
    a MQTT broker.
*/

/*!
    \property QMqttClient::clientId
    \brief The clients identifier value.

    Each client needs to have a unique ID to be able to connect to a MQTT broker. If no client ID
    is specified by the user, one will be generated automatically when a connection is established.
*/

/*!
    \property QMqttClient::hostname
    \brief The hostname of the MQTT broker to connect to.

    If no transport is specified via \l QMqttClient::setTransport(), then the client will
    instantiate a socket connection to the specified hostname itself.
*/

/*!
    \property QMqttClient::port
    \brief The port to connect to the MQTT broker.

    If no transport is specified via \l QMqttClient::setTransport, then the client will
    instantiate a socket connection to a host with this port number.
*/

/*!
    \property QMqttClient::keepAlive
    \brief Period to send regular ping messages to the broker.

    Once a connection to a broker is established, the client needs to send frequent updates to
    propagate it can still be reached. The interval between those updates is specified by this
    property.
*/

/*!
    \property QMqttClient::protocolVersion
    \brief The MQTT standard version to use for connections.

    Specifies the version of the standard the client uses for connecting to a broker. Values values
    are

    \list
    \li 3: MQTT standard version 3.1.
    \li 4: MQTT standard version 3.1.1, often referred to MQTT 4.
    \endlist
*/

/*!
    \property QMqttClient::state
    \brief Specifies the current state of the client.
*/

/*!
    \property QMqttClient::username
    \brief Specifies the user name for connecting to a broker.
*/

/*!
    \property QMqttClient::password
    \brief Specifies the password for connecting to a broker.
*/

/*!
    \enum QMqttClient::TransportType

    This enum type specifies the connection method to be used to instantiate a connection with
    a broker.

    \value IODevice
           The transport uses a class based on a QIODevice.
    \value AbstractSocket
           The transport uses a class based on a QAbstractSocket.
*/

/*!
    \enum QMqttClient::State

    This enum type specifies the states a client can enter
    \value Disconnected
           The client is disconnected from the broker.
    \value Connecting
           A connection request has been made, but the broker has not approved the connection yet.
    \value Connected
           The client is connected to the broker.
*/

/*!
    \fn QMqttClient::connected()

    This signal is emitted when a connection has been established.
*/

/*!
    \fn QMqttClient::disconnected()

    This signal is emitted when a connection has been closed. A disconnect can happen because
    of a call of connectToHost() or the broker disconnected.
*/

/*!
    \fn QMqttClient::messageReceived(const QString &topic, const QByteArray &message)

    This signal is emitted when a new message has been received. The category of the message is
    specified in \a topic with the content being \a message.
*/

/*!
    \fn QMqttClient::pingResponse()

    If requestPing() is called or every keepAlive() milliseconds, this signal is emitted after
    the broker reponded and the connection is still valid.
*/

/*!
    Creates a new QMqttClient instance with the specified \a parent
 */
QMqttClient::QMqttClient(QObject *parent) : QObject(*(new QMqttClientPrivate), parent)
{
    Q_D(QMqttClient);
    d->m_connection.setClient(this);
}

/*!
    Sets the transport to \a device. A transport can be either a socket type
    or derived from QIODevice and is specified by \a transport.

    Note that the transport can only be exchanged if the \l QMqttClient is in \l Disconnected
    state.
 */
void QMqttClient::setTransport(QIODevice *device, QMqttClient::TransportType transport)
{
    Q_D(QMqttClient);

    if (d->m_state != Disconnected) {
        qWarning("Changing transport layer while connected is not possible");
        return;
    }
    d->m_connection.setTransport(device, transport);
}

/*!
    Returns the transport used for communication with the broker.
 */
QIODevice *QMqttClient::transport() const
{
    Q_D(const QMqttClient);
    return d->m_connection.transport();
}

/*!
    Adds a new subscription to receive notifications on \a topic. \a qos specifies the level
    of security messages are received. For more information on various QoS levels, please refer
    to \l {Quality of Service}.

    This functions returns a \l QSharedPointer to a \l QMqttSubscription. If a subscription to
    the same topic is made twice, the return value is pointing to the same subscription instance.
 */
QSharedPointer<QMqttSubscription> QMqttClient::subscribe(const QString &topic, quint8 qos)
{
    Q_D(QMqttClient);

    if (d->m_state != QMqttClient::Connected)
        return QSharedPointer<QMqttSubscription>();

    return d->m_connection.sendControlSubscribe(topic, qos);
}

/*!
    Unsubscribes from \a topic. No notifications will be sent to any of the subscriptions made
    by \l QMqttClient::subscribe().
 */
void QMqttClient::unsubscribe(const QString &topic)
{
    Q_D(QMqttClient);
    d->m_connection.sendControlUnsubscribe(topic);
}

/*!
    Publishes a \a message to the broker with the specified \a topic. \a qos specifies the level
    of required security that the message is transfered.

    If \a retain is set to true, the message will...

    Returns \c true if the publish request could be send.
 */
qint32 QMqttClient::publish(const QString &topic, const QByteArray &message, quint8 qos, bool retain)
{
    Q_D(QMqttClient);
    if (qos > 2)
        return -1;

    if (d->m_state != QMqttClient::Connected)
        return -1;

    return d->m_connection.sendControlPublish(topic, message, qos, retain);
}

/*!
    Sends a ping message to the broker and expects a pong to be send back by the broker. If the
    connection is active, \l QMqttClient will automatically send a ping message every
    \l QMqttClient::keepAlive miliseconds.

    To check whether the ping is successful, connect to the \l pingResponse signal.

    Returns \c true if the ping request could be send.
 */
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

/*!
    Initiates a connection to the MQTT broker.
 */
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

/*!
    Disconnect from the MQTT broker.
 */
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
