// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTMQTTCLIENT_WEBSOCKET_IODEVICE_BASE_H
#define QTMQTTCLIENT_WEBSOCKET_IODEVICE_BASE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifdef QT_MQTT_WITH_WEBSOCKETS

#include <QtWebSockets/QWebSocketHandshakeOptions>
#include <QtWebSockets/QWebSocket>
#include <QtCore/QIODevice>

#include "qmqttclient.h"

QT_BEGIN_NAMESPACE

class QMqttWebSocketIOBase : public QIODevice
{
    Q_OBJECT
protected:
    QMqttWebSocketIOBase(QObject *parent = nullptr, QWebSocket *webSocket = nullptr);

public:
    Q_DISABLE_COPY_MOVE(QMqttWebSocketIOBase)

    ~QMqttWebSocketIOBase() override;

    bool open(OpenMode mode) override;
    void close() override;

    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

    void setProtocol(QMqttClient::ProtocolVersion version);
    void setProtocol(const QByteArray &data);

    bool isSequential() const override
    {
        return true;
    }

    qint64 bytesAvailable() const override
    {
        return static_cast<qint64>(m_buffer.size()) + QIODevice::bytesAvailable();
    }

    bool isConnected() const
    {
        return (m_socket->state() == QAbstractSocket::ConnectedState);
    }

    QAbstractSocket::SocketState state() const
    {
        return m_socket->state();
    }

    bool isExternalSocket() const
    {
        return m_externalSocket;
    }
Q_SIGNALS:
    void disconnected();
    void errorOccurred(QAbstractSocket::SocketError error);

public slots:
    void doSocketHandleBinaryMessage(const QByteArray &msg);
    void doSocketDisconnected();
    void doSocketErrorOccured(QAbstractSocket::SocketError error);

protected:
    std::shared_ptr<QWebSocket> m_socket;
    bool m_externalSocket = false;
    QByteArray m_protocol;
    QByteArray m_buffer;
};

QT_END_NAMESPACE

#endif /* QT_MQTT_WITH_WEBSOCKETS */

#endif /* QTMQTTCLIENT_WEBSOCKET_IODEVICE_BASE_H */
