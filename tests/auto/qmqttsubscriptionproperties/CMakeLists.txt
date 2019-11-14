# Generated from qmqttsubscriptionproperties.pro.

#####################################################################
## tst_qmqttsubscriptionproperties Test:
#####################################################################

qt_add_test(tst_qmqttsubscriptionproperties
    SOURCES
        ../../common/broker_connection.h
        tst_qmqttsubscriptionproperties.cpp
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
