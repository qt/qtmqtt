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
#ifndef QMQTTSUBSCRIPTION_H
#define QMQTTSUBSCRIPTION_H
#include <QtCore/QObject>
#include <QtMqtt/qmqttglobal.h>

QT_BEGIN_NAMESPACE

class Q_MQTT_EXPORT QMqttSubscription : public QObject
{
    Q_OBJECT
    Q_PROPERTY(SubscriptionState state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString topic READ topic)
public:
    ~QMqttSubscription();
    enum SubscriptionState {
        Unsubscribed = 0,
        Pending,
        Subscribed,
        Error
    };

    SubscriptionState state() const;
    QString topic() const;

signals:
    void stateChanged(SubscriptionState state);
    void messageReceived(const QString msg);

public slots:
    void setState(SubscriptionState state);
    void unsubscribe();

private:
    friend class QMqttConnection;
    friend class QMqttClient;
    explicit QMqttSubscription(QObject *parent = nullptr);
    SubscriptionState m_state;
    QString m_topic;
};

QT_END_NAMESPACE

#endif // QMQTTSUBSCRIPTION_H
