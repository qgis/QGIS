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
  add_subdirectory(grass)
endif()

if (CUSTOM_PLUGINS)
  add_subdirectory(${CUSTOM_PLUGINS})
endif()


# headers installed in qgis_core target
