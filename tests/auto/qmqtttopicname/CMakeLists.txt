# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from qmqtttopicname.pro.

#####################################################################
## tst_qmqtttopicname Test:
#####################################################################

qt_internal_add_test(tst_qmqtttopicname
    SOURCES
        tst_qmqtttopicname.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/\\\"
    LIBRARIES
        Qt::MqttPrivate
    PUBLIC_LIBRARIES
        Qt::Mqtt
)
