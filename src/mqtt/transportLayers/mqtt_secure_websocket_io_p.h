// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTMQTTCLIENT_SECURE_WEBSOCKET_IODEVICE_H
#define QTMQTTCLIENT_SECURE_WEBSOCKET_IODEVICE_H

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
#include "transportLayers/mqtt_websocket_io_base_p.h"
#include "qmqttclient.h"

QT_BEGIN_NAMESPACE

class QMqttSecureWebSocketIO : public QMqttWebSocketIOBase
{
    Q_OBJECT

public:
    Q_DISABLE_COPY_MOVE(QMqttSecureWebSocketIO)

    ~QMqttSecureWebSocketIO() override;
    QMqttSecureWebSocketIO(QObject *parent = nullptr, QWebSocket *webSocket = nullptr);

    void close() override;

    void connectToHostEncrypted(const QString &hostname, quint16 port, QMqttClient::ProtocolVersion version);

Q_SIGNALS:
    void encrypted();

public slots:
    void doSocketEncrypted();
};

QT_END_NAMESPACE

#endif /* QT_MQTT_WITH_WEBSOCKETS */

#endif /* QTMQTTCLIENT_SECURE_WEBSOCKET_IODEVICE_H */
