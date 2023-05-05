# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)
project(qtmqtt_pub_sub LANGUAGES CXX)

if (ANDROID)
    message(FATAL_ERROR "This project cannot be built on Android.")
endif()

set(CMAKE_AUTOMOC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/consolepubsub")

find_package(Qt6 REQUIRED COMPONENTS Mqtt)

#
# Pub
#

qt_add_executable(qtmqtt_pub
    configuration.h
    main_pub.cpp
)

set_target_properties(qtmqtt_pub PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

target_compile_definitions(qtmqtt_pub PUBLIC
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(qtmqtt_pub PUBLIC
    Qt::Mqtt
)

install(TARGETS qtmqtt_pub
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)

#
# Sub
#

qt_add_executable(qtmqtt_sub
    configuration.h
    main_sub.cpp
)

set_target_properties(qtmqtt_sub PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

target_compile_definitions(qtmqtt_sub PUBLIC
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(qtmqtt_sub PUBLIC
    Qt::Mqtt
)

install(TARGETS qtmqtt_sub
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
