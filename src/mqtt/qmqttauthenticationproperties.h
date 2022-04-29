/******************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
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
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
******************************************************************************/

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
