TEMPLATE = subdirs

win32|if(linux:!cross_compile): SUBDIRS += cmake \
                                      conformance \
                                      qmqttconnectionproperties \
                                      qmqttcontrolpacket \
                                      qmqttclient \
                                      qmqttpublishproperties \
                                      qmqttsubscription \
                                      qmqttsubscriptionproperties \
                                      qmqtttopicname \
                                      qmqtttopicfilter
