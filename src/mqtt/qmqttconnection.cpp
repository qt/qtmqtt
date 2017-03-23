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

#include <QtNetwork/QTcpSocket>

QT_BEGIN_NAMESPACE

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

bool QMqttConnection::ensureTransport()
{
    if (m_transport)
        return true;

    // We are asked to create a transport layer
    if (m_client->hostname().isEmpty() || m_client->port() == 0) {
        qWarning("No hostname specified");
        return false;
    }
    auto socket = new QTcpSocket();
    m_transport = socket;
    m_ownTransport = true;
    m_transportType = QMqttClient::AbstractSocket;

    connect(socket, &QAbstractSocket::disconnected, this, &QMqttConnection::transportConnectionClosed);
    connect(m_transport, &QIODevice::aboutToClose, this, &QMqttConnection::transportConnectionClosed);
    connect(m_transport, &QIODevice::readyRead, this, &QMqttConnection::transportReadReady);
    return true;
}

bool QMqttConnection::ensureTransportOpen()
{
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
    }
    return true;
}

bool QMqttConnection::sendControlConnect()
{
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
    flags |= 1;

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

bool QMqttConnection::sendControlPublish(const QString &topic, const QByteArray &message)
{
    // ### TODO: DUP, QOS, RETAIN
    QMqttControlPacket packet(QMqttControlPacket::PUBLISH);

    packet.append(topic.toUtf8());

    // ### TODO: Add packet identifier (for QOS1/2)
    packet.append(message);

    return writePacketToTransport(packet);
}

QSharedPointer<QMqttSubscription> QMqttConnection::sendControlSubscribe(const QString &topic)
{
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
    // ### TODO: Add actual QOS
    packet.append(char(0)); // QOS

    QSharedPointer<QMqttSubscription> result(new QMqttSubscription);
    result->m_topic = topic;
    result->m_client = m_client;
    result->setState(QMqttSubscription::Pending);

    if (!writePacketToTransport(packet))
        return QSharedPointer<QMqttSubscription>();

    // SUBACK must contain identifier MQTT-3.8.4-2
    m_pendingSubscriptionAck.insert(identifier, result);
    return result;
}

bool QMqttConnection::sendControlUnsubscribe()
{
    Q_UNIMPLEMENTED();
    qDebug() << Q_FUNC_INFO;
    return false;
}

bool QMqttConnection::sendControlPingRequest()
{
    const QMqttControlPacket packet(QMqttControlPacket::PINGREQ);
    if (!writePacketToTransport(packet)) {
        qWarning("Could not write DISCONNECT frame to transport");
        return false;
    }
    return true;
}

bool QMqttConnection::sendControlDisconnect()
{
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
    qWarning("Transport Connection closed!");
    m_pingTimer.stop();
    m_client->setState(QMqttClient::Disconnected);
}

void QMqttConnection::transportReadReady()
{
    // ### TODO: This heavily relies on the fact that messages are fully sent
    // before transport ReadReady is invoked.
    qint64 available = m_transport->bytesAvailable();
    while (available > 0) {
        quint8 msg;
        m_transport->read((char*)&msg, 1);
        switch (msg) {
        case QMqttControlPacket::CONNACK: {
            if (m_internalState != BrokerWaitForConnectAck) {
                qWarning("Received CONNACK at unexpected time!");
                break;
            }
            //quint8 payloadSize = ptr[1];
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
            if (result <= 2) {
                // ### TODO: subscriptionSucceeded/Failed
                // 0 Success with QoS 0
                // 1 Success with QoS 1
                // 2 Success with QoS 2
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
        case QMqttControlPacket::PUBLISH: {
            // remaining length
            char offset;
            m_transport->read(&offset, 1); // ### TODO: Should we care about remaining length???

            // String topic
            const quint16 topicLength = qFromBigEndian<quint16>(reinterpret_cast<const quint16 *>(m_transport->read(2).constData()));
            const QString topic = QString::fromUtf8(reinterpret_cast<const char *>(m_transport->read(topicLength).constData()));

            // String message
            const quint16 messageLength = qFromBigEndian<quint16>(reinterpret_cast<const quint16 *>(m_transport->read(2).constData()));
            const QByteArray message = m_transport->read(messageLength);

            emit m_client->messageReceived(topic, message);

            // Find a subscription
            const QMap<QString, QSharedPointer<QMqttSubscription>>::const_iterator sub = m_activeSubscriptions.find(topic);
            if (sub != m_activeSubscriptions.constEnd())
                emit (*sub)->messageReceived(message);
            break;
        }
        case QMqttControlPacket::PINGRESP: {
            quint8 v;
            m_transport->read((char*)&v, 1);
            if (v != 0)
                //if (ptr[1] != 0)
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
