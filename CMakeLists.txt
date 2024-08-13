# CMake minimum version requirement
cmake_minimum_required(VERSION 3.10)

# Project name and version
project(CalcLib VERSION 1.0 LANGUAGES C)

# Set the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)

# Library source files
set(LIBRARY_SOURCES calc.c)
set(LIBRARY_HEADERS calc.h calc_internal.h)

# Create the dynamic library
add_library(calc_shared SHARED ${LIBRARY_SOURCES})
set_target_properties(calc_shared PROPERTIES OUTPUT_NAME calc)

# Create the static library
add_library(calc_static STATIC ${LIBRARY_SOURCES})
set_target_properties(calc_static PROPERTIES OUTPUT_NAME calc)

# Add the executable (linked with the dynamic library)
add_executable(calc main.c)
target_link_libraries(calc PRIVATE calc_shared readline)

# Add the static executable (linked with the static library)
add_executable(calc_st main.c)
target_link_libraries(calc_st PRIVATE calc_static readline)

# Installation directives
install(TARGETS calc_shared calc_static DESTINATION ${INSTALL_PATH}/lib)
install(FILES ${LIBRARY_HEADERS} DESTINATION ${INSTALL_PATH}/include)
