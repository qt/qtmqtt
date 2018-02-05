CONFIG += testcase
QT       += testlib mqtt
QT       -= gui
QT_PRIVATE += mqtt-private

TARGET = tst_qmqtttopicfilter

SOURCES += \
    tst_qmqtttopicfilter.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
