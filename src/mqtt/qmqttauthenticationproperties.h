// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QMQTTAUTHENTICATIONPROPERTIES_H
#define QMQTTAUTHENTICATIONPROPERTIES_H

#include <QtMqtt/qmqttglobal.h>
#include <QtMqtt/qmqtttype.h>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QMqttAuthenticationPropertiesData;

class Q_MQTT_EXPORT QMqttAuthenticationProperties
{
public:
    QMqttAuthenticationProperties();
    QMqttAuthenticationProperties(const QMqttAuthenticationProperties &);
    QMqttAuthenticationProperties &operator=(const QMqttAuthenticationProperties &);
    ~QMqttAuthenticationProperties();

    QString authenticationMethod() const;
    void setAuthenticationMethod(const QString &method);

    QByteArray authenticationData() const;
    void setAuthenticationData(const QByteArray &adata);

    QString reason() const;
    void setReason(const QString &r);

    QMqttUserProperties userProperties() const;
    void setUserProperties(const QMqttUserProperties &user);

private:
    QSharedDataPointer<QMqttAuthenticationPropertiesData> data;
};

QT_END_NAMESPACE

#endif // QMQTTAUTHENTICATIONPROPERTIES_H
