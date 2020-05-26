# Generated from qmqttcontrolpacket.pro.

#####################################################################
## tst_qmqttcontrolpacket Test:
#####################################################################

qt_add_test(tst_qmqttcontrolpacket
    SOURCES
        tst_qmqttcontrolpacket.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/\\\"
    LIBRARIES
        Qt::MqttPrivate
    PUBLIC_LIBRARIES
        Qt::Mqtt
        Qt::Network
)
