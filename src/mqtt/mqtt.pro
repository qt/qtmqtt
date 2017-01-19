TARGET = QtMqtt

CONFIG += c++11
QT_FOR_PRIVATE = network

QMAKE_DOCS = $$PWD/doc/qtmqtt.qdocconf

PUBLIC_HEADERS += \
    qmqttglobal.h \
    qmqttclient.h

PRIVATE_HEADERS +=

SOURCES += \
    qmqttclient.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

load(qt_module)
