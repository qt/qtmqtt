// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "websocketiodevice.h"

#include <QtCore/QDebug>

#include <QtWebSockets/qwebsockethandshakeoptions.h>

WebSocketIODevice::WebSocketIODevice(QObject *parent)
    : QIODevice(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &WebSocketIODevice::onSocketConnected);
    connect(&m_socket, &QWebSocket::binaryMessageReceived, this, &WebSocketIODevice::handleBinaryMessage);
}

bool WebSocketIODevice::isSequential() const
{
    return true;
}

qint64 WebSocketIODevice::bytesAvailable() const
{
    return static_cast<qint64>(m_buffer.size()) + QIODevice::bytesAvailable();
}

bool WebSocketIODevice::open(QIODevice::OpenMode mode)
{
    QWebSocketHandshakeOptions options;
    options.setSubprotocols(QStringList{ QString::fromUtf8(m_protocol) });

    m_socket.open(m_url, options);

    return QIODevice::open(mode);
}

void WebSocketIODevice::close()
{
    m_socket.close();
    QIODevice::close();
}

qint64 WebSocketIODevice::readData(char *data, qint64 maxlen)
{
    qint64 bytesToRead = qMin(maxlen, (qint64)m_buffer.size());
    memcpy(data, m_buffer.constData(), static_cast<size_t>(bytesToRead));
    m_buffer = m_buffer.right(m_buffer.size() - bytesToRead);
    return bytesToRead;
}

qint64 WebSocketIODevice::writeData(const char *data, qint64 len)
{
    QByteArray msg(data, len);
    const int length = m_socket.sendBinaryMessage(msg);
    return length;
}

void WebSocketIODevice::setUrl(const QUrl &url)
{
    m_url = url;
}

void WebSocketIODevice::setProtocol(const QByteArray &data)
{
    m_protocol = data;
}

void WebSocketIODevice::handleBinaryMessage(const QByteArray &msg)
{
    m_buffer.append(msg);
    emit readyRead();
}

void WebSocketIODevice::onSocketConnected()
{
    emit socketConnected();
}
