# Generated from qmqttconnectionproperties.pro.

#####################################################################
## tst_qmqttconnectionproperties Test:
#####################################################################

qt_add_test(tst_qmqttconnectionproperties
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
