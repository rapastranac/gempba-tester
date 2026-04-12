cmake_minimum_required(VERSION 3.28)
project(GemPBA-tester VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# Optional GemPBA build flags — uncomment as needed
#set(GEMPBA_MULTIPROCESSING ON CACHE BOOL "" FORCE)  # Enable MPI multiprocessing
#set(GEMPBA_DEBUG_COMMENTS  ON CACHE BOOL "" FORCE)  # Enable verbose debug output
#set(GEMPBA_BUILD_EXAMPLES  ON CACHE BOOL "" FORCE)  # Build bundled examples
#set(GEMPBA_BUILD_TESTS     ON CACHE BOOL "" FORCE)  # Build bundled tests

add_subdirectory(external)

add_compile_options(
        -std=c++23
        -O3
        -Wall
        -Wpedantic
        -Wextra
        -fexceptions
)

add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE
        external    # GemPBA + MPI + fmt + spdlog (via external/CMakeLists.txt)
        stdc++exp
)