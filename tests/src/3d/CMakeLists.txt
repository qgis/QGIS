# Standard includes and utils to compile into all tests.
set (util_SRCS)


#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tests/core #for render checker class
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/native
  ${CMAKE_CURRENT_BINARY_DIR}
)
include_directories(SYSTEM
  ${QT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${QT5_3DEXTRA_INCLUDE_DIR}
)

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time
#############################################################
# Tests:

macro (ADD_QGIS_TEST testname testsrc)
  set(qgis_${testname}_SRCS ${testsrc} ${util_SRCS})
  set(qgis_${testname}_MOC_CPPS ${testsrc})
  add_executable(qgis_${testname} ${qgis_${testname}_SRCS})
  # require c++17
  target_compile_features(qgis_${testname} PRIVATE cxx_std_17)
  set_target_properties(qgis_${testname} PROPERTIES AUTORCC TRUE)
  target_link_libraries(qgis_${testname}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    qgis_core
    qgis_3d
    qgis_native)
  add_test(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
  target_link_libraries(qgis_${testname} qgis_native)
  add_test(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname})
  #set_target_properties(qgis_${testname} PROPERTIES
  #  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  #  INSTALL_RPATH_USE_LINK_PATH true )
endmacro (ADD_QGIS_TEST)

add_subdirectory(sandbox)

ADD_QGIS_TEST(3dutilstest testqgs3dutils.cpp)
ADD_QGIS_TEST(3drenderingtest testqgs3drendering.cpp)
ADD_QGIS_TEST(layout3dmaptest testqgslayout3dmap.cpp)
ADD_QGIS_TEST(materialregistrytest testqgsmaterialregistry.cpp)
ADD_QGIS_TEST(3dmaterialtest testqgs3dmaterial.cpp)
ADD_QGIS_TEST(tessellatortest testqgstessellator.cpp)
ADD_QGIS_TEST(3dsymbolregistrytest testqgs3dsymbolregistry.cpp)

