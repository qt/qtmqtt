# Generated from qmqttclient.pro.

#####################################################################
## tst_qmqttclient Test:
#####################################################################

qt_add_test(tst_qmqttclient
    SOURCES
        ../../common/broker_connection.h
        tst_qmqttclient.cpp
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
