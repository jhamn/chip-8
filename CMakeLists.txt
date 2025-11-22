cmake_minimum_required(VERSION 3.10)
project(chip8_emulator)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(chip8_emulator src/main.cpp src/chip8.cpp)

target_link_libraries(chip8_emulator ${SDL2_LIBRARIES})
