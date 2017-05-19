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
#include "qmqttmessage.h"
#include "qmqttsubscription.h"
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtCore/QtEndian>
#include <QtCore/QBuffer>

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
    ~QMqttConnection() override;

    void setTransport(QIODevice *device, QMqttClient::TransportType transport);
    QIODevice *transport() const;

    bool ensureTransport(bool createSecureIfNeeded = false);
    bool ensureTransportOpen(const QString &sslPeerName = QString());

    bool sendControlConnect();
    qint32 sendControlPublish(const QString &topic, const QByteArray &message, quint8 qos = 0, bool retain = false);
    bool sendControlPublishAcknowledge(quint16 id);
    bool sendControlPublishRelease(quint16 id);
    bool sendControlPublishReceive(quint16 id);
    bool sendControlPublishComp(quint16 id);
    QSharedPointer<QMqttSubscription> sendControlSubscribe(const QString &topic, quint8 qos = 0);
    bool sendControlUnsubscribe(const QString &topic);
    bool sendControlPingRequest();
    bool sendControlDisconnect();

    void setClient(QMqttClient *client);

    inline InternalConnectionState internalState() const { return m_internalState; }

public Q_SLOTS:
    void transportConnectionClosed();
    void transportReadReady();

public:
    QIODevice *m_transport{nullptr};
    QMqttClient::TransportType m_transportType{QMqttClient::IODevice};
    bool m_ownTransport{false};
    QMqttClient *m_client{nullptr};
private:
    Q_DISABLE_COPY(QMqttConnection)
    void someFuncToBeRemoved();
    void finalize_connack();
    void finalize_suback();
    void finalize_unsuback();
    void finalize_publish();
    void finalize_pubAckRecComp();
    void finalize_pubrel();
    void finalize_pingresp();
    void processData();
    void readBuffer(char *data, qint64 size);
    QByteArray readBuffer(qint64 size);
    QByteArray m_readBuffer;
    qint64 m_missingData{0};
    struct PublishData {
        quint8 qos;
        bool dup;
        bool retain;
    };
    PublishData m_currentPublish;
    QMqttControlPacket::PacketType m_currentPacket{QMqttControlPacket::UNKNOWN};

    bool writePacketToTransport(const QMqttControlPacket &p);
    QMap<quint16, QSharedPointer<QMqttSubscription>> m_pendingSubscriptionAck;
    QMap<quint16, QSharedPointer<QMqttSubscription>> m_pendingUnsubscriptions;
    QMap<QString, QSharedPointer<QMqttSubscription>> m_activeSubscriptions;
    QMap<quint16, QSharedPointer<QMqttControlPacket>> m_pendingMessages;
    QMap<quint16, QSharedPointer<QMqttControlPacket>> m_pendingReleaseMessages;
    InternalConnectionState m_internalState{BrokerDisconnected};
    QTimer m_pingTimer;
};

QT_END_NAMESPACE

#endif // QMQTTCONNECTION_P_H
