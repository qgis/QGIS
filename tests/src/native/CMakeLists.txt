# Standard includes and utils to compile into all tests.

#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_SOURCE_DIR}/src/native
  ${CMAKE_BINARY_DIR}/src/native
)

if(APPLE)
  include_directories(${CMAKE_SOURCE_DIR}/src/native/mac)
endif()

#############################################################
# Compiler defines

# This define is used for tests that need to locate the test
# data under tests/testdata in the qgis source tree.
# the TEST_DATA_DIR variable is set in the top level CMakeLists.txt
add_definitions(-DTEST_DATA_DIR="${TEST_DATA_DIR}")

add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

#############################################################
# libraries

# because of htonl
if (WIN32)
  set(PLATFORM_LIBRARIES wsock32)
endif()

# Since the tests are not actually installed, but rather
# run directly from the build/src/tests dir we need to
# ensure the qgis libs can be found.
if (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
#  set (CMAKE_INSTALL_NAME_DIR @executable_path/../../../src/core)
endif()



#############################################################
# Tests:

if (APPLE)
  add_qgis_test(testqgsmacnative.cpp MODULE native LINKEDLIBRARIES qgis_core qgis_native)
endif()
