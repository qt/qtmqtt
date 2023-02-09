# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmqtttopicfilter Test:
#####################################################################

qt_internal_add_test(tst_qmqtttopicfilter
    SOURCES
        tst_qmqtttopicfilter.cpp
    DEFINES
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/"
    LIBRARIES
        Qt::MqttPrivate
        Qt::Mqtt
)
