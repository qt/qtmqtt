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

#ifndef QMQTTSUBSCRIPTION_P_H
#define QMQTTSUBSCRIPTION_P_H

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

#include "qmqttsubscription.h"
#include <private/qobject_p.h>

class QMqttSubscriptionPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QMqttSubscription)
public:
    QMqttSubscriptionPrivate();
    ~QMqttSubscriptionPrivate() override = default;
    QMqttClient *m_client{nullptr};
    QMqttSubscription::SubscriptionState m_state{QMqttSubscription::Unsubscribed};
    QString m_topic;
    quint8 m_qos{0};
};

#endif // QMQTTSUBSCRIPTION_P_H
