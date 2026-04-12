cmake_minimum_required(VERSION 3.28)
project(external)

# Interface library — no sources compiled here, only dependency propagation
add_library(external INTERFACE)

find_package(MPI     REQUIRED)
find_package(gempba  REQUIRED)
message(STATUS ">>> gempba version found by CMake: ${gempba_VERSION}")

# All dependencies are propagated to any target that links against 'external'
target_link_libraries(external INTERFACE
        gempba::gempba
        MPI::MPI_CXX
)