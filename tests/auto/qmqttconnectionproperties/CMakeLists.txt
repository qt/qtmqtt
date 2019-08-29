# Generated from qmqttconnectionproperties.pro.

#####################################################################
## tst_qmqttconnectionproperties Test:
#####################################################################

add_qt_test(tst_qmqttconnectionproperties
    GUI
    SOURCES
        ../../common/broker_connection.h
        tst_qmqttconnectionproperties.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/\\\"
    INCLUDE_DIRECTORIES
        ../../common
    LIBRARIES
        Qt::MqttPrivate
    PUBLIC_LIBRARIES
        Qt::Mqtt
        Qt::Network
)
