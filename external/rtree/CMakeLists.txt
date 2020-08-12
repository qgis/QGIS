# CMake version check
cmake_minimum_required(VERSION 3.10.0)

# Check build directory
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Do not build in-source. Please remove CMakeCache.txt and the CMakeFiles/ directory. Then build out-of-source.")
endif()

# RTree project
project(RTree
    VERSION 0.1.0)

# Project build options
option(RTREE_BUILD_TESTS "Build test programs" OFF)

# RTree as header only library
add_library(RTree INTERFACE)
target_include_directories(RTree
    INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_compile_features(RTree
    INTERFACE cxx_std_11)

# Tests
if (RTREE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
