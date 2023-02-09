# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmqttcontrolpacket Test:
#####################################################################

qt_internal_add_test(tst_qmqttcontrolpacket
    SOURCES
        tst_qmqttcontrolpacket.cpp
    DEFINES
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/"
    LIBRARIES
        Qt::MqttPrivate
        Qt::Mqtt
        Qt::Network
)
