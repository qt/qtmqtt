/******************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtMqtt module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/

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

#include "qmqttclient.h"

#include <QtNetwork/QAbstractSocket>

#include <private/qmqttconnection_p.h>
#include <private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QMqttClientPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QMqttClient)
public:
    QMqttClientPrivate();
    ~QMqttClientPrivate() override;
    QString m_hostname;
    quint16 m_port{0};
    QMqttConnection m_connection;
    QString m_clientId; // auto-generated
    quint16 m_keepAlive{60};
    // 3 == MQTT Standard 3.1
    // 4 == MQTT Standard 3.1.1
    QMqttClient::ProtocolVersion m_protocolVersion{QMqttClient::MQTT_3_1_1};
    QMqttClient::State m_state{QMqttClient::Disconnected};
    QString m_willTopic;
    QByteArray m_willMessage;
    quint8 m_willQoS{0};
    bool m_willRetain{false};
    QString m_username;
    QString m_password;
    bool m_cleanSession{true};
};

QT_END_NAMESPACE
#endif // QMQTTCLIENT_P_H
