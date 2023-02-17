# Standard includes and utils to compile into all tests.
set (util_SRCS)


#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/test
)

if(HAVE_OPENCL)
    include_directories(SYSTEM ${OpenCL_INCLUDE_DIRS})
endif()

#############################################################
# Tests:
set(TESTS
 testqgsgeometrysnapper.cpp
 testqgsinterpolator.cpp
 testqgsprocessingalgspt1.cpp
 testqgsprocessingalgspt2.cpp
 testqgsprocessingmodelalgorithm.cpp
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
  set(TESTS ${TESTS} testqgsgcptransformer.cpp)
endif()

foreach(TESTSRC ${TESTS})
    add_qgis_test(${TESTSRC} MODULE analysis LINKEDLIBRARIES qgis_analysis)
endforeach(TESTSRC)
add_qgis_test(testqgsprocessing.cpp MODULE analysis LINKEDLIBRARIES qgis_analysis LABELS "POSTGRES")
