cmake_minimum_required(VERSION 3.28)

project(GemPBA-tester VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)


#sets the executable in main folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

add_compile_options(
        -std=c++23
        -O3
        -Wall
        -Wpedantic
        -Wextra
        -fexceptions
)

# Add the source files
add_executable(main src/main.cpp)
target_compile_definitions(main PRIVATE)
