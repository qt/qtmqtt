# Generated from mqtt.pro.

add_subdirectory(consolepubsub)
if(TARGET Qt::Gui AND TARGET Qt::Widgets)
    add_subdirectory(simpleclient)
    add_subdirectory(subscriptions)
endif()
if(TARGET Qt::Quick)
    add_subdirectory(quicksubscription)
    add_subdirectory(quickpublication)
endif()
if(TARGET Qt::WebSockets)
    add_subdirectory(websocketsubscription)
endif()
