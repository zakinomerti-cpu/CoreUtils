cmake_minimum_required(VERSION 4.3)
project(CoreUtils C)

set(CMAKE_C_STANDARD 99)

add_library(core STATIC
    hashArray/pvd_realization.c
    memallocate/memallocate.c
    pVoidArray/pVoidArray.c
)

target_include_directories(core PUBLIC stdlib/)

add_executable(CoreUtils main.c)

target_link_libraries(CoreUtils PRIVATE core)