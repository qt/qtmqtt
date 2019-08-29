# Generated from auto.pro.

if(WIN32 OR (LINUX AND NOT cross_compile))
    add_subdirectory(conformance)
    add_subdirectory(qmqttconnectionproperties)
    add_subdirectory(qmqttcontrolpacket)
    add_subdirectory(qmqttclient)
    add_subdirectory(qmqttlastwillproperties)
    add_subdirectory(qmqttpublishproperties)
    add_subdirectory(qmqttsubscription)
    add_subdirectory(qmqttsubscriptionproperties)
    add_subdirectory(qmqtttopicname)
    add_subdirectory(qmqtttopicfilter)
endif()
