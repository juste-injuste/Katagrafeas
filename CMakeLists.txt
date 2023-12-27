cmake_minimum_required(VERSION 3.5)

project(Katagrafeas)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -g3 -Wall -Wextra -Wconversion -Wpedantic -pedantic -Wdisabled-optimization -Wstrict-aliasing=2 -Wstrict-overflow=5 -Wmissing-include-dirs")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(KATAGRAFEAS_SRC     ${CMAKE_CURRENT_SOURCE_DIR}/examples/)
set(KATAGRAFEAS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

include_directories(${KATAGRAFEAS_INCLUDE})

add_executable(Katagrafeas ${KATAGRAFEAS_SRC}/main.cpp)