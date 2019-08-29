# Generated from qmqtttopicfilter.pro.

#####################################################################
## tst_qmqtttopicfilter Test:
#####################################################################

add_qt_test(tst_qmqtttopicfilter
    GUI
    SOURCES
        tst_qmqtttopicfilter.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/\\\"
    LIBRARIES
        Qt::MqttPrivate
    PUBLIC_LIBRARIES
        Qt::Mqtt
)
