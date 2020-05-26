# Generated from conformance.pro.

#####################################################################
## tst_conformance Test:
#####################################################################

qt_add_test(tst_conformance
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
