CONFIG += testcase
QT       += testlib mqtt
QT       -= gui
QT_PRIVATE += mqtt-private

TARGET = tst_qmqtttopicname

SOURCES += \
    tst_qmqtttopicname.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
