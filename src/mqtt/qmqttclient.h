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

#ifndef QTMQTTCLIENT_H
#define QTMQTTCLIENT_H

#include <QtMqtt/qmqttglobal.h>
#include <QtMqtt/QMqttSubscription>

#include <QObject>
#include <QtCore/QIODevice>
#include <QtCore/QSharedPointer>
#include <QtNetwork/QTcpSocket>

QT_BEGIN_NAMESPACE

class QMqttClientPrivate;

class Q_MQTT_EXPORT QMqttClient : public QObject
{
public:
    enum TransportType {
        IODevice = 0,
        AbstractSocket,
        SecureSocket
    };
    enum State {
        Disconnected = 0,
        Connecting,
        Connected
    };
    enum ProtocolVersion {
        MQTT_3_1 = 3,
        MQTT_3_1_1 = 4
    };

private:
    Q_OBJECT
    Q_ENUMS(State)
    Q_PROPERTY(QString clientId READ clientId WRITE setClientId NOTIFY clientIdChanged)
    Q_PROPERTY(QString hostname READ hostname WRITE setHostname NOTIFY hostnameChanged)
    Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(quint16 keepAlive READ keepAlive WRITE setKeepAlive NOTIFY keepAliveChanged)
    Q_PROPERTY(ProtocolVersion protocolVersion READ protocolVersion WRITE setProtocolVersion NOTIFY protocolVersionChanged)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool cleanSession READ cleanSession WRITE setCleanSession NOTIFY cleanSessionChanged)
    Q_PROPERTY(QString willTopic READ willTopic WRITE setWillTopic NOTIFY willTopicChanged)
    Q_PROPERTY(QByteArray willMessage READ willMessage WRITE setWillMessage NOTIFY willMessageChanged)
    Q_PROPERTY(quint8 willQoS READ willQoS WRITE setWillQoS NOTIFY willQoSChanged)
    Q_PROPERTY(bool willRetain READ willRetain WRITE setWillRetain NOTIFY willRetainChanged)
public:
    explicit QMqttClient(QObject *parent = nullptr);

    void setTransport(QIODevice *device, TransportType transport);
    QIODevice *transport() const;

    QSharedPointer<QMqttSubscription> subscribe(const QString& topic, quint8 qos = 0);
    void unsubscribe(const QString& topic);

    Q_INVOKABLE qint32 publish(const QString &topic, const QByteArray& message = QByteArray(),
                 quint8 qos = 0, bool retain = false);
    bool requestPing();

    QString hostname() const;
    quint16 port() const;
    QString clientId() const;
    quint16 keepAlive() const;
    ProtocolVersion protocolVersion() const;

    Q_INVOKABLE void connectToHost();
#ifndef QT_NO_SSL
    Q_INVOKABLE void connectToHostEncrypted(const QString &sslPeerName = QString());
#endif
    Q_INVOKABLE void disconnectFromHost();

    State state() const;

    QString username() const;
    QString password() const;
    bool cleanSession() const;

    QString willTopic() const;
    quint8 willQoS() const;
    QByteArray willMessage() const;
    bool willRetain() const;

Q_SIGNALS:
    void connected();
    void disconnected();
    void messageReceived(const QByteArray &message, const QString &topic = QString());
    void messageSent(qint32 id);
    void pingResponse();
    void brokerSessionRestored();

    void hostnameChanged(QString hostname);
    void portChanged(quint16 port);
    void clientIdChanged(QString clientId);
    void keepAliveChanged(quint16 keepAlive);
    void protocolVersionChanged(ProtocolVersion protocolVersion);
    void stateChanged(State state);
    void usernameChanged(QString username);
    void passwordChanged(QString password);
    void cleanSessionChanged(bool cleanSession);

    void willTopicChanged(QString willTopic);
    void willQoSChanged(quint8 willQoS);
    void willMessageChanged(QByteArray willMessage);
    void willRetainChanged(bool willRetain);

public Q_SLOTS:
    void setHostname(const QString &hostname);
    void setPort(quint16 port);
    void setClientId(const QString &clientId);
    void setKeepAlive(quint16 keepAlive);
    void setProtocolVersion(ProtocolVersion protocolVersion);
    void setState(State state);
    void setUsername(const QString &username);
    void setPassword(const QString &password);
    void setCleanSession(bool cleanSession);

    void setWillTopic(const QString &willTopic);
    void setWillQoS(quint8 willQoS);
    void setWillMessage(const QByteArray &willMessage);
    void setWillRetain(bool willRetain);

private:
    void connectToHost(bool encrypted, const QString &sslPeerName);
    Q_DISABLE_COPY(QMqttClient)
    Q_DECLARE_PRIVATE(QMqttClient)
};

QT_END_NAMESPACE

#endif // QTMQTTCLIENT_H
