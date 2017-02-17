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

#ifndef QMQTTCONNECTION_P_H
#define QMQTTCONNECTION_P_H

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
#include "qmqttcontrolpacket_p.h"
#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QTimer>
#include <QtCore/QtEndian>

QT_BEGIN_NAMESPACE

class Q_AUTOTEST_EXPORT QMqttConnection : public QObject
{
    Q_OBJECT
public:
    enum InternalConnectionState {
        BrokerDisconnected = 0,
        BrokerWaitForConnectAck,
        BrokerConnected
    };

    enum ConnectionError {
        NoError                = 0,
        InvalidProtocolVersion = 1,
        IdRejected             = 2,
        ServerUnavailable      = 3,
        BadUsernameOrPassword  = 4,
        NotAuthorized          = 5
    };
    explicit QMqttConnection(QObject *parent = 0);
    ~QMqttConnection();

    void setTransport(QIODevice *device, QMqttClient::TransportType transport);
    QIODevice *transport() const;

    bool ensureTransport();
    bool ensureTransportOpen();

    bool sendControlConnect();
    bool sendControlPublish(const QString &topic, const QString &message);
    bool sendControlSubscribe(const QString &topic);
    bool sendControlUnsubscribe();
    bool sendControlPingRequest();
    bool sendControlDisconnect();

    void setClient(QMqttClient *client);

    inline InternalConnectionState internalState() const { return m_internalState; }
signals:

public slots:
    void transportConnectionClosed();
    void transportReadReady();

public:
    QIODevice *m_transport{nullptr};
    QMqttClient::TransportType m_transportType{QMqttClient::IODevice};
    bool m_ownTransport{false};
    QMqttClient *m_client{nullptr};
private:
    bool writePacketToTransport(const QMqttControlPacket &p);
    QSet<quint16> m_pendingSubscriptionAck;
    InternalConnectionState m_internalState{BrokerDisconnected};
    QTimer m_pingTimer;
};

QT_END_NAMESPACE

#endif // QMQTTCONNECTION_P_H
