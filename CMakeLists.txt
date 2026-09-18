cmake_minimum_required(VERSION 3.5)
project(CoreUtils C)

set(CMAKE_C_STANDARD 99)

add_library(core STATIC
    hashArray/pvd_realization.c
    pVoidArray/pVoidArray.c
)

add_executable(CoreUtils main.c)

target_include_directories(core PUBLIC
	${CMAKE_SOURCE_DIR}/pVoidArray/
)
target_link_libraries(CoreUtils PRIVATE core)
