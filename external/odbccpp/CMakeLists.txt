cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)

project(odbccpp)

find_package(ODBC REQUIRED)

add_subdirectory(src)
