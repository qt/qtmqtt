# Generated from websocketsubscription.pro.

cmake_minimum_required(VERSION 3.14)
project(websocketsubscription LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/websocketsubscription")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS WebSockets)
find_package(Qt6 COMPONENTS Mqtt)

add_executable(websocketsubscription
    clientsubscription.cpp clientsubscription.h
    main.cpp
    websocketiodevice.cpp websocketiodevice.h
)
target_compile_definitions(websocketsubscription PUBLIC
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(websocketsubscription PUBLIC
    Qt::Core
    Qt::Mqtt
    Qt::WebSockets
)

install(TARGETS websocketsubscription
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
