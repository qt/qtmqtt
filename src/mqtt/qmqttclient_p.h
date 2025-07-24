// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QMQTTCLIENT_P_H
#define QMQTTCLIENT_P_H

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

#include "qmqttglobal_p.h"
#include "qmqttclient.h"
#include "qmqttconnection_p.h"

#include <QtNetwork/QAbstractSocket>

#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QMqttClientPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QMqttClient)
public:
    QMqttClientPrivate(QMqttClient *c);
    ~QMqttClientPrivate() override;
    void setStateAndError(QMqttClient::ClientState s, QMqttClient::ClientError e = QMqttClient::NoError);
    void setClientId(const QString &id);
    QMqttClient *m_client{nullptr};
    QString m_hostname;
    quint16 m_port{0};
    QMqttConnection m_connection;
    QString m_clientId; // auto-generated
    quint16 m_keepAlive{60};
    QMqttClient::ProtocolVersion m_protocolVersion{QMqttClient::MQTT_3_1_1};
    QMqttClient::ClientState m_state{QMqttClient::Disconnected};
    QMqttClient::ClientError m_error{QMqttClient::NoError};
    QString m_willTopic;
    QByteArray m_willMessage;
    quint8 m_willQoS{0};
    bool m_willRetain{false};
    bool m_autoKeepAlive{true};
    QString m_username;
    QString m_password;
    bool m_cleanSession{true};
    QMqttConnectionProperties m_connectionProperties;
    QMqttLastWillProperties m_lastWillProperties;
    QMqttServerConnectionProperties m_serverConnectionProperties;
};

QT_END_NAMESPACE
#endif // QMQTTCLIENT_P_H
