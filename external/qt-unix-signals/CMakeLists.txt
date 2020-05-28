CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)

CMAKE_POLICY (SET CMP0048 NEW)

project(QTSignals VERSION 0.1)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

SET(QTSignal_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "QTSignal Include Dir")

find_package(Qt5Core REQUIRED)

if (Qt5_POSITION_INDEPENDENT_CODE)
  SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif(Qt5_POSITION_INDEPENDENT_CODE)

SET(CMAKE_CXX_STANDARD 11)

option(CMAKE_ENABLE "Enable automatic path configuration" ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

UNSET(CMAKE_CXX_VISIBILITY_PRESET)

include_directories(${Qt5Core_INCLUDE_DIRS})
add_library(QTSignal STATIC sigwatch.cpp)
target_link_libraries (QTSignal ${Qt5Core_LIBRARIES})

#option(WITH_EXAMPLE "Build example executable" ON)
#if (WITH_EXAMPLE)
#  add_executable (sigwatch-demo example.cpp )
#  target_link_libraries (sigwatch-demo
#      QTSignal
#  )
#endif(WITH_EXAMPLE)
