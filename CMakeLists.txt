cmake_minimum_required(VERSION 3.30)
project(paralelni-algoritmy-i)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable OpenMP if available
find_package(OpenMP)
if (OpenMP_CXX_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    message(STATUS "OpenMP found, enabling parallelization.")
else ()
    message(WARNING "OpenMP not found, parallelization will be disabled.")
endif ()

# Add executables
add_executable(srflp project_1/srflp.cpp)
add_executable(affinity_propagation project_2/affinity_propagation.cpp)
add_executable(page_rank project_3/page_rank.cpp)

# Compiler flags for each target
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(srflp PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(affinity_propagation PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_options(page_rank PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(srflp PRIVATE /W4 /WX)
    target_compile_options(affinity_propagation PRIVATE /W4 /WX)
    target_compile_options(page_rank PRIVATE /W4 /WX)
endif ()
