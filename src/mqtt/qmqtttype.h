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

#ifndef QMQTTTYPE_H
#define QMQTTTYPE_H

#include <QtMqtt/qmqttglobal.h>

#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QSharedDataPointer>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QMqttStringPairData;
class Q_MQTT_EXPORT QMqttStringPair
{
public:
    QMqttStringPair();
    QMqttStringPair(const QString &name, const QString &value);
    QMqttStringPair(const QMqttStringPair &);
    ~QMqttStringPair();

    QString name() const;
    void setName(const QString &n);

    QString value() const;
    void setValue(const QString &v);

    bool operator==(const QMqttStringPair &other) const;
    bool operator!=(const QMqttStringPair &other) const;
    QMqttStringPair &operator=(const QMqttStringPair &);
private:
    QSharedDataPointer<QMqttStringPairData> data;
};

#ifndef QT_NO_DEBUG_STREAM
Q_MQTT_EXPORT QDebug operator<<(QDebug d, const QMqttStringPair &s);
#endif

class Q_MQTT_EXPORT QMqttUserProperties : public QList<QMqttStringPair>
{
public:
};

QT_END_NAMESPACE

#endif // QMQTTTYPE_H
