# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmqttconnectionproperties Test:
#####################################################################

qt_internal_add_test(tst_qmqttconnectionproperties
    SOURCES
        ../../common/broker_connection.h
        tst_qmqttconnectionproperties.cpp
    DEFINES
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/"
    INCLUDE_DIRECTORIES
        ../../common
    LIBRARIES
        Qt::MqttPrivate
        Qt::Mqtt
        Qt::Network
)
