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

#include "qmqttclient.h"
#include "qmqttclient_p.h"

#include <QtCore/QUuid>
#include <QtCore/QtEndian>

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

    The interval is specified in milliseconds. Note that most brokers are not capable of using
    such a high granularity and will fallback to an interval specified in seconds.
*/

/*!
    \property QMqttClient::protocolVersion
    \brief The MQTT standard version to use for connections.

    Specifies the version of the standard the client uses for connecting to a broker. Valid values
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
    \property QMqttClient::cleanSession
    \brief Specifies the state after connecting to a broker.
*/

/*!
    \property QMqttClient::willTopic
    \brief Specifies the topic of the will testament.
*/

/*!
    \property QMqttClient::willMessage
    \brief Specifies the payload of a will testament message.
*/

/*!
    \property QMqttClient::willQoS
    \brief Specifies the level of QoS the will testament message is send and stored.
*/

/*!
    \property QMqttClient::willRetain
    \brief Specifies whether the will testament message should retain on the broker for future
           subscribers to receive.
*/

/*!
    \enum QMqttClient::TransportType

    This enum type specifies the connection method to be used to instantiate a connection with
    a broker.

    \value IODevice
           The transport uses a class based on a QIODevice.
    \value AbstractSocket
           The transport uses a class based on a QAbstractSocket.
    \value SecureSocket
           The transport uses a class based on a QSslSocket.
*/

/*!
    \enum QMqttClient::State

    This enum type specifies the states a client can enter.

    \value Disconnected
           The client is disconnected from the broker.
    \value Connecting
           A connection request has been made, but the broker has not approved the connection yet.
    \value Connected
           The client is connected to the broker.
*/

/*!
    \enum QMqttClient::ProtocolVersion

    This enum specifies the protocol version of MQTT standard to use during communication
    with a broker.

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

    This signal is emitted when a connection has been closed. A disconnect can happen because
    of a call of connectToHost() or the broker disconnected.
*/

/*!
    \fn QMqttClient::messageReceived(const QByteArray &message, const QString &topic)

    This signal is emitted when a new message has been received. The category of the message is
    specified in \a topic with the content being \a message.
*/

/*!
    \fn QMqttClient::messageSent(qint32 id)

    A message which has been sent via \l QMqttClient::publish has been received by the broker.
    The \a id is the same as returned by \l QMqttClient::publish to help tracking of the status
    of the message.
*/

/*!
    \fn QMqttClient::pingResponse()

    If requestPing() is called or every keepAlive() milliseconds, this signal is emitted after
    the broker responded and the connection is still valid.
*/

/*!
    \fn QMqttClient::brokerSessionRestored()

    After a client has successfully connected to a broker with the cleanSession property set to
    false, and the broker is capable of restoring a session, this signal will be emitted.

    Sessions can be restored if a client has connected previously using the same clientId.
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

    If \a retain is set to true, the message will stay on the broker for later clients to connect
    and receive this message.

    Returns an \c ID which is used internally to identify the message.
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
        setState(Disconnected);
        return;
    }
    setState(Connecting);

    if (!d->m_connection.ensureTransportOpen(sslPeerName)) {
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
