# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from qmqttsubscription.pro.

#####################################################################
## tst_qmqttsubscription Test:
#####################################################################

qt_internal_add_test(tst_qmqttsubscription
    SOURCES
        ../../common/broker_connection.h
        tst_qmqttsubscription.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/\\\"
    INCLUDE_DIRECTORIES
        ../../common
    LIBRARIES
        Qt::MqttPrivate
        Qt::Mqtt
        Qt::Network
)
