cmake_minimum_required(VERSION 3.28)

project(GemPBA-tester VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

# GemPBA flags and includes
set(GEMPBA_MULTIPROCESSING ON CACHE BOOL "" FORCE) # (Needed if you want to enable multiprocessing)
#set(GEMPBA_DEBUG_COMMENTS ON CACHE BOOL "" FORCE)  # (Optional) Enable debug comments in the code
#set(GEMPBA_BUILD_EXAMPLES ON CACHE BOOL "" FORCE)  # (Optional) Enable building examples
#set(GEMPBA_BUILD_TESTS ON CACHE BOOL "" FORCE)     # (Optional) Enable building tests
add_subdirectory(external)

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

target_link_libraries(main PUBLIC
        gempba::gempba
)
