# Generated from consolepubsub.pro.

cmake_minimum_required(VERSION 3.14)
project(qtmqtt_pub LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/consolepubsub")

find_package(Qt6 COMPONENTS Mqtt)

add_executable(qtmqtt_pub
    configuration.h
    main_pub.cpp
)
target_compile_definitions(qtmqtt_pub PUBLIC
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(qtmqtt_pub PUBLIC
    # Remove: gui
    Qt::Mqtt
)

install(TARGETS qtmqtt_pub
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
cmake_minimum_required(VERSION 3.14)
project(qtmqtt_sub LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/mqtt/consolepubsub")

find_package(Qt6 COMPONENTS Mqtt)

add_executable(qtmqtt_sub
    configuration.h
    main_sub.cpp
)
target_compile_definitions(qtmqtt_sub PUBLIC
    QT_DEPRECATED_WARNINGS
)

target_link_libraries(qtmqtt_sub PUBLIC
    # Remove: gui
    Qt::Mqtt
)

install(TARGETS qtmqtt_sub
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
