IF (ENABLE_TESTS)
  IF (APPLE)
    # override default data path, otherwise looks for Resources in app bundle
    SET (QGIS_DATA_SUBDIR    "${CMAKE_CURRENT_SOURCE_DIR}/resources")
  ENDIF (APPLE)
  ADD_SUBDIRECTORY(src)
  ADD_SUBDIRECTORY(bench)
ENDIF (ENABLE_TESTS)
