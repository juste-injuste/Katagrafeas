cmake_minimum_required(VERSION 3.5)

project(Katagrafeas)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23 -Ofast -g3 -Wall -Wextra -Wconversion -Wpedantic -pedantic -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-aliasing=2 -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused-parameter -Wdeprecated")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(KATAGRAFEAS_SRC     ${CMAKE_CURRENT_SOURCE_DIR}/examples/)
set(KATAGRAFEAS_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

include_directories(${KATAGRAFEAS_INCLUDE})

add_executable(Katagrafeas ${KATAGRAFEAS_SRC}/main.cpp ${KATAGRAFEAS_SRC}/ODR.cpp)