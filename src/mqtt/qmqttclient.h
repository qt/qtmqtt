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

#ifndef QTMQTTCLIENT_H
#define QTMQTTCLIENT_H

#include <QtMqtt/qmqttglobal.h>
#include <QtMqtt/QMqttSubscription>

#include <QObject>
#include <QtCore/QIODevice>
#include <QtCore/QSharedPointer>

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

private:
    Q_OBJECT
    Q_PROPERTY(QString clientId READ clientId WRITE setClientId NOTIFY clientIdChanged)
    Q_PROPERTY(QString hostname READ hostname WRITE setHostname NOTIFY hostnameChanged)
    Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(quint16 keepAlive READ keepAlive WRITE setKeepAlive NOTIFY keepAliveChanged)
    Q_PROPERTY(quint8 protocolVersion READ protocolVersion WRITE setProtocolVersion NOTIFY protocolVersionChanged)
    Q_PROPERTY(State state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString username READ username WRITE setUsername NOTIFY usernameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
    Q_PROPERTY(bool cleanSession READ cleanSession WRITE setCleanSession NOTIFY cleanSessionChanged)
public:
    explicit QMqttClient(QObject *parent = 0);

    void setTransport(QIODevice *device, TransportType transport);
    QIODevice *transport() const;

    QSharedPointer<QMqttSubscription> subscribe(const QString& topic, quint8 qos = 0);
    void unsubscribe(const QString& topic);

    qint32 publish(const QString &topic, const QByteArray& message = QByteArray(),
                 quint8 qos = 0, bool retain = false);
    bool requestPing();

    QString hostname() const;
    quint16 port() const;
    QString clientId() const;
    quint16 keepAlive() const;
    quint8 protocolVersion() const;

    void connectToHost();
    void connectToHostEncrypted(const QString &sslPeerName = QString());
    void disconnectFromHost();

    State state() const;

    QString username() const;
    QString password() const;
    bool cleanSession() const;

signals:
    void connected();
    void disconnected();
    void messageReceived(const QByteArray &message, const QString &topic = QString());
    void messageSent(qint32);
    void pingResponse();

    void hostnameChanged(QString hostname);
    void portChanged(quint16 port);
    void clientIdChanged(QString clientId);
    void keepAliveChanged(quint16 keepAlive);
    void protocolVersionChanged(quint8 protocolVersion);
    void stateChanged(State state);
    void usernameChanged(QString username);
    void passwordChanged(QString password);
    void cleanSessionChanged(bool cleanSession);

public slots:
    void setHostname(QString hostname);
    void setPort(quint16 port);
    void setClientId(QString clientId);
    void setKeepAlive(quint16 keepAlive);
    void setProtocolVersion(quint8 protocolVersion);
    void setState(State state);
    void setUsername(QString username);
    void setPassword(QString password);
    void setCleanSession(bool cleanSession);

private:
    void connectToHost(bool encrypted, const QString &sslPeerName);
    Q_DISABLE_COPY(QMqttClient)
    Q_DECLARE_PRIVATE(QMqttClient)
};

QT_END_NAMESPACE

#endif // QTMQTTCLIENT_H
