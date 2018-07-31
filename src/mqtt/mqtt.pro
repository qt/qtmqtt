TARGET = QtMqtt

CONFIG += c++11
QT = core network

QT += core-private

QMAKE_DOCS = $$PWD/doc/qtmqtt.qdocconf

PUBLIC_HEADERS += \
    qmqttauthenticationproperties.h \
    qmqttglobal.h \
    qmqttclient.h \
    qmqttconnectionproperties.h \
    qmqttmessage.h \
    qmqttpublishproperties.h \
    qmqttsubscription.h \
    qmqttsubscriptionproperties.h \
    qmqtttopicfilter.h \
    qmqtttopicname.h \
    qmqtttype.h

PRIVATE_HEADERS += \
    qmqttclient_p.h \
    qmqttconnection_p.h \
    qmqttconnectionproperties_p.h \
    qmqttcontrolpacket_p.h \
    qmqttmessage_p.h \
    qmqttpublishproperties_p.h \
    qmqttsubscription_p.h

SOURCES += \
    qmqttauthenticationproperties.cpp \
    qmqttclient.cpp \
    qmqttconnection.cpp \
    qmqttconnectionproperties.cpp \
    qmqttcontrolpacket.cpp \
    qmqttmessage.cpp \
    qmqttpublishproperties.cpp \
    qmqttsubscription.cpp \
    qmqttsubscriptionproperties.cpp \
    qmqtttopicfilter.cpp \
    qmqtttopicname.cpp \
    qmqtttype.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

load(qt_module)
