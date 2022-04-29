/******************************************************************************
**
** Copyright (C) 2017 Lorenz Haas
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

#ifndef QMQTTTOPICFILTER_H
#define QMQTTTOPICFILTER_H

#include <QtMqtt/qmqttglobal.h>
#include <QtMqtt/qmqtttopicname.h>

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QMetaType>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QMqttTopicFilterPrivate;

class QMqttTopicFilter;
// qHash is a friend, but we can't use default arguments for friends (§8.3.6.4)
Q_MQTT_EXPORT size_t qHash(const QMqttTopicFilter &name, size_t seed = 0) Q_DECL_NOTHROW;

class Q_MQTT_EXPORT QMqttTopicFilter
{
public:
    enum MatchOption {
        NoMatchOption                            = 0x0000,
        WildcardsDontMatchDollarTopicMatchOption = 0x0001
    };
    Q_DECLARE_FLAGS(MatchOptions, MatchOption)

    QMqttTopicFilter(const QString &filter = QString());
    QMqttTopicFilter(const QLatin1String &filter);
    QMqttTopicFilter(const QMqttTopicFilter &filter);
    ~QMqttTopicFilter();
    QMqttTopicFilter &operator=(const QMqttTopicFilter &filter);

#ifdef Q_COMPILER_RVALUE_REFS
    QMqttTopicFilter &operator=(QMqttTopicFilter &&other) noexcept { swap(other); return *this; }
#endif

    void swap(QMqttTopicFilter &other) noexcept { d.swap(other.d); }

    QString filter() const;
    void setFilter(const QString &filter);

    QString sharedSubscriptionName() const;

    Q_REQUIRED_RESULT bool isValid() const;
    Q_REQUIRED_RESULT bool match(const QMqttTopicName &name, MatchOptions matchOptions = NoMatchOption) const;

    friend Q_MQTT_EXPORT bool operator==(const QMqttTopicFilter &lhs, const QMqttTopicFilter &rhs) Q_DECL_NOTHROW;
    friend inline bool operator!=(const QMqttTopicFilter &lhs, const QMqttTopicFilter &rhs) Q_DECL_NOTHROW { return !(lhs == rhs); }
    friend Q_MQTT_EXPORT bool operator<(const QMqttTopicFilter &lhs, const QMqttTopicFilter &rhs) Q_DECL_NOTHROW;
    friend Q_MQTT_EXPORT size_t qHash(const QMqttTopicFilter &filter, size_t seed) Q_DECL_NOTHROW;

private:
    QExplicitlySharedDataPointer<QMqttTopicFilterPrivate> d;
};

Q_DECLARE_SHARED(QMqttTopicFilter)
Q_DECLARE_OPERATORS_FOR_FLAGS(QMqttTopicFilter::MatchOptions)

#ifndef QT_NO_DATASTREAM
Q_MQTT_EXPORT QDataStream &operator<<(QDataStream &, const QMqttTopicFilter &);
Q_MQTT_EXPORT QDataStream &operator>>(QDataStream &, QMqttTopicFilter &);
#endif

#ifndef QT_NO_DEBUG_STREAM
Q_MQTT_EXPORT QDebug operator<<(QDebug, const QMqttTopicFilter &);
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QMqttTopicFilter)

#endif // QMQTTTOPICFILTER_H
