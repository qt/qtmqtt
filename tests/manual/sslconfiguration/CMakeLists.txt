# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## sslconfiguration Binary:
#####################################################################

qt_add_manual_test(sslconfiguration
    SOURCES
        main.cpp
    DEFINES
        QT_DEPRECATED_WARNINGS
    LIBRARIES
        Qt::Mqtt
        Qt::Network
)
