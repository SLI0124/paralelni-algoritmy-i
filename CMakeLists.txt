cmake_minimum_required(VERSION 3.27)
project(paralelni-algoritmy-i)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable OpenMP if available
find_package(OpenMP REQUIRED CXX)

# Check if OpenMP is found
if (OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found, enabling parallelization.")

    # Print the _OPENMP version for debugging
    add_definitions(-DOPENMP_VERSION=${_OPENMP})
    message(STATUS "Detected OpenMP version: ${_OPENMP}")

    # Check if OpenMP version is 3.0 or higher
    if (_OPENMP LESS 200805)
        message(FATAL_ERROR "OpenMP version 3.0 or higher is required, but found version ${_OPENMP}.")
    endif ()

else ()
    message(FATAL_ERROR "OpenMP not found.")
endif ()

# Add executables
add_executable(srflp project_1/srflp.cpp)
add_executable(affinity_propagation project_2/affinity_propagation.cpp)
add_executable(page_rank project_3/page_rank.cpp)

# Link OpenMP if found
if (OpenMP_CXX_FOUND)
    target_link_libraries(affinity_propagation OpenMP::OpenMP_CXX)
    target_link_libraries(page_rank OpenMP::OpenMP_CXX)
endif ()

# Compiler flags for each target
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(srflp PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(affinity_propagation PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(page_rank PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(affinity_propagation PRIVATE /W4)
    target_compile_options(page_rank PRIVATE /W4)
    target_compile_options(srflp PRIVATE /W4)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MinGW")
    target_compile_options(srflp PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(affinity_propagation PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(page_rank PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()
