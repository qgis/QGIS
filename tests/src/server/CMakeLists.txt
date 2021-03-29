if(NOT MSVC)
add_subdirectory(wms)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/server
  ${CMAKE_SOURCE_DIR}/src/test

  ${CMAKE_BINARY_DIR}/src/server
)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time

#No relinking and full RPATH for the install tree
#See: http://www.cmake.org/Wiki/CMake_RPATH_handling#No_relinking_and_full_RPATH_for_the_install_tree

macro (ADD_QGIS_TEST TESTSRC)
  set (TESTNAME  ${TESTSRC})
  string(REPLACE "test" "" TESTNAME ${TESTNAME})
  string(REPLACE "qgs" "" TESTNAME ${TESTNAME})
  string(REPLACE ".cpp" "" TESTNAME ${TESTNAME})
  set (TESTNAME  "qgis_${TESTNAME}test")
  add_executable(${TESTNAME} ${TESTSRC} ${util_SRCS})
  target_compile_features(${TESTNAME} PRIVATE cxx_std_17)
  target_link_libraries(${TESTNAME}
    ${Qt5Core_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    qgis_server)
  add_test(${TESTNAME} ${CMAKE_BINARY_DIR}/output/bin/${TESTNAME} -maxwarnings 10000)
endmacro (ADD_QGIS_TEST)

#############################################################
# Tests:

set(TESTS
  testqgsserverquerystringparameter.cpp
)

foreach(TESTSRC ${TESTS})
    ADD_QGIS_TEST(${TESTSRC})
endforeach(TESTSRC)
