cmake_minimum_required(VERSION 3.5)
project(hdtasstatic)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# Usages: cmake -DCMAKE_BUILD_TYPE=Debug ..
set( CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb" )
# Usages: cmake -DCMAKE_BUILD_TYPE=Release ..
set( CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set( SOURCE_FILES
        utility.cpp
        protocol_data_parser.cpp
        hdtasparser.cpp )

add_library(hdtas STATIC ${SOURCE_FILES})

