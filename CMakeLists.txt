cmake_minimum_required(VERSION 3.10)
project(CHIP8Emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES 
    src/main.cpp
    src/chip8.cpp
)

add_executable(CHIP8 ${SOURCES})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

target_link_libraries(CHIP8 ${SDL2_LIBRARIES})
