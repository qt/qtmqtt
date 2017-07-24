CONFIG += benchmark
QT       += network testlib mqtt
QT       -= gui
QT_PRIVATE += mqtt-private

TARGET = tst_qmqttclient

SOURCES += \
    tst_qmqttclient.cpp

HEADERS += \
    $$PWD/../../common/broker_connection.h

INCLUDEPATH += \
    $$PWD/../../common

DEFINES += SRCDIR=\\\"$$PWD/\\\"
