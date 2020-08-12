##
# The ADD_QGIS_RESOURCES macro handles resource file installation
#
# - at build time it copies files to buildPath/output/data
# - at install time it installs the files into the qgis data folder (typically INSTALL_PREFIX/share/qgis)
#
# The SOURCE_PREFIX is the base path of all the files in the source folder (typically ${CMAKE_CURRENT_SOURCE_DIR})
# The TARGET_PREFIX is the subdirectory inside the data folder where files should be placed
# DEST_FILES is an output variable where a list of all generated files in the build folder is written to. This can be used to define a custom target
# The SOURCE_FILE_PATHS takes a list of files relative to SOURCE_PREFIX
##
MACRO(ADD_QGIS_RESOURCES SOURCE_PREFIX TARGET_PREFIX DEST_FILES SOURCE_FILE_PATHS)

# On build copy all resource files to build folder
FOREACH(RESOURCE_FILE ${SOURCE_FILE_PATHS})
  ADD_CUSTOM_COMMAND(
          OUTPUT "${CMAKE_BINARY_DIR}/output/data/${TARGET_PREFIX}/${RESOURCE_FILE}"
    COMMAND ${CMAKE_COMMAND} -E copy
      "${SOURCE_PREFIX}/${RESOURCE_FILE}"
      "${CMAKE_BINARY_DIR}/output/data/${TARGET_PREFIX}/${RESOURCE_FILE}"
    DEPENDS "${SOURCE_PREFIX}/${RESOURCE_FILE}"
  )
  LIST(APPEND ${DEST_FILES}
          "${CMAKE_BINARY_DIR}/output/data/${TARGET_PREFIX}/${RESOURCE_FILE}")
ENDFOREACH(RESOURCE_FILE)

# Install resources to system resource folder
FOREACH(RESOURCE_FILE ${SOURCE_FILE_PATHS})
  GET_FILENAME_COMPONENT(PATH_NAME "${TARGET_PREFIX}/${RESOURCE_FILE}" PATH)
  INSTALL(FILES "${SOURCE_PREFIX}/${RESOURCE_FILE}" DESTINATION "${QGIS_DATA_DIR}/${PATH_NAME}")
ENDFOREACH()

ENDMACRO()
