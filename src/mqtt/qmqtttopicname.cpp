// Copyright (C) 2017 Lorenz Haas
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qmqtttopicname.h"

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

/*!
    \class QMqttTopicName
    \inmodule QtMqtt
    \reentrant
    \ingroup shared

    \brief The QMqttTopicName class represents a MQTT topic name.

    QMqttTopicName is a thin wrapper around a QString providing an expressive
    data type for MQTT topic names. Beside the benefits of having a strong type
    preventing unintended misuse, QMqttTopicName provides convenient functions
    related to topic names like isValid() or levels().

    \sa QMqttTopicFilter
 */

/*!
    \fn void QMqttTopicName::swap(QMqttTopicName &other)
    Swaps the MQTT topic name \a other with this MQTT topic name. This
    operation is very fast and never fails.
 */

class QMqttTopicNamePrivate : public QSharedData
{
public:
    QString name;
};

/*!
    Creates a new MQTT topic name with the specified \a name.
 */
QMqttTopicName::QMqttTopicName(const QString &name) : d(new QMqttTopicNamePrivate)
{
    d->name = name;
}

/*!
    Creates a new MQTT topic name with the specified \a name.
 */
QMqttTopicName::QMqttTopicName(const QLatin1String &name) : d(new QMqttTopicNamePrivate)
{
    d->name = name;
}

/*!
    Creates a new MQTT topic name as a copy of \a name.
 */
QMqttTopicName::QMqttTopicName(const QMqttTopicName &name) : d(name.d)
{
}

/*!
    Destroys the QMqttTopicName object.
 */
QMqttTopicName::~QMqttTopicName()
{
}

/*!
    Assigns the MQTT topic name \a name to this object, and returns a reference
    to the copy.
 */
QMqttTopicName &QMqttTopicName::operator=(const QMqttTopicName &name)
{
    d = name.d;
    return *this;
}

/*!
    Returns the topic name.
 */
QString QMqttTopicName::name() const
{
    return d->name;
}

/*!
    Sets the topic name to \a name.
 */
void QMqttTopicName::setName(const QString &name)
{
    d.detach();
    d->name = name;
}

/*!
    Returns \c true if the topic name is valid according to the MQTT standard
    section 4.7, or \c false otherwise.
 */
bool QMqttTopicName::isValid() const
{
    const int bytes = d->name.size();
    return bytes > 0                                    // [MQTT-4.7.3-1]
           && bytes < 65536                             // [MQTT-4.7.3-3]
           && !d->name.contains(QLatin1Char('#'))       // [MQTT-4.7.1-1]
           && !d->name.contains(QLatin1Char('+'))       // [MQTT-4.7.1-1]
           && !d->name.contains(QChar(QChar::Null));    // [MQTT-4.7.3-2]
}

/*!
    Returns the total number of topic levels.
 */
int QMqttTopicName::levelCount() const
{
    return d->name.isEmpty() ? 0 : d->name.count(QLatin1Char('/')) + 1;
}

/*!
    Returns the topic levels.
 */
QStringList QMqttTopicName::levels() const
{
    return d->name.split(QLatin1Char('/'), Qt::KeepEmptyParts);
}

/*!
    //! friend
    \fn bool QMqttTopicName::operator==(const QMqttTopicName &lhs, const QMqttTopicName &rhs)

    Returns \c true if the topic names \a lhs and \a rhs are equal,
    otherwise returns \c false.
 */
bool operator==(const QMqttTopicName &lhs, const QMqttTopicName &rhs) Q_DECL_NOTHROW
{
    return (lhs.d == rhs.d) || (lhs.d->name == rhs.d->name);
}

/*!
    //! friend
    \fn bool QMqttTopicName::operator!=(const QMqttTopicName &lhs, const QMqttTopicName &rhs)

    Returns \c true if the topic names \a lhs and \a rhs are different,
    otherwise returns \c false.
 */

/*!
    //! friend
    \fn bool QMqttTopicName::operator<(const QMqttTopicName &lhs, const QMqttTopicName &rhs)

    Returns \c true if the topic name \a lhs is lexically less than the topic
    name \a rhs; otherwise returns \c false.
 */
bool operator<(const QMqttTopicName &lhs, const QMqttTopicName &rhs) Q_DECL_NOTHROW
{
    return lhs.d->name < rhs.d->name;
}

/*!
    \relates QHash

    Returns the hash value for \a name. If specified, \a seed is used to
    initialize the hash.
*/
size_t qHash(const QMqttTopicName &name, size_t seed) Q_DECL_NOTHROW
{
    return qHash(name.d->name, seed);
}

#ifndef QT_NO_DATASTREAM
/*! \relates QMqttTopicName

    Writes the topic name \a name to the stream \a out and returns a reference
    to the stream.

    \sa{Serializing Qt Data Types}{Format of the QDataStream operators}
*/
QDataStream &operator<<(QDataStream &out, const QMqttTopicName &name)
{
    out << name.name();
    return out;
}

/*! \relates QMqttTopicName

    Reads a topic name into \a name from the stream \a in and returns a
    reference to the stream.

    \sa{Serializing Qt Data Types}{Format of the QDataStream operators}
*/
QDataStream &operator>>(QDataStream &in, QMqttTopicName &name)
{
    QString n;
    in >> n;
    name.setName(n);
    return in;
}
#endif // QT_NO_DATASTREAM

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug d, const QMqttTopicName &name)
{
    QDebugStateSaver saver(d);
    d.nospace() << "QMqttTopicName(" << name.name() << ')';
    return d;
}
#endif

QT_END_NAMESPACE
