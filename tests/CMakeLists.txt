IF (ENABLE_TESTS)

  # Install any resoure files needed here...
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/resources/srs.db DESTINATION ${QGIS_OUTPUT_DIRECTORY}/share/qgis/resources/)
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/resources/qgis.db DESTINATION ${QGIS_OUTPUT_DIRECTORY}/share/qgis/resources/)

  IF (APPLE)
    # override default data path, otherwise looks for Resources in app bundle
    SET (QGIS_DATA_SUBDIR    "${CMAKE_SOURCE_DIR}/resources")
  ENDIF (APPLE)
  ADD_SUBDIRECTORY(src)
  ADD_SUBDIRECTORY(bench)
ENDIF (ENABLE_TESTS)
