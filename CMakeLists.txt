cmake_minimum_required(VERSION 2.6)
project(test)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG_ -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "")
###########
## GTest
###########
ADD_SUBDIRECTORY (googletest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

###########
## unit tests
##########
add_definitions (-std=c++11)
add_library(asn asn.cpp)
add_executable( Test test.cpp )

target_link_libraries( Test asn gtest pthread )
add_test( Test Test )
