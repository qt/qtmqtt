TEMPLATE = subdirs
SUBDIRS += \
           simpleclient \
           subscriptions

qtHaveModule(quick): SUBDIRS += quicksubscription
qtHaveModule(websockets): SUBDIRS += websocketsubscription
