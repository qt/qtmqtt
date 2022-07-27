# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from qmqtttopicfilter.pro.

#####################################################################
## tst_qmqtttopicfilter Test:
#####################################################################

qt_internal_add_test(tst_qmqtttopicfilter
    SOURCES
        tst_qmqtttopicfilter.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/\\\"
    LIBRARIES
        Qt::MqttPrivate
        Qt::Mqtt
)
