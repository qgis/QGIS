if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS_OLD "${CMAKE_CXX_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif()
# override default path where built files are put to allow running qgis without installing
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_PLUGIN_SUBDIR})

add_subdirectory(topology)
add_subdirectory(geometry_checker)

if (WITH_SPATIALITE)
  add_subdirectory(offline_editing)
endif()

if (GRASS_FOUND)
  set (WITH_GRASS_PLUGIN TRUE CACHE BOOL "Determines whether the GRASS c++ plugin should be built")
  if (WITH_GRASS_PLUGIN)
    add_subdirectory(grass)
  endif()
endif()

if (CUSTOM_PLUGINS)
  add_subdirectory(${CUSTOM_PLUGINS})
endif()


# cleanup previously built libraries that do not match the current build version
file(GLOB OLD_PLUGIN_LIBS "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/*.so.*")
foreach(LIB_FILE ${OLD_PLUGIN_LIBS})
  if(NOT LIB_FILE MATCHES ".*${COMPLETE_VERSION}$")
      message(STATUS "Removing obsolete plugin library: ${LIB_FILE}")
      file(REMOVE ${LIB_FILE})
  endif()
endforeach()

# headers installed in qgis_core target
