# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(WIN32 OR (LINUX AND NOT CMAKE_CROSSCOMPILING))
    add_subdirectory(cmake)
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
