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

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree

macro (ADD_QGIS_TEST testname testsrc)
  set(qgis_${testname}_SRCS ${testsrc} ${util_SRCS})
  set(qgis_${testname}_MOC_CPPS ${testsrc})
  add_executable(qgis_${testname} ${qgis_${testname}_SRCS})
  target_compile_features(qgis_${testname} PRIVATE cxx_std_17)
  target_link_libraries(qgis_${testname}
    ${Qt5Core_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    qgis_core
    qgis_native
  )
  add_test(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
  #set_target_properties(qgis_${testname} PROPERTIES
  #  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  #  INSTALL_RPATH_USE_LINK_PATH true )
endmacro (ADD_QGIS_TEST)



#############################################################
# Tests:

if (APPLE)
  ADD_QGIS_TEST(macnativetest testqgsmacnative.cpp)
endif()
