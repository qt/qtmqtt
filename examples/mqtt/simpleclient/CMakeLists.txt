# Generated from simpleclient.pro.

cmake_minimum_required(VERSION 3.14)
project(simplemqttclient LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/simpleclient")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Network)
find_package(Qt6 COMPONENTS Mqtt)
find_package(Qt6 COMPONENTS Widgets) # special case

add_qt_gui_executable(simplemqttclient
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)
target_compile_definitions(simplemqttclient PUBLIC
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(simplemqttclient PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Mqtt
    Qt::Network
)
target_link_libraries(simplemqttclient PUBLIC Qt::Widgets) # special case

if((QT_MAJOR_VERSION GREATER 4))
    target_link_libraries(simplemqttclient PUBLIC
        Qt::Widgets
    )
endif()

install(TARGETS simplemqttclient
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
