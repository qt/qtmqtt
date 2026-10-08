// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QMQTTMESSAGE_P_H
#define QMQTTMESSAGE_P_H

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

#include "qmqttglobal.h"
#include "qmqtttopicname.h"
#include "qmqttpublishproperties.h"

#include <QtCore/QSharedData>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QMqttMessagePrivate : public QSharedData
{
public:
    bool operator==(const QMqttMessagePrivate &other) const {
        return m_topic == other.m_topic
                && m_payload == other.m_payload
                && m_id == other.m_id
                && m_qos == other.m_qos
                && m_duplicate == other.m_duplicate
                && m_retain == other.m_retain;
    }
    QMqttTopicName m_topic;
    QByteArray m_payload;
    quint16 m_id{0};
    quint8 m_qos{0};
    bool m_duplicate{false};
    bool m_retain{false};
    QMqttPublishProperties m_publishProperties;
};

QT_END_NAMESPACE

#endif // QMQTTMESSAGE_P_H
