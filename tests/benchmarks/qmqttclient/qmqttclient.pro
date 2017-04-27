CONFIG += testcase
QT       += network testlib mqtt
QT       -= gui
QT_PRIVATE += mqtt-private

TARGET = tst_qmqttclient

SOURCES += \
    tst_qmqttclient.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"
