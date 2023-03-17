TEMPLATE = subdirs

qtHaveModule(gui):qtHaveModule(widgets): SUBDIRS += \
           simpleclient \
           subscriptions

qtHaveModule(quick): SUBDIRS += quicksubscription
qtHaveModule(quick): SUBDIRS += quickpublication
qtHaveModule(websockets): SUBDIRS += websocketsubscription
