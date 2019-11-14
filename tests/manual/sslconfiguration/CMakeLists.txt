# Generated from sslconfiguration.pro.

#####################################################################
## sslconfiguration Binary:
#####################################################################

qt_add_manual_test(sslconfiguration
    SOURCES
        main.cpp
    DEFINES
        QT_DEPRECATED_WARNINGS
    PUBLIC_LIBRARIES
        Qt::Mqtt
        Qt::Network
)

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:sslconfiguration.pro:QNX:
# target.path = "/tmp/$${TARGET}/bin"

#### Keys ignored in scope 4:.:.:sslconfiguration.pro:UNIX AND NOT ANDROID:
# target.path = "/opt/$${TARGET}/bin"

#### Keys ignored in scope 5:.:.:sslconfiguration.pro:NOT target.path_ISEMPTY:
# INSTALLS = "target"
