# Generated from quicksubscription.pro.

cmake_minimum_required(VERSION 3.14)
project(quicksubscription LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/quicksubscription")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Qml)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS Mqtt)

add_qt_gui_executable(quicksubscription
    main.cpp
    qmlmqttclient.cpp qmlmqttclient.h
)
target_compile_definitions(quicksubscription PUBLIC
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(quicksubscription PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Mqtt
    Qt::Qml
    Qt::Quick
)


# Resources:
set(qml_resource_files
    "main.qml"
)

qt6_add_resources(quicksubscription "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)

install(TARGETS quicksubscription
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
