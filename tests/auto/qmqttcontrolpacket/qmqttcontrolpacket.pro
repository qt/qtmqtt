CONFIG += testcase
QT       += network testlib mqtt
QT       -= gui
QT_PRIVATE += mqtt-private

TARGET = tst_qmqttcontrolpacket

SOURCES += \
    tst_qmqttcontrolpacket.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
