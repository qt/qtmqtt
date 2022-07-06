# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(simplemqttclient LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/simpleclient")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Mqtt Network Widgets)

qt_add_executable(simplemqttclient
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)

set_target_properties(simplemqttclient PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
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

target_link_libraries(simplemqttclient PUBLIC Qt::Widgets)

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
