cmake_minimum_required(VERSION 3.30)
project(paralelni-algoritmy-i)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# add executables
add_executable(srflp project_1/srflp.cpp)
add_executable(affinity_propagation project_2/affinity_propagation.cpp)
add_executable(page_rank project_3/page_rank.cpp)

# Compiler flags
if (CMAKE_CXX_COMPILER_ID EQUAL "GNU")
    target_compile_options(paralelni-algoritmy-i PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif (CMAKE_CXX_COMPILER_ID EQUAL "Clang")
    target_compile_options(paralelni-algoritmy-i PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif (CMAKE_CXX_COMPILER_ID EQUAL "MSVC")
    target_compile_options(paralelni-algoritmy-i PRIVATE /W4 /WX)
endif ()
