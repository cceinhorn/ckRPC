# Set the minimum required CMake version and project name
cmake_minimum_required(VERSION 3.10)
project(mprpc)

# Build a debug version to enable debugging with gdb
set(CMAKE_BUILD_TYPE "Debug")

# Set the output directory for executable files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
# Set the output directory for library files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# Set the include directories for header file search (-I)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/test)
# Set the library search directories (-L)
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 'src' contains all the related code for the mprpc framework
add_subdirectory(src)
# 'example' contains example code demonstrating how to use the mprpc framework
add_subdirectory(test)
