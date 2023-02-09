# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Mqtt Module:
#####################################################################

qt_internal_add_module(Mqtt
    SOURCES
        qmqttauthenticationproperties.cpp qmqttauthenticationproperties.h
        qmqttclient.cpp qmqttclient.h qmqttclient_p.h
        qmqttconnection.cpp qmqttconnection_p.h
        qmqttconnectionproperties.cpp qmqttconnectionproperties.h qmqttconnectionproperties_p.h
        qmqttcontrolpacket.cpp qmqttcontrolpacket_p.h
        qmqttglobal.h
        qmqttmessage.cpp qmqttmessage.h qmqttmessage_p.h
        qmqttpublishproperties.cpp qmqttpublishproperties.h qmqttpublishproperties_p.h
        qmqttsubscription.cpp qmqttsubscription.h qmqttsubscription_p.h
        qmqttsubscriptionproperties.cpp qmqttsubscriptionproperties.h
        qmqtttopicfilter.cpp qmqtttopicfilter.h
        qmqtttopicname.cpp qmqtttopicname.h
        qmqtttype.cpp qmqtttype.h
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
    GENERATE_CPP_EXPORTS
)
qt_internal_add_docs(Mqtt
    doc/qtmqtt.qdocconf
)
