# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

# Generated from auto.pro.

if(WIN32 OR (LINUX AND NOT CMAKE_CROSSCOMPILING))
# special case begin
#   add_subdirectory(cmake)
# special case end
    add_subdirectory(conformance)
    add_subdirectory(qmqttconnectionproperties)
    add_subdirectory(qmqttcontrolpacket)
    add_subdirectory(qmqttclient)
    add_subdirectory(qmqttlastwillproperties)
    add_subdirectory(qmqttpublishproperties)
    add_subdirectory(qmqttsubscription)
    add_subdirectory(qmqttsubscriptionproperties)
    add_subdirectory(qmqtttopicname)
    add_subdirectory(qmqtttopicfilter)
endif()
