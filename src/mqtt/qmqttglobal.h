/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
****************************************************************************/

#pragma once

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
