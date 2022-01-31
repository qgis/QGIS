include_directories(
  ${CMAKE_SOURCE_DIR}/src/providers/grass
)
include_directories(BEFORE SYSTEM
  ${PROJ_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)
include_directories(SYSTEM
  ${POSTGRES_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
)


macro (ADD_QGIS_GRASS_TEST grass_build_version testname testsrc)
  set(qgis_${testname}${grass_build_version}_SRCS ${testsrc} ${util_SRCS})
  add_custom_target(qgis_${testname}${grass_build_version}moc ALL DEPENDS ${qgis_${testname}${grass_build_version}_MOC_SRCS})
  add_executable(qgis_${testname}${grass_build_version} ${qgis_${testname}${grass_build_version}_SRCS})
  target_compile_features(qgis_${testname}${grass_build_version} PRIVATE cxx_std_17)

  include_directories(
    ${CMAKE_BINARY_DIR}/src/providers/grass/${grass_build_version}
  )

  set_target_properties(qgis_${testname}${grass_build_version} PROPERTIES
    COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX${grass_build_version}}\\\" \"-DGRASS_BUILD_VERSION=${grass_build_version}\""
  )
  target_link_libraries(qgis_${testname}${grass_build_version}
    ${QT_VERSION_BASE}::Xml
    ${QT_VERSION_BASE}::Core
    ${QT_VERSION_BASE}::Svg
    ${QT_VERSION_BASE}::Test
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    qgis_core
    qgisgrass${grass_build_version}
  )
  if(WIN32 AND NOT USING_NMAKE)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${testname}.cmake
"
set(ENV{PATH} \"${GRASS_PREFIX${grass_build_version}}/bin;${GRASS_PREFIX${grass_build_version}}/lib;${CMAKE_BINARY_DIR}/output/bin/\${CMAKE_BUILD_TYPE};${CMAKE_BINARY_DIR}/output/plugins/\${CMAKE_BUILD_TYPE};\$ENV{PATH}\")
set(ENV{GISBASE} \"${GRASS_PREFIX${grass_build_version}}\")
message(STATUS \"Running ${CMAKE_BINARY_DIR}/output/bin/\${CMAKE_BUILD_TYPE}/qgis_${testname}${grass_build_version}\")
message(STATUS \"PATH:\$ENV{PATH}\")
message(STATUS \"GISBASE:\$ENV{GISBASE}\")
execute_process(
  COMMAND ${CMAKE_BINARY_DIR}/output/bin/\${CMAKE_BUILD_TYPE}/qgis_${testname}${grass_build_version}
  RESULT_VARIABLE import_res
)
if(import_res)
  message(FATAL_ERROR \"Test failed: \${import_res}\")
endif()
"
)
    add_test(NAME qgis_${testname}${grass_build_version} COMMAND ${CMAKE_COMMAND} -D CMAKE_BUILD_TYPE=$<CONFIGURATION> -P ${CMAKE_CURRENT_BINARY_DIR}/${testname}.cmake)
  else()
    add_test(qgis_${testname}${grass_build_version} ${CMAKE_BINARY_DIR}/output/bin/qgis_${testname}${grass_build_version} -maxwarnings 10000)
  endif()

  if(WIN32)
    set_property(TEST qgis_${testname}${grass_build_version} PROPERTY ENVIRONMENT "GISBASE=${GRASS_PREFIX${grass_build_version}}")
    if (USING_NMAKE)
      set_property(TEST qgis_${testname}${grass_build_version} PROPERTY
        ENVIRONMENT "PATH=${GRASS_PREFIX${grass_build_version}}/lib;${CMAKE_BINARY_DIR}/output/bin;${CMAKE_BINARY_DIR}/output/plugins;$ENV{PATH}"
      )
    endif()
  else()
    if (APPLE)
      set_property(TEST qgis_${testname}${grass_build_version} PROPERTY
        ENVIRONMENT "DYLD_LIBRARY_PATH=${GRASS_PREFIX${grass_build_version}}/lib:$ENV{DYLD_LIBRARY_PATH}"
      )
    else()
      # UNIX
      set_property(TEST qgis_${testname}${grass_build_version} PROPERTY
        ENVIRONMENT "LD_LIBRARY_PATH=${GRASS_PREFIX${grass_build_version}}/lib:$ENV{LD_LIBRARY_PATH}"
      )
    endif()
  endif()
endmacro (ADD_QGIS_GRASS_TEST)

macro (ADD_QGIS_GRASS_TESTS version)
  ADD_QGIS_GRASS_TEST(${version} grassprovidertest ../testqgsgrassprovider.cpp)
  if(MSVC)
    # 4611 setjmp
    set_source_files_properties(../testqgsgrassprovider.cpp PROPERTIES COMPILE_FLAGS "-wd4611")
  endif()
endmacro (ADD_QGIS_GRASS_TESTS)

foreach(GRASS_BUILD_VERSION 6 7 8)
  if(GRASS_FOUND${GRASS_BUILD_VERSION})
    add_subdirectory(${GRASS_BUILD_VERSION})
  endif()
endforeach(GRASS_BUILD_VERSION 6 7 8)
