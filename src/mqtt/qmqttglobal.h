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

#ifndef QTQMQTTGLOBAL_H
#define QTQMQTTGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_MQTT_LIB)
#    define Q_MQTT_EXPORT Q_DECL_EXPORT
#  else
#    define Q_MQTT_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_MQTT_EXPORT
#endif

QT_END_NAMESPACE

#endif //QTQMQTTGLOBAL_H
