TEMPLATE = app
TARGET = quickpublication

QT += qml quick mqtt
CONFIG += qmltypes

SOURCES += main.cpp \
    qmlmqttclient.cpp

HEADERS += \
    qmlmqttclient.h

qml_resources.files = Main.qml qmldir
qml_resources.prefix = /qt/qml/publication

RESOURCES += qml_resources

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = $$pwd/.
QML_IMPORT_NAME = publication
QML_IMPORT_MAJOR_VERSION = 1

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_UP_TO=0x060000 # disables all APIs deprecated in Qt 6.0.0 and earlier

target.path = $$[QT_INSTALL_EXAMPLES]/mqtt/quickpublication
INSTALLS += target
