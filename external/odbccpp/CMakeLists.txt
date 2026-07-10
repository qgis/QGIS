cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)

project(odbccpp)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_subdirectory(src)
