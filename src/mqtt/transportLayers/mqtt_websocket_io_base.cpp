// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default
#include "mqtt_websocket_io_base_p.h"

#ifdef QT_MQTT_WITH_WEBSOCKETS

QMqttWebSocketIOBase::~QMqttWebSocketIOBase()
{
    disconnect(m_socket.get(), &QWebSocket::disconnected, this, &QMqttWebSocketIOBase::doSocketDisconnected);
    disconnect(m_socket.get(), &QWebSocket::errorOccurred, this, &QMqttWebSocketIOBase::doSocketErrorOccured);
    disconnect(m_socket.get(), &QWebSocket::binaryMessageReceived, this, &QMqttWebSocketIOBase::doSocketHandleBinaryMessage);
}

QMqttWebSocketIOBase::QMqttWebSocketIOBase(QObject *parent, QWebSocket *webSocket)
    :
    QIODevice(parent)
{
    m_externalSocket = (webSocket != nullptr);

    if (webSocket)
        m_socket = std::shared_ptr<QWebSocket>(webSocket, [](QWebSocket *) { ; }); // skip deleter
    else
        m_socket = std::make_shared<QWebSocket>();

    connect(m_socket.get(), &QWebSocket::disconnected, this, &QMqttWebSocketIOBase::doSocketDisconnected);
    connect(m_socket.get(), &QWebSocket::errorOccurred, this, &QMqttWebSocketIOBase::doSocketErrorOccured);
    connect(m_socket.get(), &QWebSocket::binaryMessageReceived, this, &QMqttWebSocketIOBase::doSocketHandleBinaryMessage);

    setProtocol(QMqttClient::ProtocolVersion::MQTT_3_1);
}

bool QMqttWebSocketIOBase::open(QIODevice::OpenMode mode)
{
    if (!QIODevice::open(mode))
    {
        close();
        emit errorOccurred(QAbstractSocket::SocketResourceError);
    }
    // Will emit onConnectionEstablished or onConnectionError
    return false;
}

void QMqttWebSocketIOBase::close()
{
    QIODevice::close();
}

qint64 QMqttWebSocketIOBase::readData(char *data, qint64 maxlen)
{
    qint64 bytesToRead = qMin(maxlen, (qint64)m_buffer.size());
    memcpy(data, m_buffer.constData(), bytesToRead);
    m_buffer = m_buffer.right(m_buffer.size() - bytesToRead);
    return bytesToRead;
}

qint64 QMqttWebSocketIOBase::writeData(const char *data, qint64 len)
{
    QByteArray msg(data, len);
    const int length = m_socket->sendBinaryMessage(msg);
    emit bytesWritten(length);
    return length;
}

void QMqttWebSocketIOBase::setProtocol(QMqttClient::ProtocolVersion version)
{
    switch (version) {
    case QMqttClient::ProtocolVersion::MQTT_3_1:
        setProtocol("mqttv3.1");
        break;
    case QMqttClient::ProtocolVersion::MQTT_3_1_1:
        setProtocol("mqtt");
        break;
    case QMqttClient::ProtocolVersion::MQTT_5_0:
        setProtocol("mqtt");
        break;
    } /* end-switch */
}

void QMqttWebSocketIOBase::setProtocol(const QByteArray &data)
{
    m_protocol = data;
}

void QMqttWebSocketIOBase::doSocketHandleBinaryMessage(const QByteArray &msg)
{
    m_buffer.append(msg);
    emit readyRead();
}

void QMqttWebSocketIOBase::doSocketDisconnected()
{
    emit disconnected();
}

void QMqttWebSocketIOBase::doSocketErrorOccured(QAbstractSocket::SocketError error)
{
    emit errorOccurred(error);
}

#include "moc_mqtt_websocket_io_base_p.cpp"

#endif /* QT_MQTT_WITH_WEBSOCKETS */
