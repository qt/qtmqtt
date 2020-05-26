# Generated from qmqttpublishproperties.pro.

#####################################################################
## tst_qmqttpublishproperties Test:
#####################################################################

qt_add_test(tst_qmqttpublishproperties
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
