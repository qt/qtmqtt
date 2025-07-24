// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default
#include "mqtt_websocket_io_p.h"

#ifdef QT_MQTT_WITH_WEBSOCKETS

QMqttWebSocketIO::~QMqttWebSocketIO()
{
    disconnect(m_socket.get(), &QWebSocket::connected, this, &QMqttWebSocketIO::doSocketConnected);
}

QMqttWebSocketIO::QMqttWebSocketIO(QObject *parent, QWebSocket *webSocket)
    :
    QMqttWebSocketIOBase(parent, webSocket)
{
    connect(m_socket.get(), &QWebSocket::connected, this, &QMqttWebSocketIO::doSocketConnected);
}

void QMqttWebSocketIO::connectToHost(const QString &hostname, quint16 port, QMqttClient::ProtocolVersion version)
{
    setProtocol(version);
    if (!isExternalSocket()) {
        QUrl url(QString(u"ws://") + hostname + u":" + QString().setNum(port));
        QWebSocketHandshakeOptions options;
        options.setSubprotocols(QStringList{ QString::fromUtf8(m_protocol) });
        m_socket->open(url, options);
    }
    open(QIODevice::ReadWrite);
}

void QMqttWebSocketIO::close()
{
    m_socket->close();
    QMqttWebSocketIOBase::close();
}

void QMqttWebSocketIO::doSocketConnected()
{
    emit connected();
}

#include "moc_mqtt_websocket_io_p.cpp"

#endif /* QT_MQTT_WITH_WEBSOCKETS */
