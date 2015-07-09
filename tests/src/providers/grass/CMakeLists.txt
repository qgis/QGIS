ADD_DEFINITIONS("-DGRASS_EXPORT=${DLLIMPORT} -DGRASS_LIB_EXPORT=${DLLIMPORT}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/providers/grass)


MACRO (ADD_QGIS_GRASS_TEST grass_build_version testname testsrc)
  SET(qgis_${testname}${grass_build_version}_SRCS ${testsrc} ${util_SRCS})
  ADD_CUSTOM_TARGET(qgis_${testname}${grass_build_version}moc ALL DEPENDS ${qgis_${testname}${grass_build_version}_MOC_SRCS})
  ADD_EXECUTABLE(qgis_${testname}${grass_build_version} ${qgis_${testname}${grass_build_version}_SRCS})

  SET_TARGET_PROPERTIES(qgis_${testname}${grass_build_version} PROPERTIES
    AUTOMOC TRUE
    COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX${grass_build_version}}\\\" \"-DGRASS_BUILD_VERSION=${grass_build_version}\""
  )
  TARGET_LINK_LIBRARIES(qgis_${testname}${grass_build_version}
    ${QT_QTXML_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSVG_LIBRARY}
    ${QT_QTTEST_LIBRARY}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    qgis_core
    qgisgrass${grass_build_version}
  )
  IF(WIN32 AND NOT USING_NMAKE)
    FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${testname}.cmake
"
SET(ENV{PATH} \"${GRASS_PREFIX${grass_build_version}}/lib;${CMAKE_BINARY_DIR}/output/bin/\${CMAKE_BUILD_TYPE};${CMAKE_BINARY_DIR}/output/plugins/\${CMAKE_BUILD_TYPE};\$ENV{PATH}\")
MESSAGE(STATUS \"Running ${CMAKE_BINARY_DIR}/output/bin/\${CMAKE_BUILD_TYPE}/qgis_${testname}${grass_build_version}\")
EXECUTE_PROCESS(
  COMMAND ${CMAKE_BINARY_DIR}/output/bin/\${CMAKE_BUILD_TYPE}/qgis_${testname}${grass_build_version}
  RESULT_VARIABLE import_res
  OUTPUT_VARIABLE import_output
  ERROR_VARIABLE  import_output
)
# Pass the output back to ctest
IF(import_output)
  MESSAGE(" \${import_output} ")
ENDIF(import_output)
IF(import_res)
  MESSAGE(SEND_ERROR " \${import_res} ")
ENDIF(import_res)
"
)
    ADD_TEST(NAME qgis_${testname}${grass_build_version} COMMAND ${CMAKE_COMMAND} -D CMAKE_BUILD_TYPE=$<CONFIGURATION> -P ${CMAKE_CURRENT_BINARY_DIR}/${testname}.cmake)
  ELSE (WIN32 AND NOT USING_NMAKE)
    ADD_TEST(qgis_${testname}${grass_build_version} ${CMAKE_BINARY_DIR}/output/bin/qgis_${testname}${grass_build_version} -maxwarnings 10000)
  ENDIF (WIN32 AND NOT USING_NMAKE)

  IF(WIN32)
    SET_PROPERTY(TEST qgis_${testname}${grass_build_version} PROPERTY ENVIRONMENT "GISBASE=${GRASS_PREFIX${grass_build_version}}")
    IF (USING_NMAKE)
      SET_PROPERTY(TEST qgis_${testname}${grass_build_version} PROPERTY
        ENVIRONMENT "PATH=${GRASS_PREFIX${grass_build_version}}/lib;${CMAKE_BINARY_DIR}/output/bin;${CMAKE_BINARY_DIR}/output/plugins;$ENV{PATH}"
      )
    ENDIF (USING_NMAKE)
  ELSE(WIN32)
    IF (APPLE)
      SET_PROPERTY(TEST qgis_${testname}${grass_build_version} PROPERTY
        ENVIRONMENT "DYLD_LIBRARY_PATH=${GRASS_PREFIX${grass_build_version}}/lib:$ENV{DYLD_LIBRARY_PATH}"
      )
    ELSE (APPLE)
      # UNIX
      SET_PROPERTY(TEST qgis_${testname}${grass_build_version} PROPERTY
        ENVIRONMENT "LD_LIBRARY_PATH=${GRASS_PREFIX${grass_build_version}}/lib:$ENV{LD_LIBRARY_PATH}"
      )
    ENDIF (APPLE)
  ENDIF (WIN32)
ENDMACRO (ADD_QGIS_GRASS_TEST)

MACRO (ADD_QGIS_GRASS_TESTS version)
    ADD_QGIS_GRASS_TEST(${version} grassprovidertest ../testqgsgrassprovider.cpp)
ENDMACRO (ADD_QGIS_GRASS_TESTS)

FOREACH(GRASS_BUILD_VERSION 6 7)
  IF(GRASS_FOUND${GRASS_BUILD_VERSION})
    ADD_SUBDIRECTORY(${GRASS_BUILD_VERSION})
  ENDIF(GRASS_FOUND${GRASS_BUILD_VERSION})
ENDFOREACH(GRASS_BUILD_VERSION 6 7)
