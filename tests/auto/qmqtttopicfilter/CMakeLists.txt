# Generated from qmqtttopicfilter.pro.

#####################################################################
## tst_qmqtttopicfilter Test:
#####################################################################

qt_add_test(tst_qmqtttopicfilter
    SOURCES
        tst_qmqtttopicfilter.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/\\\"
    LIBRARIES
        Qt::MqttPrivate
    PUBLIC_LIBRARIES
        Qt::Mqtt
)
