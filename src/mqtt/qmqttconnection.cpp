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
#include "qmqttcontrolpacket_p.h"
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

qint32 QMqttConnection::sendControlPublish(const QMqttTopicName &topic, const QByteArray &message, quint8 qos, bool retain)
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

QMqttSubscription *QMqttConnection::sendControlSubscribe(const QMqttTopicFilter &topic, quint8 qos)
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
    memcpy(data, m_readBuffer.constData(), size);
    m_readBuffer = m_readBuffer.mid(size);
}

void QMqttConnection::closeConnection(QMqttClient::ClientError error)
{
    m_readBuffer.clear();
    m_pingTimer.stop();
    m_activeSubscriptions.clear();
    m_internalState = BrokerDisconnected;
    m_transport->disconnect();
    m_transport->close();
    m_clientPrivate->setStateAndError(QMqttClient::Disconnected, error);
}

QByteArray QMqttConnection::readBuffer(qint64 size)
{
    QByteArray res = m_readBuffer.left(size);
    m_readBuffer = m_readBuffer.mid(size);
    return res;
}

void QMqttConnection::finalize_connack()
{
    qCDebug(lcMqttConnectionVerbose) << "Finalize CONNACK";
    quint8 ackFlags;
    readBuffer((char*)&ackFlags, 1);
    if (ackFlags > 1) { // MQTT-3.2.2.1
        qWarning("Unexpected CONNACK Flags set");
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

    quint8 connectResultValue;
    readBuffer((char*)&connectResultValue, 1);
    if (connectResultValue != 0) {
        qWarning("Connection has been rejected");
        // MQTT-3.2.2-5
        m_readBuffer.clear();
        m_transport->close();
        m_internalState = BrokerDisconnected;
        // Table 3.1, values 1-5
        m_clientPrivate->setStateAndError(QMqttClient::Disconnected, static_cast<QMqttClient::ClientError>(connectResultValue));
        return;
    }
    m_internalState = BrokerConnected;
    m_clientPrivate->setStateAndError(QMqttClient::Connected);

    m_pingTimer.setInterval(m_clientPrivate->m_keepAlive * 1000);
    m_pingTimer.start();
}

void QMqttConnection::finalize_suback()
{
    quint16 id;
    readBuffer((char*)&id, 2);
    id = qFromBigEndian<quint16>(id);
    if (!m_pendingSubscriptionAck.contains(id)) {
        qWarning("Received SUBACK for unknown subscription request");
        return;
    }
    quint8 result;
    readBuffer((char*)&result, 1);
    auto sub = m_pendingSubscriptionAck.take(id);
    qCDebug(lcMqttConnectionVerbose) << "Finalize SUBACK: id:" << id << "qos:" << result;
    if (result <= 2) {
        // The broker might have a different support level for QoS than what
        // the client requested
        if (result != sub->qos()) {
            sub->setQos(result);
            emit sub->qosChanged(result);
        }
        sub->setState(QMqttSubscription::Subscribed);
    } else if (result == 0x80) {
        qWarning() << "Subscription for id " << id << " failed.";
        sub->setState(QMqttSubscription::Error);
    } else {
        qWarning("Received invalid SUBACK result value");
        sub->setState(QMqttSubscription::Error);
    }
}

void QMqttConnection::finalize_unsuback()
{
    quint16 id;
    readBuffer((char*)&id, 2);
    id = qFromBigEndian<quint16>(id);
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
    const quint16 topicLength = qFromBigEndian<quint16>(reinterpret_cast<const quint16 *>(readBuffer(2).constData()));
    const QMqttTopicName topic = QString::fromUtf8(reinterpret_cast<const char *>(readBuffer(topicLength).constData()));

    quint16 id = 0;
    if (m_currentPublish.qos > 0) {
        id = qFromBigEndian<quint16>(reinterpret_cast<const quint16 *>(readBuffer(2).constData()));
    }

    // message
    qint64 payloadLength = m_missingData - (topicLength + 2) - (m_currentPublish.qos > 0 ? 2 : 0);
    const QByteArray message = readBuffer(payloadLength);

    qCDebug(lcMqttConnectionVerbose) << "Finalize PUBLISH: topic:" << topic
                                     << " payloadLength:" << payloadLength;;

    emit m_clientPrivate->m_client->messageReceived(message, topic);

    QMqttMessage qmsg(topic, message, id, m_currentPublish.qos,
                      m_currentPublish.dup, m_currentPublish.retain);

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
    quint16 id;
    readBuffer((char*)&id, 2);
    id = qFromBigEndian<quint16>(id);

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
    quint16 id;
    readBuffer((char*)&id, 2);
    id = qFromBigEndian<quint16>(id);

    qCDebug(lcMqttConnectionVerbose) << "Finalize PUBREL:" << id;

    // ### TODO: send to our app now or not???
    // See standard Figure 4.3 Method A or B ???
    sendControlPublishComp(id);
}

void QMqttConnection::finalize_pingresp()
{
    qCDebug(lcMqttConnectionVerbose) << "Finalize PINGRESP";
    quint8 v;
    readBuffer((char*)&v, 1);
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
        if (m_readBuffer.size() < m_missingData)
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
        m_missingData = 0;
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

    switch (m_currentPacket & 0xF0) {
    case QMqttControlPacket::CONNACK: {
        qCDebug(lcMqttConnectionVerbose) << "Received CONNACK";
        if (m_internalState != BrokerWaitForConnectAck) {
            qWarning("Received CONNACK at unexpected time!");
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }

        quint8 payloadSize;
        readBuffer((char*)&payloadSize, 1);
        if (payloadSize != 2) {
            qWarning("Unexpected FRAME size in CONNACK");
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }
        m_missingData = 2;
        break;
    }
    case QMqttControlPacket::SUBACK: {
        qCDebug(lcMqttConnectionVerbose) << "Received SUBACK";
        quint8 remaining;
        readBuffer((char*)&remaining, 1);
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

        // remaining length
        quint32 multiplier = 1;
        quint32 msgLength = 0;
        quint8 b = 0;
        quint8 iteration = 0;
        do {
            readBuffer((char*)&b, 1);
            msgLength += (b & 127) * multiplier;
            multiplier *= 128;
            iteration++;
            if (iteration > 4) {
                qWarning("Publish message is too big to handle");
                closeConnection(QMqttClient::ProtocolViolation);
                return;
            }
        } while ((b & 128) != 0);
        m_missingData = msgLength;
        break;
    }
    case QMqttControlPacket::PINGRESP:
        qCDebug(lcMqttConnectionVerbose) << "Received PINGRESP";
        m_missingData = 1;
        break;


    case QMqttControlPacket::PUBREL: {
        qCDebug(lcMqttConnectionVerbose) << "Received PUBREL";
        char remaining;
        readBuffer(&remaining, 1);
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
        char remaining;
        readBuffer(&remaining, 1);
        if (remaining != 0x02) {
            qWarning("Received 2 byte message with invalid remaining length");
            closeConnection(QMqttClient::ProtocolViolation);
            return;
        }
        m_missingData = 2;
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
