# Generated from qmqttpublishproperties.pro.

#####################################################################
## tst_qmqttpublishproperties Test:
#####################################################################

add_qt_test(tst_qmqttpublishproperties
    GUI
    SOURCES
        ../../common/broker_connection.h
        tst_qmqttpublishproperties.cpp
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
