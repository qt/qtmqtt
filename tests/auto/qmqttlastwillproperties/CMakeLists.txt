# Generated from qmqttlastwillproperties.pro.

#####################################################################
## tst_qmqttlastwillproperties Test:
#####################################################################

add_qt_test(tst_qmqttlastwillproperties
    GUI
    SOURCES
        ../../common/broker_connection.h
        tst_qmqttlastwillproperties.cpp
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
