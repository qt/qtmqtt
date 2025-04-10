# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# This function adds the test 12 times if run on a computer where the
# mosquitto broker has been provisioned by running the corresponding
# script located qt6/coin/provisioning/*/*mosquitto*
#
# The script adds the file /etc/mosquitto/conf.d/mqtt.conf on unix-like
# systems, and the file C:\\Program Files\\mosquitto\\conf.d\\mqtt.conf
# on windows like systems. So this is what we test for.
#
# If we are on a provisioned system as detected by the two files above
# we define these variables which the test use
#   MQTT_TEST_TYPE= selection from; 3, 4, 5
#     3 -> Vers 3.1
#     4 -> Vers 3.1.1
#     5 -> Vers 5.0
#   MQTT_TEST_PROTOCOL= selection from; Tcp,Ssl,Ws,Wss
#   MQTT_TEST_BROKER="localhost"
#
# If neither of these files are present, we use the existing
# design which is to run for tcp,3.1.1 by default.
# In this case set the environment variable MQTT_TEST_BROKER
# to localhost
#
# Finally, we allow the test to override for individual test
# cases, if wanted. This is mostly used for MQTT_TEST_TYPE
#

function(qtmqtt_is_mosquitto_installed result)
    set(skip $ENV{COIN_NO_MOSQUITTO})
    if(skip)
       set(${result} FALSE PARENT_SCOPE)
     elseif(EXISTS "/etc/mosquitto/conf.d/mqtt.conf" OR
       EXISTS "C:\\Program Files\\mosquitto\\conf.d\\mqtt.conf")
       set(${result} ON PARENT_SCOPE)
    else()
       set(${result} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(qtmqtt_add_test)

    set(options)
    set(oneValueArgs TARGET)
    set(multiValueArgs SOURCES DEFINES INCLUDE_DIRECTORIES LIBRARIES)

    cmake_parse_arguments(
        arg_mqtt
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    set(TYPES)
    set(PROTOCOLS)

    qtmqtt_is_mosquitto_installed(is_installed)

    if(${is_installed})
        if(WASM)
            if(TARGET Qt::WebSockets)
                set(TYPES Ws Wss)
                set(PROTOCOLS 3 4 5)
            endif()
        elseif(QT_FEATURE_ssl)
            if(TARGET Qt::WebSockets)
                set(TYPES Tcp Ssl Ws Wss)
            else()
                set(TYPES Tcp Ssl)
            endif()
            set(PROTOCOLS 3 4 5)
        else()
            if(TARGET Qt::WebSockets)
                set(TYPES Tcp Ws)
            else()
                set(TYPES Tcp)
            endif()
            set(PROTOCOLS 3 4 5)
        endif()
    elseif(NOT WASM)
        set(TYPES Tcp)
        set(PROTOCOLS 4)
    endif()

    foreach(TYPE ${TYPES})
        foreach(PROTOCOL ${PROTOCOLS})
            set(EXTRA_DEFINES)
            set(EXTRA_LIBS)
            if(${is_installed})
                set(EXTRA_DEFINES MQTT_TEST_TYPE="${TYPE}" MQTT_TEST_PROTOCOL=${PROTOCOL} MQTT_TEST_BROKER="localhost")
            endif()

            if(TARGET Qt::WebSockets)
                set(EXTRA_DEFINES ${EXTRA_DEFINES} QT_MQTT_WITH_WEBSOCKETS)
                set(EXTRA_LIBS Qt::WebSockets)
            endif()

            qt_internal_add_test(
                    ${arg_mqtt_TARGET}_${TYPE}_${PROTOCOL}
                SOURCES
                    ${arg_mqtt_SOURCES}
                DEFINES
                    ${arg_mqtt_DEFINES} ${EXTRA_DEFINES}
                INCLUDE_DIRECTORIES
                    ${arg_mqtt_INCLUDE_DIRECTORIES}
                LIBRARIES
                    ${arg_mqtt_LIBRARIES} ${EXTRA_LIBS}
                TESTDATA
                    "../../certificate/cert.crt"
                    "../../certificate/cert.key"
                BUILTIN_TESTDATA
            )

        endforeach()
    endforeach()
endfunction()

if(QT_BUILD_STANDALONE_TESTS)
    # Add qt_find_package calls for extra dependencies that need to be found when building
    # the standalone tests here.
endif()
qt_build_tests()
