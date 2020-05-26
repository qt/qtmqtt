# Generated from qmqtttopicname.pro.

#####################################################################
## tst_qmqtttopicname Test:
#####################################################################

qt_add_test(tst_qmqtttopicname
    SOURCES
        tst_qmqtttopicname.cpp
    DEFINES
        SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}/\\\"
    LIBRARIES
        Qt::MqttPrivate
    PUBLIC_LIBRARIES
        Qt::Mqtt
)
