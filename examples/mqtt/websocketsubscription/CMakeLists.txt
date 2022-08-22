# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(websocketsubscription LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/websocketsubscription")

find_package(Qt6 REQUIRED COMPONENTS Core Mqtt WebSockets)

qt_add_executable(websocketsubscription
    clientsubscription.cpp clientsubscription.h
    main.cpp
    websocketiodevice.cpp websocketiodevice.h
)

set_target_properties(websocketsubscription PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
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
