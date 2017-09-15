TEMPLATE = subdirs

win32|if(linux:!cross_compile): SUBDIRS += cmake \
                                      conformance \
                                      qmqttcontrolpacket \
                                      qmqttclient \
                                      qmqttsubscription
