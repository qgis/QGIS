cmake_minimum_required(VERSION 3.5)
project(UntwineApi VERSION 1.0 LANGUAGES CXX)
include(FeatureSummary)
include(${PROJECT_SOURCE_DIR}/../cmake/compiler_options.cmake)

add_executable(qgt
    QgisTest2.cpp
    QgisUntwine.cpp
)

untwine_target_compile_settings(qgt)

