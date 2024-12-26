cmake_minimum_required(VERSION 3.30)
project(paralelni_algoritmy_i)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if (CMAKE_CXX_COMPILER_ID EQUAL "GNU")
    target_compile_options(paralelni_algoritmy_i PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif (CMAKE_CXX_COMPILER_ID EQUAL "Clang")
    target_compile_options(paralelni_algoritmy_i PRIVATE -Wall -Wextra -Wpedantic -Werror)
elseif (CMAKE_CXX_COMPILER_ID EQUAL "MSVC")
    target_compile_options(paralelni_algoritmy_i PRIVATE /W4 /WX)
endif ()
