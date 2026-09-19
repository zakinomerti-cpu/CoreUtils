cmake_minimum_required(VERSION 3.14)
project(noownContainers C)

add_library(noownConts STATIC
    hashArray/pvd_realization.c
    pVoidArray/pVoidArray.c
)

target_compile_features(noownConts PUBLIC c_std_99)
target_include_directories(noownConts PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/pVoidArray/
	${CMAKE_CURRENT_SOURCE_DIR}/hashArray/
)

add_library(noownContainers::noownConts ALIAS noownConts)
