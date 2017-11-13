TARGET = QtMqtt

CONFIG += c++11
QT = core network

QT += core-private

QMAKE_DOCS = $$PWD/doc/qtmqtt.qdocconf

PUBLIC_HEADERS += \
    qmqttglobal.h \
    qmqttclient.h \
    qmqttmessage.h \
    qmqttsubscription.h \
    qmqtttopicfilter.h \
    qmqtttopicname.h

PRIVATE_HEADERS += \
    qmqttclient_p.h \
    qmqttconnection_p.h \
    qmqttcontrolpacket_p.h \
    qmqttsubscription_p.h

SOURCES += \
    qmqttclient.cpp \
    qmqttconnection.cpp \
    qmqttcontrolpacket.cpp \
    qmqttmessage.cpp \
    qmqttsubscription.cpp \
    qmqtttopicfilter.cpp \
    qmqtttopicname.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

load(qt_module)
