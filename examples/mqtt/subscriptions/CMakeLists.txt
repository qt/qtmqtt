# Generated from subscriptions.pro.

cmake_minimum_required(VERSION 3.14)
project(mqttsubscriptions LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/subscriptions")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Network)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS Mqtt)

add_qt_gui_executable(mqttsubscriptions
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    subscriptionwindow.cpp subscriptionwindow.h subscriptionwindow.ui
)
target_compile_definitions(mqttsubscriptions PUBLIC
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(mqttsubscriptions PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Mqtt
    Qt::Network
    Qt::Widgets
)

install(TARGETS mqttsubscriptions
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
