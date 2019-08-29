cmake_minimum_required(VERSION 3.15.0)

project(QtMqtt
    VERSION 6.0.0
    DESCRIPTION "Qt Mqtt Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core Network)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Quick WebSockets # For tests
                                                               Gui Widgets) # For examples
qt_build_repo()

