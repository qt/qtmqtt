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
    ~QMqttClientPrivate();
    QString m_hostname;
    quint16 m_port{0};
    QMqttConnection m_connection;
    QString m_clientId; // auto-generated
    quint16 m_keepAlive{60};
    // 3 == MQTT Standard 3.1
    // 4 == MQTT Standard 3.1.1
    quint8 m_protocolVersion{3};
    QMqttClient::State m_state{QMqttClient::Disconnected};
    QString m_username;
    QString m_password;
};

QT_END_NAMESPACE
#endif // QMQTTCLIENT_P_H
