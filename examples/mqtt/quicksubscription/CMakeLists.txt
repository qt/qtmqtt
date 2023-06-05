# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(quicksubscription LANGUAGES CXX)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/quicksubscription")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Mqtt Qml Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(quicksubscription
    WIN32
    MACOSX_BUNDLE
    main.cpp
)

target_compile_definitions(quicksubscription PUBLIC
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(quicksubscription PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Mqtt
    Qt::Qml
    Qt::Quick
)

qt_add_qml_module(quicksubscription
    URI subscription
    QML_FILES
        "Main.qml"
    SOURCES
        qmlmqttclient.cpp qmlmqttclient.h
)

install(TARGETS quicksubscription
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
