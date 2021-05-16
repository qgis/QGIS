# Standard includes and utils to compile into all tests.
set (util_SRCS)


#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/test
)

if(HAVE_OPENCL)
    include_directories(SYSTEM ${OpenCL_INCLUDE_DIRS})
endif()

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

  set(${TESTNAME}_SRCS ${TESTSRC} ${util_SRCS})
  set(${TESTNAME}_MOC_CPPS ${TESTSRC})
  add_executable(${TESTNAME} ${${TESTNAME}_SRCS})
  # require c++17
  target_compile_features(${TESTNAME} PRIVATE cxx_std_17)
  target_link_libraries(${TESTNAME}
    ${Qt5Core_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    qgis_analysis)
  add_test(${TESTNAME} ${CMAKE_BINARY_DIR}/output/bin/${TESTNAME} -maxwarnings 10000)
  #set_target_properties(qgis_${testname} PROPERTIES
  #  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  #  INSTALL_RPATH_USE_LINK_PATH true )
endmacro (ADD_QGIS_TEST)

#############################################################
# Tests:
set(TESTS
 testqgsgeometrysnapper.cpp
 testqgsinterpolator.cpp
 testqgsprocessing.cpp
 testqgsprocessingalgs.cpp
 testqgszonalstatistics.cpp
 testqgsrastercalculator.cpp
 testqgsreclassifyutils.cpp
 testqgsalignraster.cpp
 testqgsnetworkanalysis.cpp
 testqgsninecellfilters.cpp
 testqgsmeshcalculator.cpp
 testqgsmeshcontours.cpp
 testqgstriangulation.cpp
)

if(HAVE_GSL)
  set(TESTS
    ${TESTS}
    testqgsgcptransformer.cpp
    )
endif()

foreach(TESTSRC ${TESTS})
    ADD_QGIS_TEST(${TESTSRC})
endforeach(TESTSRC)

SET_TESTS_PROPERTIES(qgis_processingtest
  PROPERTIES LABELS "POSTGRES")
