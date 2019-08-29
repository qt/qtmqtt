# Generated from qmqttcontrolpacket.pro.

#####################################################################
## tst_qmqttcontrolpacket Test:
#####################################################################

add_qt_test(tst_qmqttcontrolpacket
    GUI
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
