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

#include "qmqttconnection_p.h"
#include "qmqttconnectionproperties_p.h"
#include "qmqttcontrolpacket_p.h"
#include "qmqttmessage_p.h"
#include "qmqttsubscription_p.h"
#include "qmqttclient_p.h"

#include <QtCore/QLoggingCategory>
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QTcpSocket>

#include <limits>
#include <cstdint>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcMqttConnection, "qt.mqtt.connection")
Q_LOGGING_CATEGORY(lcMqttConnectionVerbose, "qt.mqtt.connection.verbose");

template<>
quint32 QMqttConnection::readBufferTyped()
{
    return qFromBigEndian<quint32>(reinterpret_cast<const quint32 *>(readBuffer(4).constData()));
}

template<>
quint16 QMqttConnection::readBufferTyped()
{
    return qFromBigEndian<quint16>(reinterpret_cast<const quint16 *>(readBuffer(2).constData()));
}

template<>
quint8 QMqttConnection::readBufferTyped()
{
    return *readBuffer(1).constData();
}

template<>
QString QMqttConnection::readBufferTyped()
{
    const quint16 size = readBufferTyped<quint16>();
    return QString::fromUtf8(reinterpret_cast<const char *>(readBuffer(size).constData()), size);
}

QMqttConnection::QMqttConnection(QObject *parent) : QObject(parent)
{
    m_pingTimer.setSingleShot(false);
    m_pingTimer.connect(&m_pingTimer, &QTimer::timeout, this, &QMqttConnection::sendControlPingRequest);
}

QMqttConnection::~QMqttConnection()
{
    if (m_internalState == BrokerConnected)
        sendControlDisconnect();

    if (m_ownTransport && m_transport)
        delete m_transport;
}

void QMqttConnection::setTransport(QIODevice *device, QMqttClient::TransportType transport)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << device << " Type:" << transport;

    if (m_transport) {
        disconnect(m_transport, &QIODevice::aboutToClose, this, &QMqttConnection::transportConnectionClosed);
        disconnect(m_transport, &QIODevice::readyRead, this, &QMqttConnection::transportReadReady);
        if (m_ownTransport)
            delete m_transport;
    }

    m_transport = device;
    m_transportType = transport;
    m_ownTransport = false;

    connect(m_transport, &QIODevice::aboutToClose, this, &QMqttConnection::transportConnectionClosed);
    connect(m_transport, &QIODevice::readyRead, this, &QMqttConnection::transportReadReady);
}

QIODevice *QMqttConnection::transport() const
{
    return m_transport;
}

bool QMqttConnection::ensureTransport(bool createSecureIfNeeded)
{
    Q_UNUSED(createSecureIfNeeded); // QT_NO_SSL
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << m_transport;

    if (m_transport) {
        if (m_ownTransport)
            delete m_transport;
        else
            return true;
    }

    // We are asked to create a transport layer
    if (m_clientPrivate->m_hostname.isEmpty() || m_clientPrivate->m_port == 0) {
        qWarning("Trying to create a transport layer, but no hostname is specified");
        return false;
    }
    auto socket =
#ifndef QT_NO_SSL
            createSecureIfNeeded ? new QSslSocket() :
#endif
                                   new QTcpSocket();
    m_transport = socket;
    m_ownTransport = true;
    m_transportType =
#ifndef QT_NO_SSL
        createSecureIfNeeded ? QMqttClient::SecureSocket :
#endif
                               QMqttClient::AbstractSocket;

    connect(socket, &QAbstractSocket::connected, this, &QMqttConnection::transportConnectionEstablished);
    connect(socket, &QAbstractSocket::disconnected, this, &QMqttConnection::transportConnectionClosed);
    connect(m_transport, &QIODevice::aboutToClose, this, &QMqttConnection::transportConnectionClosed);
    connect(m_transport, &QIODevice::readyRead, this, &QMqttConnection::transportReadReady);
    return true;
}

bool QMqttConnection::ensureTransportOpen(const QString &sslPeerName)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << m_transportType;

    if (m_transportType == QMqttClient::IODevice) {
        if (m_transport->isOpen())
            return sendControlConnect();

        if (!m_transport->open(QIODevice::ReadWrite)) {
            qWarning("Could not open Transport IO device");
            m_internalState = BrokerDisconnected;
            return false;
        }
        return sendControlConnect();
    } else if (m_transportType == QMqttClient::AbstractSocket) {
        auto socket = dynamic_cast<QTcpSocket*>(m_transport);
        Q_ASSERT(socket);
        if (socket->state() == QAbstractSocket::ConnectedState)
            return sendControlConnect();

        m_internalState = BrokerConnecting;
        socket->connectToHost(m_clientPrivate->m_hostname, m_clientPrivate->m_port);
    }
#ifndef QT_NO_SSL
    else if (m_transportType == QMqttClient::SecureSocket) {
        auto socket = dynamic_cast<QSslSocket*>(m_transport);
        Q_ASSERT(socket);
        if (socket->state() == QAbstractSocket::ConnectedState)
            return sendControlConnect();

        m_internalState = BrokerConnecting;
        socket->connectToHostEncrypted(m_clientPrivate->m_hostname, m_clientPrivate->m_port, sslPeerName);

        if (!socket->waitForConnected()) {
            qWarning("Could not establish socket connection for transport");
            return false;
        }

        if (!socket->waitForEncrypted()) {
            qWarning("Could not initiate encryption.");
            return false;
        }
    }
#else
    Q_UNUSED(sslPeerName);
#endif

    return true;
}

bool QMqttConnection::sendControlConnect()
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO;

    QMqttControlPacket packet(QMqttControlPacket::CONNECT);

    // Variable header
    // 3.1.2.1 Protocol Name
    // 3.1.2.2 Protocol Level
    switch (m_clientPrivate->m_protocolVersion) {
    case QMqttClient::MQTT_3_1:
        packet.append("MQIsdp");
        packet.append(char(3)); // Version 3.1
        break;
    case QMqttClient::MQTT_3_1_1:
        packet.append("MQTT");
        packet.append(char(4)); // Version 3.1.1
        break;
    case QMqttClient::MQTT_5_0:
        packet.append("MQTT");
        packet.append(char(5)); // Version 5.0
        break;
    default:
        qCWarning(lcMqttConnection) << "Illegal MQTT Version";
        m_clientPrivate->setStateAndError(QMqttClient::Disconnected, QMqttClient::InvalidProtocolVersion);
        return false;
    }

    // 3.1.2.3 Connect Flags
    quint8 flags = 0;
    // Clean session
    if (m_clientPrivate->m_cleanSession)
        flags |= 1 << 1;

    if (!m_clientPrivate->m_willMessage.isEmpty()) {
        flags |= 1 << 2;
        if (m_clientPrivate->m_willQoS > 2) {
            qWarning("Will QoS does not have a valid value");
            return false;
        }
        if (m_clientPrivate->m_willQoS == 1)
            flags |= 1 << 3;
        else if (m_clientPrivate->m_willQoS == 2)
            flags |= 1 << 4;
        if (m_clientPrivate->m_willRetain)
            flags |= 1 << 5;
    }
    if (m_clientPrivate->m_username.size())
        flags |= 1 << 7;

    if (m_clientPrivate->m_password.size())
        flags |= 1 << 6;

    packet.append(char(flags));

    // 3.1.2.10 Keep Alive
    packet.append(m_clientPrivate->m_keepAlive);

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        packet.appendRaw(writeConnectProperties());

    // 3.1.3 Payload
    // 3.1.3.1 Client Identifier
    const QByteArray clientStringArray = m_clientPrivate->m_clientId.toUtf8();
    if (clientStringArray.size()) {
        packet.append(clientStringArray);
    } else {
        packet.append(char(0));
        packet.append(char(0));
    }

    if (!m_clientPrivate->m_willMessage.isEmpty()) {
        if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0) {
            // ### TODO: Add Will properties
            packet.append(char(0));
        }

        packet.append(m_clientPrivate->m_willTopic.toUtf8());
        packet.append(m_clientPrivate->m_willMessage);
    }

    if (m_clientPrivate->m_username.size())
        packet.append(m_clientPrivate->m_username.toUtf8());

    if (m_clientPrivate->m_password.size())
        packet.append(m_clientPrivate->m_password.toUtf8());

    if (!writePacketToTransport(packet)) {
        qWarning("Could not write CONNECT frame to transport");
        return false;
    }

    m_internalState = BrokerWaitForConnectAck;
    return true;
}

qint32 QMqttConnection::sendControlPublish(const QMqttTopicName &topic,
                                           const QByteArray &message,
                                           quint8 qos,
                                           bool retain,
                                           const QMqttPublishProperties &properties)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << topic << " Size:" << message.size() << " bytes."
                              << "QoS:" << qos << " Retain:" << retain;

    if (!topic.isValid())
        return -1;

    quint8 header = QMqttControlPacket::PUBLISH;
    if (qos == 1)
        header |= 0x02;
    else if (qos == 2)
        header |= 0x04;

    if (retain)
        header |= 0x01;

    QSharedPointer<QMqttControlPacket> packet(new QMqttControlPacket(header));
    packet->append(topic.name().toUtf8());
    quint16 identifier = 0;
    if (qos > 0) {
        identifier = unusedPacketIdentifier();
        packet->append(identifier);
        m_pendingMessages.insert(identifier, packet);
    }

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        packet->appendRaw(writePublishProperties(properties));

    packet->appendRaw(message);

    const bool written = writePacketToTransport(*packet.data());

    if (!written && qos > 0)
        m_pendingMessages.remove(identifier);
    return written ? identifier : -1;
}

bool QMqttConnection::sendControlPublishAcknowledge(quint16 id)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << id;
    QMqttControlPacket packet(QMqttControlPacket::PUBACK);
    packet.append(id);
    return writePacketToTransport(packet);
}

bool QMqttConnection::sendControlPublishRelease(quint16 id)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << id;
    quint8 header = QMqttControlPacket::PUBREL;
    header |= 0x02; // MQTT-3.6.1-1

    QMqttControlPacket packet(header);
    packet.append(id);
    return writePacketToTransport(packet);
}

bool QMqttConnection::sendControlPublishReceive(quint16 id)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << id;
    QMqttControlPacket packet(QMqttControlPacket::PUBREC);
    packet.append(id);
    return writePacketToTransport(packet);
}

bool QMqttConnection::sendControlPublishComp(quint16 id)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << id;
    QMqttControlPacket packet(QMqttControlPacket::PUBCOMP);
    packet.append(id);
    return writePacketToTransport(packet);
}

QMqttSubscription *QMqttConnection::sendControlSubscribe(const QMqttTopicFilter &topic,
                                                         quint8 qos,
                                                         const QMqttSubscriptionProperties &properties)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << " Topic:" << topic << " qos:" << qos;

    if (m_activeSubscriptions.contains(topic))
        return m_activeSubscriptions[topic];

    // has to have 0010 as bits 3-0, maybe update SUBSCRIBE instead?
    // MQTT-3.8.1-1
    const quint8 header = QMqttControlPacket::SUBSCRIBE + 0x02;
    QMqttControlPacket packet(header);

    // Add Packet Identifier
    const quint16 identifier = unusedPacketIdentifier();

    packet.append(identifier);

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        packet.appendRaw(writeSubscriptionProperties(properties));

    // Overflow protection
    if (!topic.isValid()) {
        qWarning("Subscribed topic filter is not valid.");
        return nullptr;
    }

    packet.append(topic.filter().toUtf8());

    switch (qos) {
    case 0: packet.append(char(0x0)); break;
    case 1: packet.append(char(0x1)); break;
    case 2: packet.append(char(0x2)); break;
    default: return nullptr;
    }

    auto result = new QMqttSubscription(this);
    result->setTopic(topic);
    result->setClient(m_clientPrivate->m_client);
    result->setQos(qos);
    result->setState(QMqttSubscription::SubscriptionPending);

    if (!writePacketToTransport(packet)) {
        delete result;
        return nullptr;
    }

    // SUBACK must contain identifier MQTT-3.8.4-2
    m_pendingSubscriptionAck.insert(identifier, result);
    m_activeSubscriptions.insert(result->topic(), result);
    return result;
}

bool QMqttConnection::sendControlUnsubscribe(const QMqttTopicFilter &topic)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << " Topic:" << topic;

    // MQTT-3.10.3-2
    if (!topic.isValid())
        return false;

    if (!m_activeSubscriptions.contains(topic))
        return false;

    if (m_internalState != QMqttConnection::BrokerConnected) {
        m_activeSubscriptions.remove(topic);
        return true;
    }

    // has to have 0010 as bits 3-0, maybe update UNSUBSCRIBE instead?
    // MQTT-3.10.1-1
    const quint8 header = QMqttControlPacket::UNSUBSCRIBE + 0x02;
    QMqttControlPacket packet(header);

    // Add Packet Identifier
    const quint16 identifier = unusedPacketIdentifier();

    packet.append(identifier);

    packet.append(topic.filter().toUtf8());
    auto sub = m_activeSubscriptions[topic];
    sub->setState(QMqttSubscription::UnsubscriptionPending);

    if (!writePacketToTransport(packet))
        return false;

    // Do not remove from m_activeSubscriptions as there might be QoS1/2 messages to still
    // be sent before UNSUBSCRIBE is acknowledged.
    m_pendingUnsubscriptions.insert(identifier, sub);

    return true;
}

bool QMqttConnection::sendControlPingRequest()
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO;

    if (m_internalState != QMqttConnection::BrokerConnected)
        return false;

    const QMqttControlPacket packet(QMqttControlPacket::PINGREQ);
    if (!writePacketToTransport(packet)) {
        qWarning("Could not write DISCONNECT frame to transport");
        return false;
    }
    return true;
}

bool QMqttConnection::sendControlDisconnect()
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO;

    m_pingTimer.stop();

    m_activeSubscriptions.clear();

    const QMqttControlPacket packet(QMqttControlPacket::DISCONNECT);
    if (!writePacketToTransport(packet)) {
        qWarning("Could not write DISCONNECT frame to transport");
        return false;
    }
    m_internalState = BrokerDisconnected;

    if (m_transport->waitForBytesWritten(30000)) {
        // MQTT-3.14.4-1 must disconnect
        m_transport->close();
        return true;
    }
    return false;
}

void QMqttConnection::setClientPrivate(QMqttClientPrivate *clientPrivate)
{
    m_clientPrivate = clientPrivate;
}

quint16 QMqttConnection::unusedPacketIdentifier() const
{
    // MQTT-2.3.1-1 Control Packets MUST contain a non-zero 16-bit Packet Identifier
    static quint16 packetIdentifierCounter = 1;
    const std::uint16_t u16max = std::numeric_limits<std::uint16_t>::max();

    // MQTT-2.3.1-2 ...it MUST assign it a currently unused Packet Identifier
    const quint16 lastIdentifier = packetIdentifierCounter;
    do {
        if (packetIdentifierCounter == u16max)
            packetIdentifierCounter = 1;
        else
            packetIdentifierCounter++;

        if (lastIdentifier == packetIdentifierCounter) {
            qWarning("Can't generate unique packet identifier.");
            break;
        }
    } while (m_pendingSubscriptionAck.contains(packetIdentifierCounter)
             || m_pendingUnsubscriptions.contains(packetIdentifierCounter)
             || m_pendingMessages.contains(packetIdentifierCounter)
             || m_pendingReleaseMessages.contains(packetIdentifierCounter));
    return packetIdentifierCounter;
}

void QMqttConnection::cleanSubscriptions()
{
    for (auto item : m_pendingSubscriptionAck)
        item->setState(QMqttSubscription::Unsubscribed);
    m_pendingSubscriptionAck.clear();

    for (auto item : m_pendingUnsubscriptions)
        item->setState(QMqttSubscription::Unsubscribed);
    m_pendingUnsubscriptions.clear();

    for (auto item : m_activeSubscriptions)
        item->setState(QMqttSubscription::Unsubscribed);
    m_activeSubscriptions.clear();
}

void QMqttConnection::transportConnectionEstablished()
{
    if (m_internalState != BrokerConnecting) {
        qCWarning(lcMqttConnection) << "Connection established at an unexpected time";
        return;
    }

    if (!sendControlConnect()) {
        qWarning("Could not send CONNECT to broker");
        // ### Who disconnects now? Connection or client?
        m_clientPrivate->setStateAndError(QMqttClient::Disconnected, QMqttClient::TransportInvalid);
    }
}

void QMqttConnection::transportConnectionClosed()
{
    m_readBuffer.clear();
    m_readPosition = 0;
    m_pingTimer.stop();
    m_clientPrivate->setStateAndError(QMqttClient::Disconnected, QMqttClient::TransportInvalid);
}

void QMqttConnection::transportReadReady()
{
    qCDebug(lcMqttConnectionVerbose) << Q_FUNC_INFO;
    m_readBuffer.append(m_transport->readAll());
    processData();
}

void QMqttConnection::readBuffer(char *data, qint64 size)
{
    memcpy(data, m_readBuffer.constData() + m_readPosition, size);
    m_readPosition += size;
}

qint32 QMqttConnection::readVariableByteInteger(qint32 *byteCount)
{
    quint32 multiplier = 1;
    quint32 msgLength = 0;
    quint8 b = 0;
    quint8 iteration = 0;
    if (byteCount)
        *byteCount = 0;
    do {
        b = readBufferTyped<quint8>();
        msgLength += (b & 127) * multiplier;
        multiplier *= 128;
        iteration++;
        if (iteration > 4) {
            qWarning("Publish message is too big to handle");
            closeConnection(QMqttClient::ProtocolViolation);
            return -1;
        }
    } while ((b & 128) != 0);
    if (byteCount)
        *byteCount += iteration;
    return msgLength;
}

void QMqttConnection::closeConnection(QMqttClient::ClientError error)
{
    m_readBuffer.clear();
    m_readPosition = 0;
    m_pingTimer.stop();
    m_activeSubscriptions.clear();
    m_internalState = BrokerDisconnected;
    m_transport->disconnect();
    m_transport->close();
    m_clientPrivate->setStateAndError(QMqttClient::Disconnected, error);
}

QByteArray QMqttConnection::readBuffer(qint64 size)
{
    QByteArray res(m_readBuffer.constData() + m_readPosition, size);
    m_readPosition += size;
    return res;
}

void QMqttConnection::readConnackProperties()
{
    qint32 propertyLength = readVariableByteInteger();
    m_missingData = 0;

    QMqttServerConnectionProperties serverProperties;
    serverProperties.serverData->valid = true;

    while (propertyLength > 0) {
        quint8 propertyId = readBufferTyped<quint8>();
        propertyLength--;
        switch (propertyId) {
        case 0x11: { // 3.2.2.3.2 Session Expiry Interval
            const quint32 expiryInterval = readBufferTyped<quint32>();
            propertyLength -= 4;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::SessionExpiryInterval;
            serverProperties.setSessionExpiryInterval(expiryInterval);
            break;
        }
        case 0x21: { // 3.2.2.3.3 Receive Maximum
            const quint16 receiveMaximum = readBufferTyped<quint16>();
            propertyLength -=2;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::MaximumReceive;
            serverProperties.setMaximumReceive(receiveMaximum);
            break;
        }
        case 0x24: { // 3.2.2.3.4 Maximum QoS Level
            const quint8 maxQoS = readBufferTyped<quint8>();
            propertyLength--;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::MaximumQoS;
            serverProperties.serverData->maximumQoS = maxQoS;
            break;
        }
        case 0x25: { // 3.2.2.3.5 Retain available
            const quint8 retainAvailable = readBufferTyped<quint8>();
            propertyLength--;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::RetainAvailable;
            serverProperties.serverData->retainAvailable = retainAvailable == 1 ? true : false;
            break;
        }
        case 0x27: { // 3.2.2.3.6 Maximum packet size
            const quint32 maxPacketSize = readBufferTyped<quint32>();
            propertyLength -= 4;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::MaximumPacketSize;
            serverProperties.setMaximumPacketSize(maxPacketSize);
            break;
        }
        case 0x12: { // 3.2.2.3.7 Assigned clientId
            const QString assignedClientId = readBufferTyped<QString>();
            propertyLength -= assignedClientId.length() + 2;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::AssignedClientId;
            m_clientPrivate->m_client->setClientId(assignedClientId);
            break;
        }
        case 0x22: { // 3.2.2.3.8 Topic Alias Maximum
            const quint16 topicAliasMaximum = readBufferTyped<quint16>();
            propertyLength -=2;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::MaximumTopicAlias;
            serverProperties.setMaximumTopicAlias(topicAliasMaximum);
            break;
        }
        case 0x1F: { // 3.2.2.3.9 Reason String
            const QString reasonString = readBufferTyped<QString>();
            propertyLength -= reasonString.length() + 2;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::ReasonString;
            serverProperties.serverData->reasonString = reasonString;
            break;
        }
        case 0x26: { // 3.2.2.3.10 User property
            const QString propertyName = readBufferTyped<QString>();
            propertyLength -= propertyName.length() + 2;

            const QString propertyValue = readBufferTyped<QString>();
            propertyLength -= propertyValue.length() + 2;

            serverProperties.serverData->details |= QMqttServerConnectionProperties::UserProperty;
            serverProperties.data->userProperties.append(QMqttStringPair(propertyName, propertyValue));
            break;
        }
        case 0x28: { // 3.2.2.3.11 Wildcard subscriptions available
            const quint8 available = readBufferTyped<quint8>();
            propertyLength--;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::WildCardSupported;
            serverProperties.serverData->wildcardSupported = available == 1 ? true : false;
            break;
        }
        case 0x29: { // 3.2.2.3.12 Subscription identifiers available
            const quint8 available = readBufferTyped<quint8>();
            propertyLength--;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::SubscriptionIdentifierSupport;
            serverProperties.serverData->subscriptionIdentifierSupported = available == 1 ? true : false;
            break;
        }
        case 0x2A: { // 3.2.2.3.13 Shared subscriptions available
            const quint8 available = readBufferTyped<quint8>();
            propertyLength--;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::SharedSubscriptionSupport;
            serverProperties.serverData->sharedSubscriptionSupported = available == 1 ? true : false;
            break;
        }
        case 0x13: { // 3.2.2.3.14 Server Keep Alive
            const quint16 serverKeepAlive = readBufferTyped<quint16>();
            propertyLength -=2;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::ServerKeepAlive;
            m_clientPrivate->m_client->setKeepAlive(serverKeepAlive);
            break;
        }
        case 0x1A: { // 3.2.2.3.15 Response information
            const QString responseInfo = readBufferTyped<QString>();
            propertyLength -= responseInfo.length();
            serverProperties.serverData->details |= QMqttServerConnectionProperties::ResponseInformation;
            serverProperties.serverData->responseInformation = responseInfo;
            break;
        }
        case 0x1C: { // 3.2.2.3.16 Server reference
            const QString serverReference = readBufferTyped<QString>();
            propertyLength -= serverReference.length() + 2;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::ServerReference;
            serverProperties.serverData->serverReference = serverReference;
            break;
        }
        case 0x15: { // 3.2.2.3.17 Authentication method
            const QString method = readBufferTyped<QString>();
            propertyLength -= method.length() + 2;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::AuthenticationMethod;
            serverProperties.data->authenticationMethod = method;
            break;
        }
        case 0x16: { // 3.2.2.3.18 Authentication data
            const quint16 dataLength = readBufferTyped<quint16>();
            propertyLength -=2;
            const QByteArray data = readBuffer(dataLength);
            propertyLength -= dataLength;
            serverProperties.serverData->details |= QMqttServerConnectionProperties::AuthenticationData;
            serverProperties.data->authenticationData = data;
            break;
        }
        default:
            qWarning() << "Unknown property id in CONNACK:" << int(propertyId);
            break;
        }
    }
    m_clientPrivate->m_serverConnectionProperties = serverProperties;
}

void QMqttConnection::readPublishProperties(QMqttPublishProperties &properties)
{
    qint32 propertySize = 0;
    qint32 propertyLength = readVariableByteInteger(&propertySize);
    m_missingData -= propertySize;
    m_missingData -= propertyLength;

    QMqttUserProperties userProperties;

    while (propertyLength > 0) {
        const quint8 propertyId = readBufferTyped<quint8>();
        propertyLength--;
        switch (propertyId) {
        case 0x01: { // 3.3.2.3.2 Payload Format Indicator
            const quint8 format = readBufferTyped<quint8>();
            propertyLength--;
            if (format == 1)
                properties.setPayloadIndicator(QMqttPublishProperties::UTF8Encoded);
            break;
        }
        case 0x02: { // 3.3.2.3.3 Message Expiry Interval
            const quint32 interval = readBufferTyped<quint32>();
            propertyLength -= 4;
            properties.setMessageExpiryInterval(interval);
            break;
        }
        case 0x23: { // 3.3.2.3.4 Topic alias
            const quint16 alias = readBufferTyped<quint16>();
            propertyLength -= 2;
            properties.setTopicAlias(alias);
            break;
        }
        case 0x08: { // 3.3.2.3.5 Response Topic
            const QString responseTopic = readBufferTyped<QString>();
            propertyLength -= responseTopic.length() + 2;
            properties.setResponseTopic(responseTopic);
            break;
        }
        case 0x09: { // 3.3.2.3.6 Correlation Data
            const quint16 length = readBufferTyped<quint16>();
            propertyLength -=2;
            const QByteArray data = readBuffer(length);
            propertyLength -= length;
            properties.setCorrelationData(data);
            break;
        }
        case 0x26: { // 3.3.2.3.7 User property
            const QString propertyName = readBufferTyped<QString>();
            propertyLength -= propertyName.length() + 2;

            const QString propertyValue = readBufferTyped<QString>();
            propertyLength -= propertyValue.length() + 2;
            userProperties.append(QMqttStringPair(propertyName, propertyValue));
            break;
        }
        case 0x0b: { // 3.3.2.3.8 Subscription Identifier
            qint32 idSize = 0;
            quint32 id = readVariableByteInteger(&idSize);
            propertyLength -= idSize;
            properties.setSubscriptionIdentifier(id);
            break;
        }
        case 0x03: { // 3.3.2.3.9 Content Type
            const QString content = readBufferTyped<QString>();
            propertyLength -= content.length() + 2;
            properties.setContentType(content);
            break;
        }
        default:
            qWarning("Unknown publish property received");
            break;
        }
    }
    if (!userProperties.isEmpty())
        properties.setUserProperties(userProperties);
}

void QMqttConnection::readSubscriptionProperties(QMqttSubscription *sub)
{
    qint32 propertyLength = readVariableByteInteger();

    m_missingData -= propertyLength;
    while (propertyLength > 0) {
        const quint8 propertyId = readBufferTyped<quint8>();
        propertyLength--;
        switch (propertyId) {
        case 0x1f: { // 3.9.2.1.2 Reason String
            const QString content = readBufferTyped<QString>();
            propertyLength -= content.length() + 2;
            sub->d_func()->m_reasonString = content;
            break;
        }
        case 0x26: { // 3.9.2.1.3
            const QString propertyName = readBufferTyped<QString>();
            propertyLength -= propertyName.length() + 2;

            const QString propertyValue = readBufferTyped<QString>();
            propertyLength -= propertyValue.length() + 2;

            sub->d_func()->m_userProperties.append(QMqttStringPair(propertyName, propertyValue));
            break;
        }
        default:
            qWarning("Unknown subscription property received");
            break;
        }
    }
}

QByteArray QMqttConnection::writeConnectProperties()
{
    QMqttControlPacket properties;

    // According to MQTT5 3.1.2.11 default values do not need to be included in the
    // connect statement.

    // 3.1.2.11.2
    if (m_clientPrivate->m_connectionProperties.sessionExpiryInterval() != 0) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify sessionExpiryInterval";
        properties.append(char(0x11));
        properties.append(m_clientPrivate->m_connectionProperties.sessionExpiryInterval());
    }

    // 3.1.2.11.3
    if (m_clientPrivate->m_connectionProperties.maximumReceive() != 65535) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify maximumReceive";
        properties.append(char(0x21));
        properties.append(m_clientPrivate->m_connectionProperties.maximumReceive());
    }

    // 3.1.2.11.4
    if (m_clientPrivate->m_connectionProperties.maximumPacketSize() != std::numeric_limits<quint32>::max()) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify maximumPacketSize";
        properties.append(char(0x27));
        properties.append(m_clientPrivate->m_connectionProperties.maximumPacketSize());
    }

    // 3.1.2.11.5
    if (m_clientPrivate->m_connectionProperties.maximumTopicAlias() != 0) {
        // ### TODO: Verify this works, previous test server versions did set it to 2 if no value
        // specified
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify maximumTopicAlias";
        properties.append(char(0x22));
        properties.append(m_clientPrivate->m_connectionProperties.maximumTopicAlias());
    }

    // 3.1.2.11.6
    if (m_clientPrivate->m_connectionProperties.requestResponseInformation()) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify requestResponseInformation";
        properties.append(char(0x19));
        properties.append(char(1));
    }

    // 3.1.2.11.7
    if (!m_clientPrivate->m_connectionProperties.requestProblemInformation()) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify requestProblemInformation";
        properties.append(char(0x17));
        properties.append(char(0));
    }

    // 3.1.2.11.8 Add User properties
    auto userProperties = m_clientPrivate->m_connectionProperties.userProperties();
    if (!userProperties.isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify user properties";
        for (auto it = userProperties.constBegin();
             it != userProperties.constEnd(); ++it) {
            properties.append(char(0x26));
            properties.append(it->name().toUtf8());
            properties.append(it->value().toUtf8());
        }
    }

    // 3.1.2.11.9 Add Authentication
    const QString authenticationMethod = m_clientPrivate->m_connectionProperties.authenticationMethod();
    if (!authenticationMethod.isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Connection Properties: specify AuthenticationMethod:";
        qCDebug(lcMqttConnectionVerbose) << "    " << authenticationMethod;
        properties.append(char(0x15));
        properties.append(authenticationMethod.toUtf8());
        // 3.1.2.11.10
        const QByteArray authenticationData = m_clientPrivate->m_connectionProperties.authenticationData();
        if (!authenticationData.isEmpty()) {
            qCDebug(lcMqttConnectionVerbose) << "Connection Properties: Authentication Data:";
            qCDebug(lcMqttConnectionVerbose) << "    " << authenticationData;
            properties.append(char(0x16));
            properties.append(authenticationData);
        }
    }

    return properties.serializePayload();
}

QByteArray QMqttConnection::writePublishProperties(const QMqttPublishProperties &properties)
{
    QMqttControlPacket packet;

    // 3.3.2.3.2 Payload Indicator
    if (properties.availableProperties() & QMqttPublishProperties::PayloadFormatIndicator &&
            properties.payloadIndicator() != QMqttPublishProperties::Unspecified) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Payload Indicator:"
                                         << properties.payloadIndicator();
        packet.append(char(0x01));
        switch (properties.payloadIndicator()) {
        case QMqttPublishProperties::UTF8Encoded:
            packet.append(char(0x01));
            break;
        default:
            qWarning("Unknown payload indicator");
            break;
        }
    }

    // 3.3.2.3.3 Message Expiry
    if (properties.availableProperties() & QMqttPublishProperties::MessageExpiryInterval &&
            properties.messageExpiryInterval() > 0) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Message Expiry :"
                                         << properties.messageExpiryInterval();
        packet.append(char(0x02));
        packet.append(properties.messageExpiryInterval());
    }

    // 3.3.2.3.4 Topic alias
    if (properties.availableProperties() & QMqttPublishProperties::TopicAlias &&
            properties.topicAlias() > 0) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Topic Alias :"
                                         << properties.topicAlias();
        if (m_clientPrivate->m_serverConnectionProperties.availableProperties() & QMqttServerConnectionProperties::MaximumTopicAlias
                && properties.topicAlias() > m_clientPrivate->m_serverConnectionProperties.maximumTopicAlias()) {
            qWarning() << "Invalid topic alias specified: " << properties.topicAlias()
                       << " Maximum by server is:" << m_clientPrivate->m_serverConnectionProperties.maximumTopicAlias();

        } else {
            packet.append(char(0x23));
            packet.append(properties.topicAlias());
        }
    }

    // 3.3.2.3.5 Response Topic
    if (properties.availableProperties() & QMqttPublishProperties::ResponseTopic &&
            !properties.responseTopic().isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Response Topic :"
                                         << properties.responseTopic();
        packet.append(char(0x08));
        packet.append(properties.responseTopic().toUtf8());
    }

    // 3.3.2.3.6 Correlation Data
    if (properties.availableProperties() & QMqttPublishProperties::CorrelationData &&
            !properties.correlationData().isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Correlation Data :"
                                         << properties.correlationData();
        packet.append(char(0x09));
        packet.append(properties.correlationData());
    }

    // 3.3.2.3.7 User Property
    if (properties.availableProperties() & QMqttPublishProperties::UserProperty) {
        auto userProperties = properties.userProperties();
        if (!userProperties.isEmpty()) {
            qCDebug(lcMqttConnectionVerbose) << "Publish Properties: specify user properties";
            for (auto it = userProperties.constBegin();
                 it != userProperties.constEnd(); ++it) {
                packet.append(char(0x26));
                packet.append(it->name().toUtf8());
                packet.append(it->value().toUtf8());
            }
        }
    }

    // 3.3.2.3.8 Subscription Identifier
    if (properties.availableProperties() & QMqttPublishProperties::SubscriptionIdentifier &&
            properties.subscriptionIdentifier() > 0) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Subscription ID:"
                                         << properties.subscriptionIdentifier();
        packet.append(char(0x0b));
        packet.appendRawVariableInteger(properties.subscriptionIdentifier());
    }

    // 3.3.2.3.9 Content Type
    if (properties.availableProperties() & QMqttPublishProperties::ContentType &&
            !properties.contentType().isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Publish Properties: Content Type :"
                                         << properties.contentType();
        packet.append(char(0x03));
        packet.append(properties.contentType().toUtf8());
    }

    return packet.serializePayload();
}

QByteArray QMqttConnection::writeSubscriptionProperties(const QMqttSubscriptionProperties &properties)
{
    QMqttControlPacket packet;

    // 3.8.2.1.2 Subscription Identifier
    if (properties.subscriptionIdentifier() > 0) {
        qCDebug(lcMqttConnectionVerbose) << "Subscription Properties: Subscription Identifier";
        packet.append(char(0x0b));
        packet.appendRawVariableInteger(properties.subscriptionIdentifier());
    }

    // 3.8.2.1.3 User Property
    auto userProperties = properties.userProperties();
    if (!userProperties.isEmpty()) {
        qCDebug(lcMqttConnectionVerbose) << "Subscription Properties: specify user properties";
        for (auto it : userProperties) {
            packet.append(char(0x26));
            packet.append(it.name().toUtf8());
            packet.append(it.value().toUtf8());
        }
    }

    return packet.serializePayload();
}

void QMqttConnection::finalize_connack()
{
    qCDebug(lcMqttConnectionVerbose) << "Finalize CONNACK";

    const quint8 ackFlags = readBufferTyped<quint8>();
    m_missingData--;

    if (ackFlags > 1) { // MQTT-3.2.2.1
        qWarning("Unexpected CONNACK Flags set");
        readBuffer(m_missingData);
        m_missingData = 0;
        closeConnection(QMqttClient::ProtocolViolation);
        return;
    }
    bool sessionPresent = ackFlags == 1;

    // MQTT-3.2.2-1 & MQTT-3.2.2-2
    if (sessionPresent) {
        emit m_clientPrivate->m_client->brokerSessionRestored();
        if (m_clientPrivate->m_cleanSession)
            qWarning("Connected with a clean session, ack contains session present");
    } else {
        // MQTT-4.1.0.-1 MQTT-4.1.0-2 Session not stored on broker side
        // regardless whether cleanSession is false
        cleanSubscriptions();
    }

    quint8 connectResultValue = readBufferTyped<quint8>();
    m_missingData--;
    if (connectResultValue != 0) {
        qWarning("Connection has been rejected");
        // MQTT-3.2.2-5
        m_readBuffer.clear();
        m_readPosition = 0;
        m_transport->close();
        m_internalState = BrokerDisconnected;
        // Table 3.1, values 1-5
        m_clientPrivate->setStateAndError(QMqttClient::Disconnected, static_cast<QMqttClient::ClientError>(connectResultValue));
        return;
    }

    // MQTT 5.0 has variable part != 2 in the header
    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        readConnackProperties();

    m_internalState = BrokerConnected;
    m_clientPrivate->setStateAndError(QMqttClient::Connected);

    m_pingTimer.setInterval(m_clientPrivate->m_keepAlive * 1000);
    m_pingTimer.start();
}

void QMqttConnection::finalize_suback()
{
    const quint16 id = readBufferTyped<quint16>();
    m_missingData -= 2;
    if (!m_pendingSubscriptionAck.contains(id)) {
        qWarning("Received SUBACK for unknown subscription request");
        return;
    }

    auto sub = m_pendingSubscriptionAck.take(id);

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        readSubscriptionProperties(sub);

    while (m_missingData > 0) {
        quint8 reason = readBufferTyped<quint8>();
        m_missingData--;

        qCDebug(lcMqttConnectionVerbose) << "Finalize SUBACK: id:" << id << "qos:" << reason;
        if (reason <= 2) {
            // The broker might have a different support level for QoS than what
            // the client requested
            if (reason != sub->qos()) {
                sub->setQos(reason);
                emit sub->qosChanged(reason);
            }
            sub->setState(QMqttSubscription::Subscribed);
        } else if (reason == 0x80) {
            qWarning() << "Subscription for id " << id << " failed.";
            sub->setState(QMqttSubscription::Error);
        } else {
            bool mqtt5reason = false;
            if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0) {
                mqtt5reason = true;
                switch (reason) {
                case 0x83:
                    qWarning() << "Implementation specific error for id:" << id;
                    sub->setState(QMqttSubscription::Error);
                    break;
                case 0x87:
                    qWarning() << "Not authorized for id:" << id;
                    sub->setState(QMqttSubscription::Error);
                    break;
                case 0x8F:
                    qWarning() << "Topic filter invalid:" << id;
                    sub->setState(QMqttSubscription::Error);
                    break;
                case 0x91:
                    qWarning() << "Packet identifier in use:" << id;
                    sub->setState(QMqttSubscription::Error);
                    break;
                case 0x97:
                    qWarning() << "Quota exceeded:" << id;
                    sub->setState(QMqttSubscription::Error);
                    break;
                case 0x9E:
                    qWarning() << "Shared subscriptions not supported:" << id;
                    sub->setState(QMqttSubscription::Error);
                    break;
                case 0xA1:
                    qWarning() << "Subscription Identifiers not supported:" << id;
                    sub->setState(QMqttSubscription::Error);
                    break;
                case 0xA2:
                    qWarning() << "Wildcard subscriptions not supported:" << id;
                    sub->setState(QMqttSubscription::Error);
                    break;
                default:
                    mqtt5reason = false;
                    break;
                }
            }

            if (!mqtt5reason) {
                qWarning("Received invalid SUBACK result value");
                sub->setState(QMqttSubscription::Error);
            }
        }
    }
}

void QMqttConnection::finalize_unsuback()
{
    const quint16 id = readBufferTyped<quint16>();
    m_missingData -= 2;
    qCDebug(lcMqttConnectionVerbose) << "Finalize UNSUBACK: " << id;
    if (!m_pendingUnsubscriptions.contains(id)) {
        qWarning("Received UNSUBACK for unknown request");
        return;
    }
    auto sub = m_pendingUnsubscriptions.take(id);
    sub->setState(QMqttSubscription::Unsubscribed);
    m_activeSubscriptions.remove(sub->topic());
}

void QMqttConnection::finalize_publish()
{
    // String topic
    const QMqttTopicName topic = readBufferTyped<QString>();
    const quint16 topicLength = topic.name().length();
    m_missingData -= topicLength + 2;

    quint16 id = 0;
    if (m_currentPublish.qos > 0) {
        id = readBufferTyped<quint16>();
        m_missingData -= 2;
    }

    QMqttPublishProperties publishProperties;
    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0)
        readPublishProperties(publishProperties);

    // message
    const qint64 payloadLength = m_missingData;
    const QByteArray message = readBuffer(payloadLength);
    m_missingData -= payloadLength;

    qCDebug(lcMqttConnectionVerbose) << "Finalize PUBLISH: topic:" << topic
                                     << " payloadLength:" << payloadLength;;

    emit m_clientPrivate->m_client->messageReceived(message, topic);

    QMqttMessage qmsg(topic, message, id, m_currentPublish.qos,
                      m_currentPublish.dup, m_currentPublish.retain);
    qmsg.d->m_publishProperties = publishProperties;


    for (auto sub = m_activeSubscriptions.constBegin(); sub != m_activeSubscriptions.constEnd(); sub++) {
        if (sub.key().match(topic))
            emit sub.value()->messageReceived(qmsg);
    }

    if (m_currentPublish.qos == 1)
        sendControlPublishAcknowledge(id);
    else if (m_currentPublish.qos == 2)
        sendControlPublishReceive(id);
}

void QMqttConnection::finalize_pubAckRecComp()
{
    qCDebug(lcMqttConnectionVerbose) << "Finalize PUBACK/REC/COMP";
    const quint16 id = readBufferTyped<quint16>();
    m_missingData -= 2;

    if (m_clientPrivate->m_protocolVersion == QMqttClient::MQTT_5_0 && m_missingData > 0) {
        // Reason Code (1byte)
        const quint8 reasonCode = readBufferTyped<quint8>();
        m_missingData--;
        Q_UNUSED(reasonCode); // ### TODO: Do something with it, currently silences compiler
        // Property Length (Variable Int)
        qint32 byteCount = 0;
        const qint32 propertyLength = readVariableByteInteger(&byteCount);
        m_missingData -= byteCount;
        // ### TODO: Publish ACK/REC/COMP property handling
        if (propertyLength > 0) {
            readBuffer(propertyLength);
            m_missingData -= propertyLength;
        }
    }
    if ((m_currentPacket & 0xF0) == QMqttControlPacket::PUBCOMP) {
        qCDebug(lcMqttConnectionVerbose) << " PUBCOMP:" << id;
        auto pendingRelease = m_pendingReleaseMessages.take(id);
        if (!pendingRelease)
            qWarning("Received PUBCOMP for unknown released message");
        emit m_clientPrivate->m_client->messageSent(id);
        return;
    }

    auto pendingMsg = m_pendingMessages.take(id);
    if (!pendingMsg) {
        qWarning() << QLatin1String("Received PUBACK for unknown message: ") << id;
        return;
    }
    if ((m_currentPacket & 0xF0) == QMqttControlPacket::PUBREC) {
        qCDebug(lcMqttConnectionVerbose) << " PUBREC:" << id;
        m_pendingReleaseMessages.insert(id, pendingMsg);
        sendControlPublishRelease(id);
    } else {
        qCDebug(lcMqttConnectionVerbose) << " PUBACK:" << id;
        emit m_clientPrivate->m_client->messageSent(id);
    }
}

void QMqttConnection::finalize_pubrel()
{
    const quint16 id = readBufferTyped<quint16>();
    m_missingData -= 2;

    qCDebug(lcMqttConnectionVerbose) << "Finalize PUBREL:" << id;

    // ### TODO: send to our app now or not???
    // See standard Figure 4.3 Method A or B ???
    sendControlPublishComp(id);
}

void QMqttConnection::finalize_pingresp()
{
    qCDebug(lcMqttConnectionVerbose) << "Finalize PINGRESP";
    const quint8 v = readBufferTyped<quint8>();
    m_missingData--;

    if (v != 0) {
        qWarning("Received a PINGRESP with payload!");
        closeConnection(QMqttClient::ProtocolViolation);
        return;
    }
    emit m_clientPrivate->m_client->pingResponseReceived();
}

void QMqttConnection::processData()
{
    if (m_missingData > 0) {
        if ((m_readBuffer.size() - m_readPosition) < m_missingData)
            return;

        switch (m_currentPacket & 0xF0) {
        case QMqttControlPacket::CONNACK:
            finalize_connack();
            break;
        case QMqttControlPacket::SUBACK:
            finalize_suback();
            break;
        case QMqttControlPacket::UNSUBACK:
            finalize_unsuback();
            break;
        case QMqttControlPacket::PUBLISH:
            finalize_publish();
            break;
        case QMqttControlPacket::PUBACK:
        case QMqttControlPacket::PUBREC:
        case QMqttControlPacket::PUBCOMP:
            finalize_pubAckRecComp();
            break;
        case QMqttControlPacket::PINGRESP:
            finalize_pingresp();
            break;
        case QMqttControlPacket::PUBREL:
            finalize_pubrel();
            break;
        default:
            qWarning("Unknown packet to finalize");
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }

        Q_ASSERT(m_missingData == 0);

        m_readBuffer = m_readBuffer.mid(m_readPosition);
        m_readPosition = 0;
    }

    // MQTT-2.2 A fixed header of a control packet must be at least 2 bytes. If the payload is
    // longer than 127 bytes the header can be up to 5 bytes long.
    switch (m_readBuffer.size()) {
    case 0:
    case 1:
        return;
    case 2:
        if ((m_readBuffer.at(1) & 128) != 0)
            return;
        break;
    case 3:
        if ((m_readBuffer.at(1) & 128) != 0 && (m_readBuffer.at(2) & 128) != 0)
            return;
        break;
    case 4:
        if ((m_readBuffer.at(1) & 128) != 0 && (m_readBuffer.at(2) & 128) != 0 && (m_readBuffer.at(3) & 128) != 0)
            return;
        break;
    default:
        break;
    }

    readBuffer((char*)&m_currentPacket, 1);
    m_missingData--;

    switch (m_currentPacket & 0xF0) {
    case QMqttControlPacket::CONNACK: {
        qCDebug(lcMqttConnectionVerbose) << "Received CONNACK";
        if (m_internalState != BrokerWaitForConnectAck) {
            qWarning("Received CONNACK at unexpected time!");
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }

        qint32 payloadSize = readVariableByteInteger();
        if (m_clientPrivate->m_protocolVersion != QMqttClient::MQTT_5_0) {
            if (payloadSize != 2) {
                qWarning("Unexpected FRAME size in CONNACK");
                closeConnection(QMqttClient::ProtocolViolation);
                return;
            }
        }
        m_missingData = payloadSize;
        break;
    }
    case QMqttControlPacket::SUBACK: {
        qCDebug(lcMqttConnectionVerbose) << "Received SUBACK";
        const quint8 remaining = readBufferTyped<quint8>();
        m_missingData = remaining;
        break;
    }
    case QMqttControlPacket::PUBLISH: {
        qCDebug(lcMqttConnectionVerbose) << "Received PUBLISH";
        m_currentPublish.dup = m_currentPacket & 0x08;
        m_currentPublish.qos = (m_currentPacket & 0x06) >> 1;
        m_currentPublish.retain = m_currentPacket & 0x01;
        if ((m_currentPublish.qos == 0 && m_currentPublish.dup != 0)
            || m_currentPublish.qos > 2) {
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }

        m_missingData = readVariableByteInteger();
        if (m_missingData == -1)
            return; // Connection closed inside readVariableByteInteger
        break;
    }
    case QMqttControlPacket::PINGRESP:
        qCDebug(lcMqttConnectionVerbose) << "Received PINGRESP";
        m_missingData = 1;
        break;


    case QMqttControlPacket::PUBREL: {
        qCDebug(lcMqttConnectionVerbose) << "Received PUBREL";
        const quint8 remaining = readBufferTyped<quint8>();
        if (remaining != 0x02) {
            qWarning("Received 2 byte message with invalid remaining length");
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }
        if ((m_currentPacket & 0x0F) != 0x02) {
            qWarning("Malformed fixed header for PUBREL");
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }
        m_missingData = 2;
        break;
    }

    case QMqttControlPacket::UNSUBACK:
    case QMqttControlPacket::PUBACK:
    case QMqttControlPacket::PUBREC:
    case QMqttControlPacket::PUBCOMP: {
        qCDebug(lcMqttConnectionVerbose) << "Received UNSUBACK/PUBACK/PUBREC/PUBCOMP";
        if ((m_currentPacket & 0x0F) != 0) {
            qWarning("Malformed fixed header");
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }
        const quint8 remaining = readBufferTyped<quint8>();
        if (m_clientPrivate->m_protocolVersion != QMqttClient::MQTT_5_0 && remaining != 0x02) {
            qWarning("Received 2 byte message with invalid remaining length");
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }
        m_missingData = remaining;
        break;
    }
    default:
        qWarning("Received unknown command");
        closeConnection(QMqttClient::ProtocolViolation);
        return;
    }

    /* set current command CONNACK - PINGRESP */
    /* read command size */
    /* calculate missing_data */
    processData(); // implicitly finishes and enqueues
    return;
}

bool QMqttConnection::writePacketToTransport(const QMqttControlPacket &p)
{
    const QByteArray writeData = p.serialize();
    const qint64 res = m_transport->write(writeData.constData(), writeData.size());
    if (Q_UNLIKELY(res == -1)) {
        qWarning("Could not write frame to transport");
        return false;
    }
    return true;
}

QT_END_NAMESPACE
