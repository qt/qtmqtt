/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "qmqttclient.h"
#include "qmqttclient_p.h"

#include <QtCore/QUuid>
#include <QtCore/QtEndian>

QT_BEGIN_NAMESPACE

/*!
    \class QMqttClient

    \inmodule QtMqtt
    \brief The QMqttClient class represents the central access communicating
    with an MQTT broker.

    An MQTT client is a program or device that uses MQTT to create a network
    connection to an MQTT server, also called a \e broker. The connection
    request must contain a unique client identifier. Optionally, it can contain
    a Will Topic, Will Message, user name, and password.

    Once a connection is created, a client can send messages that other clients
    might be interested in receiving, subscribe to request notifications on
    topics, unsubscribe to remove a request for notifications, and disconnect
    from the broker.
*/

/*!
    \property QMqttClient::clientId
    \brief This property holds the client's identifier value.

    Each client needs to have a unique ID to be able to connect to an MQTT
    broker. If no client ID is specified by the user, one will be generated
    automatically when a connection is established.
*/

/*!
    \property QMqttClient::hostname
    \brief This property holds the hostname of the MQTT broker to connect to.

    If no transport is specified via setTransport(), the client will instantiate
    a socket connection to the specified hostname itself.
*/

/*!
    \property QMqttClient::port
    \brief This property holds the port to connect to the MQTT broker.

    If no transport is specified via setTransport(), the client will instantiate
    a socket connection to a host with this port number.
*/

/*!
    \property QMqttClient::keepAlive
    \brief This property holds the interval at which regular ping messages are
    sent to the broker.

    Once a connection to a broker is established, the client needs to send
    frequent updates to propagate it can still be reached. The interval between
    those updates is specified by this property.

    The interval is specified in milliseconds. However, most brokers are not
    capable of using such a high granularity and will fall back to an interval
    specified in seconds.
*/

/*!
    \property QMqttClient::protocolVersion
    \brief This property holds the MQTT standard version to use for connections.

    Specifies the version of the standard the client uses for connecting to a
    broker. Valid values are:

    \list
        \li 3: MQTT standard version 3.1.
        \li 4: MQTT standard version 3.1.1, often referred to MQTT 4.
    \endlist
*/

/*!
    \property QMqttClient::state
    \brief This property holds the current state of the client.
*/

/*!
    \property QMqttClient::error
    \brief Specifies the current error of the client.
*/

/*!
    \property QMqttClient::username
    \brief This property holds the user name for connecting to a broker.
*/

/*!
    \property QMqttClient::password
    \brief This property holds the password for connecting to a broker.
*/

/*!
    \property QMqttClient::cleanSession
    \brief This property holds the state after connecting to a broker.
*/

/*!
    \property QMqttClient::willTopic
    \brief This property holds the Will Topic.
*/

/*!
    \property QMqttClient::willMessage
    \brief This property holds the payload of a Will Message.
*/

/*!
    \property QMqttClient::willQoS
    \brief This property holds the level of QoS for sending and storing the
    Will Message.
*/

/*!
    \property QMqttClient::willRetain
    \brief This property holds whether the Will Message should be retained on
    the broker for future subscribers to receive.
*/

/*!
    \enum QMqttClient::TransportType

    This enum type specifies the connection method to be used to instantiate a
    connection to a broker.

    \value IODevice
           The transport uses a class based on a QIODevice.
    \value AbstractSocket
           The transport uses a class based on a QAbstractSocket.
    \value SecureSocket
           The transport uses a class based on a QSslSocket.
*/

/*!
    \enum QMqttClient::ClientState

    This enum type specifies the states a client can enter.

    \value Disconnected
           The client is disconnected from the broker.
    \value Connecting
           A connection request has been made, but the broker has not approved
           the connection yet.
    \value Connected
           The client is connected to the broker.
*/

/*!
    \enum QMqttClient::ClientError

    This enum type specifies the error state of a client.

    \value NoError
           No error occurred.
    \value InvalidProtocolVersion
           The broker does not accept a connection using the specified protocol
           version.
    \value IdRejected
           The client ID is malformed. This might be related to its length.
    \value ServerUnavailable
           The network connection has been established, but the service is
           unavailable on the broker side.
    \value BadUsernameOrPassword
           The data in the username or password is malformed.
    \value NotAuthorized
           The client is not authorized to connect.
    \value TransportInvalid
           The underlying transport caused an error. For example, the connection
           might have been interrupted unexpectedly.
    \value ProtocolViolation
           The client encountered a protocol violation, and therefore closed the
           connection.
    \value UnknownError
           An unknown error occurred.
*/

/*!
    \enum QMqttClient::ProtocolVersion

    This enum specifies the protocol version of the MQTT standard to use during
    communication with a broker.

    \value MQTT_3_1
           MQTT Standard 3.1
    \value MQTT_3_1_1
           MQTT Standard 3.1.1, publicly referred to as version 4
*/

/*!
    \fn QMqttClient::connected()

    This signal is emitted when a connection has been established.
*/

/*!
    \fn QMqttClient::disconnected()

    This signal is emitted when a connection has been closed. A connection may
    be closed when disconnectFromHost() is called or when the broker
    disconnects.
*/

/*!
    \fn QMqttClient::messageReceived(const QByteArray &message, const QMqttTopicName &topic)

    This signal is emitted when a new message has been received. The category of
    the message is specified by \a topic with the content being \a message.
*/

/*!
    \fn QMqttClient::messageSent(qint32 id)

    Indicates that a message that was sent via the publish() function has been
    received by the broker. The \a id is the same as returned by \c publish() to
    help tracking the status of the message.
*/

/*!
    \fn QMqttClient::pingResponseReceived()

    This signal is emitted after the broker responds to a requestPing() call or
    a keepAlive() ping message, and the connection is still valid.
*/

/*!
    \fn QMqttClient::brokerSessionRestored()

    This signal is emitted after a client has successfully connected to a broker
    with the cleanSession property set to \c false, and the broker has restored
    the session.

    Sessions can be restored if a client has connected previously using the same
    clientId.
*/

/*!
    Creates a new MQTT client instance with the specified \a parent.
 */
QMqttClient::QMqttClient(QObject *parent) : QObject(*(new QMqttClientPrivate(this)), parent)
{
    Q_D(QMqttClient);
    d->m_connection.setClientPrivate(d);
}

/*!
    Sets the transport to \a device. A transport can be either a socket type
    or derived from QIODevice and is specified by \a transport.

    \note The transport can only be exchanged if the MQTT client is in the
    \l Disconnected state.

    \note Setting a custom transport for a client does not pass over responsibility
    on connection management. The transport has to be opened for QIODevice based
    transports or connected for socket type transports before calling QMqttClient::connectToHost().
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
    Adds a new subscription to receive notifications on \a topic. The parameter
    \a qos specifies the level at which security messages are received. For more
    information about the available QoS levels, see \l {Quality of Service}.

    This function returns a pointer to a \l QMqttSubscription. If the same topic
    is subscribed twice, the return value points to the same subscription
    instance. The MQTT client is the owner of the subscription.
 */
QMqttSubscription *QMqttClient::subscribe(const QMqttTopicFilter &topic, quint8 qos)
{
    Q_D(QMqttClient);

    if (d->m_state != QMqttClient::Connected)
        return nullptr;

    return d->m_connection.sendControlSubscribe(topic, qos);
}

/*!
    Unsubscribes from \a topic. No notifications will be sent to any of the
    subscriptions made by calling subscribe().

    \note If a client disconnects from a broker without unsubscribing, the
    broker will store all messages and publish them on the next reconnect.
 */
void QMqttClient::unsubscribe(const QMqttTopicFilter &topic)
{
    Q_D(QMqttClient);
    d->m_connection.sendControlUnsubscribe(topic);
}

/*!
    Publishes a \a message to the broker with the specified \a topic. \a qos
    specifies the level of security required for transferring the message.

    If \a retain is set to \c true, the message will stay on the broker for
    other clients to connect and receive the message.

    Returns an ID that is used internally to identify the message.
 */
qint32 QMqttClient::publish(const QMqttTopicName &topic, const QByteArray &message, quint8 qos, bool retain)
{
    Q_D(QMqttClient);
    if (qos > 2)
        return -1;

    if (d->m_state != QMqttClient::Connected)
        return -1;

    return d->m_connection.sendControlPublish(topic, message, qos, retain);
}

/*!
    Sends a ping message to the broker and expects a reply. If the connection
    is active, the MQTT client will automatically send a ping message at
    \l keepAlive intervals.

    To check whether the ping is successful, connect to the
    \l pingResponseReceived() signal.

    Returns \c true if the ping request could be sent.
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
    connectToHost(false, QString());
}

/*!
    Initiates an encrypted connection to the MQTT broker.

    \a sslPeerName specifies the peer name to be passed to the socket.
 */
#ifndef QT_NO_SSL
void QMqttClient::connectToHostEncrypted(const QString &sslPeerName)
{
    connectToHost(true, sslPeerName);
}
#endif

void QMqttClient::connectToHost(bool encrypted, const QString &sslPeerName)
{
    Q_D(QMqttClient);

    if (state() == QMqttClient::Connected) {
        qWarning("Already connected to a broker. Rejecting connection request.");
        return;
    }

    if (!d->m_connection.ensureTransport(encrypted)) {
        qWarning("Could not ensure connection");
        d->setStateAndError(Disconnected, TransportInvalid);
        return;
    }
    d->m_error = QMqttClient::NoError; // Fresh reconnect, unset error
    d->setStateAndError(Connecting);

    if (d->m_cleanSession)
        d->m_connection.cleanSubscriptions();

    if (!d->m_connection.ensureTransportOpen(sslPeerName)) {
        qWarning("Could not ensure that connection is open");
        d->setStateAndError(Disconnected, TransportInvalid);
        return;
    }

    // Once transport has connected, it will invoke
    // QMqttConnection::sendControlConnect to
    // handshake with the broker
}

/*!
    Disconnects from the MQTT broker.
 */
void QMqttClient::disconnectFromHost()
{
    Q_D(QMqttClient);

    switch (d->m_connection.internalState()) {
    case QMqttConnection::BrokerConnected:
        d->m_connection.sendControlDisconnect();
    case QMqttConnection::BrokerDisconnected:
        return;
    case QMqttConnection::BrokerConnecting:
    case QMqttConnection::BrokerWaitForConnectAck:
    default:
        d->m_connection.m_transport->close();
        break;
    }
}

QMqttClient::ClientState QMqttClient::state() const
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

bool QMqttClient::cleanSession() const
{
    Q_D(const QMqttClient);
    return d->m_cleanSession;
}

QString QMqttClient::willTopic() const
{
    Q_D(const QMqttClient);
    return d->m_willTopic;
}

quint8 QMqttClient::willQoS() const
{
    Q_D(const QMqttClient);
    return d->m_willQoS;
}

QByteArray QMqttClient::willMessage() const
{
    Q_D(const QMqttClient);
    return d->m_willMessage;
}

bool QMqttClient::willRetain() const
{
    Q_D(const QMqttClient);
    return d->m_willRetain;
}

QMqttClient::ClientError QMqttClient::error() const
{
    Q_D(const QMqttClient);
    return d->m_error;
}

QMqttClient::ProtocolVersion QMqttClient::protocolVersion() const
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

void QMqttClient::setHostname(const QString &hostname)
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

void QMqttClient::setClientId(const QString &clientId)
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

void QMqttClient::setProtocolVersion(ProtocolVersion protocolVersion)
{
    Q_D(QMqttClient);
    if (d->m_protocolVersion == protocolVersion)
        return;

    // Only MQTT 3 and 4 are supported
    if (protocolVersion < 3 || protocolVersion > 4)
        return;

    d->m_protocolVersion = protocolVersion;
    emit protocolVersionChanged(protocolVersion);
}

void QMqttClient::setState(ClientState state)
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

void QMqttClient::setUsername(const QString &username)
{
    Q_D(QMqttClient);
    if (d->m_username == username)
        return;

    d->m_username = username;
    emit usernameChanged(username);
}

void QMqttClient::setPassword(const QString &password)
{
    Q_D(QMqttClient);
    if (d->m_password == password)
        return;

    d->m_password = password;
    emit passwordChanged(password);
}

void QMqttClient::setCleanSession(bool cleanSession)
{
    Q_D(QMqttClient);
    if (d->m_cleanSession == cleanSession)
        return;

    d->m_cleanSession = cleanSession;
    emit cleanSessionChanged(cleanSession);
}

void QMqttClient::setWillTopic(const QString &willTopic)
{
    Q_D(QMqttClient);
    if (d->m_willTopic == willTopic)
        return;

    d->m_willTopic = willTopic;
    emit willTopicChanged(willTopic);
}

void QMqttClient::setWillQoS(quint8 willQoS)
{
    Q_D(QMqttClient);
    if (d->m_willQoS == willQoS)
        return;

    d->m_willQoS = willQoS;
    emit willQoSChanged(willQoS);
}

void QMqttClient::setWillMessage(const QByteArray &willMessage)
{
    Q_D(QMqttClient);
    if (d->m_willMessage == willMessage)
        return;

    d->m_willMessage = willMessage;
    emit willMessageChanged(willMessage);
}

void QMqttClient::setWillRetain(bool willRetain)
{
    Q_D(QMqttClient);
    if (d->m_willRetain == willRetain)
        return;

    d->m_willRetain = willRetain;
    emit willRetainChanged(willRetain);
}

void QMqttClient::setError(ClientError e)
{
    Q_D(QMqttClient);
    if (d->m_error == e)
        return;

    d->m_error = e;
    emit errorChanged(d->m_error);
}

QMqttClientPrivate::QMqttClientPrivate(QMqttClient *c)
    : QObjectPrivate()
{
    m_client = c;
    m_clientId = QUuid::createUuid().toString();
    m_clientId.remove(QLatin1Char('{'));
    m_clientId.remove(QLatin1Char('}'));
    m_clientId.remove(QLatin1Char('-'));
    m_clientId.resize(23);
}

QMqttClientPrivate::~QMqttClientPrivate()
{
}

void QMqttClientPrivate::setStateAndError(QMqttClient::ClientState s, QMqttClient::ClientError e)
{
    Q_Q(QMqttClient);

    if (s != m_state)
        q->setState(s);
    if (e != QMqttClient::NoError && m_error != e)
        q->setError(e);
}

QT_END_NAMESPACE
