# Generated from conformance.pro.

#####################################################################
## tst_conformance Test:
#####################################################################

add_qt_test(tst_conformance
    GUI
    SOURCES
        ../../common/broker_connection.h
        tst_conformance.cpp
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
