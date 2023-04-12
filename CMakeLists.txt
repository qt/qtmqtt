# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtMqtt
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Mqtt Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Quick WebSockets # For tests
                                                               Gui Widgets) # For examples
qt_internal_project_setup()

qt_build_repo()
