# Generated from qmqtttopicname.pro.

#####################################################################
## tst_qmqtttopicname Test:
#####################################################################

add_qt_test(tst_qmqtttopicname
    GUI
    SOURCES
        tst_qmqtttopicname.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/\\\"
    LIBRARIES
        Qt::MqttPrivate
    PUBLIC_LIBRARIES
        Qt::Mqtt
)
