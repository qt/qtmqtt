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
#include "qmqttconnection_p.h"
#include "qmqttcontrolpacket_p.h"

#include <QtCore/QLoggingCategory>
#include <QtNetwork/QSslSocket>
#include <QtNetwork/QTcpSocket>

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
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << m_transport;

    if (m_transport)
        return true;

    // We are asked to create a transport layer
    if (m_client->hostname().isEmpty() || m_client->port() == 0) {
        qWarning("Trying to create a transport layer, but no hostname is specified");
        return false;
    }
    auto socket = createSecureIfNeeded ? new QSslSocket() : new QTcpSocket();
    m_transport = socket;
    m_ownTransport = true;
    m_transportType = createSecureIfNeeded ? QMqttClient::SecureSocket : QMqttClient::AbstractSocket;

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
            return true;

        if (m_transport->open(QIODevice::ReadWrite)) {
            qWarning("Could not open Transport IO device");
            return false;
        }
    } else if (m_transportType == QMqttClient::AbstractSocket) {
        auto socket = dynamic_cast<QTcpSocket*>(m_transport);
        Q_ASSERT(socket);
        if (socket->state() == QAbstractSocket::ConnectedState)
            return true;

        socket->connectToHost(m_client->hostname(), m_client->port());
        if (!socket->waitForConnected()) {
            qWarning("Could not establish socket connection for transport");
            return false;
        }
    } else if (m_transportType == QMqttClient::SecureSocket) {
        auto socket = dynamic_cast<QSslSocket*>(m_transport);
        Q_ASSERT(socket);
        if (socket->state() == QAbstractSocket::ConnectedState)
            return true;

        socket->connectToHostEncrypted(m_client->hostname(), m_client->port(), sslPeerName);
        if (!socket->waitForConnected()) {
            qWarning("Could not establish socket connection for transport");
            return false;
        }

        if (!socket->waitForEncrypted()) {
            qWarning("Could not initiate encryption.");
            return false;
        }
    }

    return true;
}

bool QMqttConnection::sendControlConnect()
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO;

    QMqttControlPacket packet(QMqttControlPacket::CONNECT);

    // Variable header
    // 3.1.2.1 Protocol Name
    // 3.1.2.2 Protocol Level
    const quint8 protocolVersion = m_client->protocolVersion();
    if (protocolVersion == 3) {
        packet.append("MQIsdp");
        packet.append(char(3)); // Version 3.1
    } else if (protocolVersion == 4) {
        packet.append("MQTT");
        packet.append(char(4)); // Version 3.1.1
    } else {
        qFatal("Illegal MQTT VERSION");
    }

    // 3.1.2.3 Connect Flags
    quint8 flags = 0;
    // Clean session
    if (m_client->cleanSession())
        flags |= 1;

    if (!m_client->willMessage().isEmpty()) {
        flags |= 1 << 2;
        if (m_client->willQoS() > 2) {
            qWarning("Will QoS does not have a valid value");
            return false;
        }
        if (m_client->willQoS() == 1)
            flags |= 1 << 3;
        else if (m_client->willQoS() == 2)
            flags |= 1 << 4;
        if (m_client->willRetain())
            flags |= 1 << 5;
    }
    if (m_client->username().size())
        flags |= 1 << 7;

    if (m_client->password().size())
        flags |= 1 << 6;

    packet.append(char(flags));

    // 3.1.2.10 Keep Alive
    packet.append(m_client->keepAlive());

    // 3.1.3 Payload
    // 3.1.3.1 Client Identifier
    // Client id maximum left is 23
    const QByteArray clientStringArray = m_client->clientId().left(23).toUtf8();
    if (clientStringArray.size()) {
        packet.append(clientStringArray);
    } else {
        packet.append(char(0));
        packet.append(char(0));
    }

    if (!m_client->willMessage().isEmpty()) {
        packet.append(m_client->willTopic().toUtf8());
        packet.append(m_client->willMessage().toUtf8());
    }

    if (m_client->username().size())
        packet.append(m_client->username().toUtf8());

    if (m_client->password().size())
        packet.append(m_client->password().toUtf8());

    if (!writePacketToTransport(packet)) {
        qWarning("Could not write CONNECT frame to transport");
        return false;
    }

    m_internalState = BrokerWaitForConnectAck;
    return true;
}

qint32 QMqttConnection::sendControlPublish(const QString &topic, const QByteArray &message, quint8 qos, bool retain)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << topic << " Size:" << message.size() << " bytes."
                              << "QoS:" << qos << " Retain:" << retain;

    if (topic.contains(QLatin1Char('#')) || topic.contains('+'))
        return -1;

    // ### TODO: DUP, QOS, RETAIN
    quint8 header = QMqttControlPacket::PUBLISH;
    if (qos == 1)
        header |= 0x02;
    else if (qos == 2)
        header |= 0x04;

    if (retain)
        header |= 0x01;

    QSharedPointer<QMqttControlPacket> packet(new QMqttControlPacket(header));

    packet->append(topic.toUtf8());
    quint16 identifier = 0;
    if (qos > 0) {
        // Add Packet Identifier
        static quint16 publishIdCounter = 0;
        if (publishIdCounter + 1 == UINT16_MAX)
            publishIdCounter = 0;
        else
            publishIdCounter++;

        identifier = publishIdCounter;
        packet->append(identifier);
    }
    packet->append(message);

    if (qos)
        m_pendingMessages.insert(identifier, packet);

    const bool written = writePacketToTransport(*packet.data());

    if (!written)
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
    QMqttControlPacket packet(QMqttControlPacket::PUBREL);
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

QSharedPointer<QMqttSubscription> QMqttConnection::sendControlSubscribe(const QString &topic, quint8 qos)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << " Topic:" << topic << " qos:" << qos;

    if (m_activeSubscriptions.contains(topic))
        return m_activeSubscriptions[topic];

    // has to have 0010 as bits 3-0, maybe update SUBSCRIBE instead?
    // MQTT-3.8.1-1
    const quint8 header = QMqttControlPacket::SUBSCRIBE + 0x02;
    QMqttControlPacket packet(header);

    // Add Packet Identifier
    const quint16 identifier = qrand();
    packet.append(identifier);

    packet.append(topic.toUtf8());

    switch (qos) {
    case 0: packet.append(char(0x0)); break;
    case 1: packet.append(char(0x1)); break;
    case 2: packet.append(char(0x2)); break;
    default: return QSharedPointer<QMqttSubscription>();
    }

    QSharedPointer<QMqttSubscription> result(new QMqttSubscription);
    result->m_topic = topic;
    result->m_client = m_client;
    result->m_qos = qos;
    result->setState(QMqttSubscription::SubscriptionPending);

    if (!writePacketToTransport(packet))
        return QSharedPointer<QMqttSubscription>();

    // SUBACK must contain identifier MQTT-3.8.4-2
    m_pendingSubscriptionAck.insert(identifier, result);
    return result;
}

bool QMqttConnection::sendControlUnsubscribe(const QString &topic)
{
    qCDebug(lcMqttConnection) << Q_FUNC_INFO << " Topic:" << topic;

    // MQTT-3.10.3-2
    if (topic.isEmpty())
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
    const quint16 identifier = qrand();
    packet.append(identifier);

    packet.append(topic.toUtf8());
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

    for (auto sub : m_activeSubscriptions)
        sub->unsubscribe();
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

void QMqttConnection::setClient(QMqttClient *client)
{
    m_client = client;
}

void QMqttConnection::transportConnectionClosed()
{
    m_pingTimer.stop();
    m_client->setState(QMqttClient::Disconnected);
}

void QMqttConnection::transportReadReady()
{
    qCDebug(lcMqttConnectionVerbose) << Q_FUNC_INFO;
    // ### TODO: This heavily relies on the fact that messages are fully sent
    // before transport ReadReady is invoked.
    qint64 available = m_transport->bytesAvailable();
    while (available > 0) {
        quint8 msg;
        m_transport->read((char*)&msg, 1);
        switch (msg & 0xF0) {
        case QMqttControlPacket::CONNACK: {
            qCDebug(lcMqttConnectionVerbose) << "Received CONNACK";
            if (m_internalState != BrokerWaitForConnectAck) {
                qWarning("Received CONNACK at unexpected time!");
                break;
            }

            quint8 payloadSize;
            m_transport->read((char*)&payloadSize, 1);
            if (payloadSize != 2) {
                qWarning("Unexpected FRAME size in CONNACK");
                // ## SET SOME ERROR
                break;
            }
            quint8 ackFlags;
            m_transport->read((char*)&ackFlags, 1);
            if (ackFlags > 1) { // MQTT-3.2.2.1
                qWarning("Unexpected CONNACK Flags set");
                // ## SET SOME ERROR
                break;
            }
            bool sessionPresent = ackFlags == 1;

            // ### TODO: MQTT-3.2.2-1
            // ### TODO: MQTT-3.2.2-2
            if (sessionPresent) {
                qWarning("Connected with a clean session, ack contains session present");
                // ## SET SOME ERROR
                // ### TODO: RABBIT MQ Spec Misalign
                // If a clean session is requested by the client, the server has to have an empty
                // session. However Rabbit MQ sends a 1 here for unknown reasons.
                //break;
            }

            quint8 connectResultValue;
            m_transport->read((char*)&connectResultValue, 1);
            if (connectResultValue != 0) {
                qWarning("Connection has been rejected");
                // MQTT-3.2.2-5
                // ConnectionError
                m_transport->close();
            }
            m_internalState = BrokerConnected;
            m_client->setState(QMqttClient::Connected);

            m_pingTimer.setInterval(m_client->keepAlive() * 1000);
            m_pingTimer.start();
            break;
        }
        case QMqttControlPacket::SUBACK: {
            char offset;
            m_transport->read(&offset, 1);
            quint16 id;
            m_transport->read((char*)&id, 2);
            id = qFromBigEndian<quint16>(id);
            if (!m_pendingSubscriptionAck.contains(id)) {
                qWarning("Received SUBACK for unknown subscription request");
                break;
            }
            quint8 result;
            m_transport->read((char*)&result, 1);
            auto sub = m_pendingSubscriptionAck.take(id);
            qCDebug(lcMqttConnectionVerbose) << "Received SUBACK: ID:" << id << "QoS:" << result;
            if (result <= 2) {
                // The broker might have a different support level for QoS than what
                // the client requested
                if (result != sub->m_qos) {
                    sub->m_qos = result;
                    emit sub->qosChanged(result);
                }
                sub->setState(QMqttSubscription::Subscribed);
                m_activeSubscriptions.insert(sub->topic(), sub);
            } else if (result == 0x80) {
                // ### TODO: subscriptionFailed
                qWarning("Subscription failed");
                sub->setState(QMqttSubscription::Error);
            } else {
                qWarning("Received invalid SUBACK result value");
                sub->setState(QMqttSubscription::Error);
            }
            break;
        }
        case QMqttControlPacket::UNSUBACK: {
            char offset;
            m_transport->read(&offset, 1);
            quint16 id;
            m_transport->read((char*)&id, 2);
            id = qFromBigEndian<quint16>(id);
            qCDebug(lcMqttConnectionVerbose) << "Received UNSUBACK: ID:" << id;
            if (!m_pendingUnsubscriptions.contains(id)) {
                qWarning("Received UNSUBACK for unknown request");
                break;
            }
            auto sub = m_pendingUnsubscriptions.take(id);
            sub->setState(QMqttSubscription::Unsubscribed);
            m_activeSubscriptions.remove(sub->topic());

            break;
        }
        case QMqttControlPacket::PUBLISH: {
            quint8 qos = (msg & 0x06) >> 1;
            bool retain = msg & 0x01;
            Q_UNUSED(retain);
            // remaining length
            char offset;
            m_transport->read(&offset, 1); // ### TODO: Should we care about remaining length???

            // String topic
            const quint16 topicLength = qFromBigEndian<quint16>(reinterpret_cast<const quint16 *>(m_transport->read(2).constData()));
            const QString topic = QString::fromUtf8(reinterpret_cast<const char *>(m_transport->read(topicLength).constData()));

            quint16 id;
            if (qos > 0) {
                id = qFromBigEndian<quint16>(reinterpret_cast<const quint16 *>(m_transport->read(2).constData()));
            }
            // String message
            const quint16 messageLength = qFromBigEndian<quint16>(reinterpret_cast<const quint16 *>(m_transport->read(2).constData()));
            const QByteArray message = m_transport->read(messageLength);

            qCDebug(lcMqttConnectionVerbose) << "Received PUBLISH: topic:" << topic
                                             << " messageLength:" << messageLength;;

            emit m_client->messageReceived(message, topic);

            for (auto sub = m_activeSubscriptions.constBegin(); sub != m_activeSubscriptions.constEnd(); sub++) {
                const QString subTopic = sub.key();

                if (subTopic == topic) {
                    emit sub.value()->messageReceived(message, topic);
                    continue;
                } else if (subTopic.endsWith(QLatin1Char('#')) && topic.startsWith(subTopic.leftRef(subTopic.size() - 1))) {
                    emit sub.value()->messageReceived(message, topic);
                    continue;
                }

                if (!subTopic.contains(QLatin1Char('+')))
                    continue;

                const QVector<QStringRef> subTopicSplit = subTopic.splitRef(QLatin1Char('/'));
                const QVector<QStringRef> topicSplit = topic.splitRef(QLatin1Char('/'));
                if (subTopicSplit.size() != topicSplit.size())
                    continue;
                const QVector<QStringRef> subPlusSplit = subTopic.splitRef(QLatin1Char('+'));

                if (topic.startsWith(subPlusSplit.at(0)) && topic.endsWith(subPlusSplit.at(1))) {
                    emit sub.value()->messageReceived(message, topic);
                }
            }

            if (qos == 1)
                sendControlPublishAcknowledge(id);
            else if (qos == 2)
                sendControlPublishReceive(id);
            break;
        }
        case QMqttControlPacket::PUBACK:
        case QMqttControlPacket::PUBREC:
        case QMqttControlPacket::PUBCOMP: {
            // remaining length
            char offset;
            m_transport->read(&offset, 1); // ### TODO: Should we care about remaining length???
            quint16 id;
            m_transport->read((char*)&id, 2);
            id = qFromBigEndian<quint16>(id);

            if ((msg & 0xF0) == QMqttControlPacket::PUBCOMP) {
                qCDebug(lcMqttConnectionVerbose) << "Received PUBCOMP:" << id;
                auto pendingRelease = m_pendingReleaseMessages.take(id);
                if (!pendingRelease)
                    qWarning("Received PUBCOMP for unknown released message");
                emit m_client->messageSent(id);
                break;
            }

            auto pendingMsg = m_pendingMessages.take(id);
            if (!pendingMsg) {
                qWarning(qPrintable(QString::fromLatin1("Received PUBACK for unknown message: %1").arg(id)));
                break;
            }
            if ((msg & 0xF0) == QMqttControlPacket::PUBREC) {
                qCDebug(lcMqttConnectionVerbose) << "Received PUBREC:" << id;
                m_pendingReleaseMessages.insert(id, pendingMsg);
                sendControlPublishRelease(id);
            } else {
                qCDebug(lcMqttConnectionVerbose) << "Received PUBACK:" << id;
                emit m_client->messageSent(id);
            }
            break;
        }
        case QMqttControlPacket::PUBREL: {
            // remaining length
            char offset;
            m_transport->read(&offset, 1); // ### TODO: Should we care about remaining length???
            quint16 id;
            m_transport->read((char*)&id, 2);
            id = qFromBigEndian<quint16>(id);

            qCDebug(lcMqttConnectionVerbose) << "Received PUBREL:" << id;

            // ### TODO: send to our app now or not???
            // See standard Figure 4.3 Method A or B ???
            sendControlPublishComp(id);
            break;
        }
        case QMqttControlPacket::PINGRESP: {
            qCDebug(lcMqttConnectionVerbose) << "Received PINGRESP:";
            quint8 v;
            m_transport->read((char*)&v, 1);
            if (v != 0)
                qWarning("Received a PINGRESP with payload!");
            emit m_client->pingResponse();
            break;
        }
        default: {
            qWarning("Transport received unknown data");
        }
        }
        available = m_transport->bytesAvailable();
    } // available
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
