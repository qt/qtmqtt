# Generated from mqtt.pro.

#####################################################################
## Mqtt Module:
#####################################################################

add_qt_module(Mqtt
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
)

extend_target(Mqtt CONDITION MSVC AND (TEST_architecture_arch STREQUAL "i386")
    LINK_OPTIONS
        "/BASE:0x68000000"
)

#### Keys ignored in scope 1:.:.:mqtt.pro:<TRUE>:
# _LOADED = "qt_module"
add_qt_docs(
    doc/qtmqtt.qdocconf
)

