# Generated from qmqttsubscription.pro.

#####################################################################
## tst_qmqttsubscription Test:
#####################################################################

qt_add_test(tst_qmqttsubscription
    SOURCES
        ../../common/broker_connection.h
        tst_qmqttsubscription.cpp
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
