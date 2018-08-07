/******************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtMqtt module.
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
******************************************************************************/

#include "qmqtttype.h"

QT_BEGIN_NAMESPACE

/*!
    \class QMqttStringPair

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttStringPair class represents the string pair data type
    of the MQTT 5.0 standard.

    This data type is used to hold a name-value pair.
*/

/*!
    \class QMqttUserProperties

    \inmodule QtMqtt
    \since 5.12

    \brief The QMqttUserProperties class represents a list of QMqttStringPair
    values which can be passed to the server.

    User properties allow a client to pass additional data to the server or
    other subscribers, which do not belong to the message payload.

    On the other hand it also provides a server the flexibility to provide
    further information to connected clients.

    The server might add additional user properties to the ones provided.
    However, the order of the provided properties is not changed during transfer.

    \note User properties are part of the MQTT 5.0 specification and
    cannot be used when using QtMqtt with an older protocol level. See
    QMqttClient::ProtocolVersion for more information.

    \sa QMqttAuthenticationProperties, QMqttConnectionProperties,
    QMqttLastWillProperties, QMqttPublishProperties, QMqttSubscriptionProperties,
    QMqttUnsubscriptionProperties, QMqttSubscription
*/

/*!
    \namespace QMqtt
    \inmodule QtMqtt
    \since 5.12

    \brief Contains miscellaneous identifiers used throughout the Qt MQTT module.
*/

/*!
    \enum QMqtt::PayloadFormatIndicator
    \since 5.12

    The payload format provides information on the content of a message.
    This can help other clients to handle the message faster.

    \value Unspecified
           The format is not specified.
    \value UTF8Encoded
           The payload of the message is formatted as UTF-8 Encoded
           Character Data.
*/

class QMqttStringPairData : public QSharedData
{
public:
    QMqttStringPairData() = default;
    QMqttStringPairData(const QString &name, const QString &value);

    bool operator==(const QMqttStringPairData &rhs) const;
    QString m_name;
    QString m_value;
};

QMqttStringPairData::QMqttStringPairData(const QString &name, const QString &value)
    : m_name(name)
    , m_value(value)
{
}

bool QMqttStringPairData::operator==(const QMqttStringPairData &rhs) const
{
    return m_name == rhs.m_name && m_value == rhs.m_value;
}

QMqttStringPair::QMqttStringPair()
    : data(new QMqttStringPairData)
{

}

QMqttStringPair::QMqttStringPair(const QString &name, const QString &value)
    : data(new QMqttStringPairData(name, value))
{
}

QMqttStringPair::QMqttStringPair(const QMqttStringPair &) = default;

QMqttStringPair::~QMqttStringPair() = default;

/*!
    Returns the name of the string pair.
*/
QString QMqttStringPair::name() const
{
    return data->m_name;
}

/*!
    Sets the name to \a n.
*/
void QMqttStringPair::setName(const QString &n)
{
    data->m_name = n;
}

/*!
    Returns the value of the string pair.
*/
QString QMqttStringPair::value() const
{
    return data->m_value;
}

/*!
    Sets the value to \a v.
*/
void QMqttStringPair::setValue(const QString &v)
{
    data->m_value = v;
}

/*!
    Returns \c true if this instance matches \a other.
*/
bool QMqttStringPair::operator==(const QMqttStringPair &other) const
{
    return *data.constData() == *other.data.constData();
}

QMqttStringPair &QMqttStringPair::operator=(const QMqttStringPair &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QMqttStringPair &s)
{
    QDebugStateSaver saver(d);
    d.nospace() << "QMqttStringPair(" << s.name() << " : " << s.value() << ')';
    return d;
}
#endif

QT_END_NAMESPACE
