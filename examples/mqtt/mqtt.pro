TEMPLATE = subdirs
qtHaveModule(gui): SUBDIRS += \
           simpleclient \
           subscriptions

qtHaveModule(quick): SUBDIRS += quicksubscription
qtHaveModule(quick): SUBDIRS += quickpublication
qtHaveModule(websockets): SUBDIRS += websocketsubscription
