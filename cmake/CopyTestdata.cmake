##
# The ADD_QGIS_TESTDATA macro handles testdata file installation
#
# - at build time it copies files to buildPath/output/testdata/
#
# The SOURCE_PREFIX is the base path of all the files in the source folder (typically ${CMAKE_CURRENT_SOURCE_DIR})
# DEST_FILES is an output variable where a list of all generated files in the build folder is written to. This can be used to define a custom target
# The SOURCE_FILE_PATHS takes a list of files relative to SOURCE_PREFIX
##
MACRO(ADD_QGIS_TESTDATA SOURCE_PREFIX SOURCE_FILE_PATHS DEST_FILES)

# On build copy all resource files to build folder
FOREACH(TESTDATA_FILE ${SOURCE_FILE_PATHS})
  ADD_CUSTOM_COMMAND(
          OUTPUT "${CMAKE_BINARY_DIR}/output/testdata/${TESTDATA_FILE}"
    COMMAND ${CMAKE_COMMAND} -E copy
      "${SOURCE_PREFIX}/testdata/${TESTDATA_FILE}"
      "${CMAKE_BINARY_DIR}/output/testdata/${TESTDATA_FILE}"
    DEPENDS "${SOURCE_PREFIX}/testdata/${TESTDATA_FILE}"
  )
  LIST(APPEND ${DEST_FILES}
          "${CMAKE_BINARY_DIR}/output/testdata/${TESTDATA_FILE}")
ENDFOREACH(TESTDATA_FILE)


ENDMACRO()
