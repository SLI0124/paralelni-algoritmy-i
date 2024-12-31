cmake_minimum_required(VERSION 3.27)
project(paralelni-algoritmy-i)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable OpenMP
find_package(OpenMP REQUIRED)

if (OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found, enabling parallelization.")
    add_definitions(-DOPENMP_VERSION=${_OPENMP})
    message(STATUS "Detected OpenMP version: ${_OPENMP}")

    if (_OPENMP LESS 200805)
        message(FATAL_ERROR "OpenMP version 3.0 or higher is required.")
    endif ()
else ()
    message(FATAL_ERROR "OpenMP not found.")
endif ()

# Add executables
add_executable(srflp project_1/srflp.cpp)
add_executable(affinity_propagation project_2/affinity_propagation.cpp)
add_executable(page_rank project_3/page_rank.cpp)

# Link OpenMP
target_link_libraries(affinity_propagation OpenMP::OpenMP_CXX)
target_link_libraries(page_rank OpenMP::OpenMP_CXX)

# Compiler flags for Linux
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(srflp PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(affinity_propagation PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(page_rank PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()
