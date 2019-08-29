# special case begin
if(NOT TARGET Qt::Test)
    cmake_minimum_required(VERSION 3.14.0)
    project(QtMqttTests C CXX)
    find_package(Qt6 REQUIRED COMPONENTS BuildInternals Core Network Test)
    qt_set_up_standalone_tests_build()
endif()
# special case end

qt_build_tests()
