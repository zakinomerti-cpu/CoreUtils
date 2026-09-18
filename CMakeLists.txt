cmake_minimum_required(VERSION 3.14)
project(CoreUtils C)

add_library(core STATIC
    hashArray/pvd_realization.c
    pVoidArray/pVoidArray.c
)

target_compile_features(core PUBLIC c_std_99)
target_include_directories(core PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/pVoidArray/
	${CMAKE_CURRENT_SOURCE_DIR}/hashArray/
)

add_library(CoreUtils::core ALIAS core)
