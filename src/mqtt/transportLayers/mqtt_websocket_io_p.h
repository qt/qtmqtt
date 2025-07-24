// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTMQTTCLIENT_WEBSOCKET_IODEVICE_H
#define QTMQTTCLIENT_WEBSOCKET_IODEVICE_H

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

class QMqttWebSocketIO : public QMqttWebSocketIOBase
{
    Q_OBJECT
public:
    Q_DISABLE_COPY_MOVE(QMqttWebSocketIO)

    ~QMqttWebSocketIO() override;
    QMqttWebSocketIO(QObject *parent = nullptr, QWebSocket *webSocket = nullptr);

    void close() override;

    void connectToHost(const QString &hostname, quint16 port, QMqttClient::ProtocolVersion version);

Q_SIGNALS:
    void connected();

public slots:
    void doSocketConnected();
};

QT_END_NAMESPACE

#endif /* QT_MQTT_WITH_WEBSOCKETS */

#endif /* QTMQTTCLIENT_WEBSOCKET_IODEVICE_H */
