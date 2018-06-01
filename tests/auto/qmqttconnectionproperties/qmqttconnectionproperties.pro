CONFIG += testcase
QT       += network testlib mqtt
QT       -= gui
QT_PRIVATE += mqtt-private

TARGET = tst_qmqttconnectionproperties

SOURCES += \
    tst_qmqttconnectionproperties.cpp

HEADERS += \
    $$PWD/../../common/broker_connection.h

INCLUDEPATH += \
    $$PWD/../../common

DEFINES += SRCDIR=\\\"$$PWD/\\\"
