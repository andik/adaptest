cmake_minimum_required (VERSION 2.6)
project (AdapTest_Examples)
# The version number.
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)

include_directories(../adaptest)

add_executable(AdapTest_BasicExample basic.cpp)