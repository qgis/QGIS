cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)

project(odbccpp)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(ODBC REQUIRED)

add_subdirectory(src)

configure_package_config_file(
    cmake/odbccppConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/odbccppConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/odbccpp"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/odbccppConfig.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/odbccpp"
)
