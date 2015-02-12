cmake_minimum_required (VERSION 2.6)
project (AdapTest_Examples)

include_directories(../adaptest)

add_executable(AdapTest_BasicExample basic.cpp)
add_executable(AdapTest_Buffer 			 buffer.cpp)
