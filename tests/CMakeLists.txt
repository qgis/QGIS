IF (ENABLE_TESTS)

  #create a variable to specify where our test data is
  #so that unit tests can use TEST_DATA_DIR to locate
  #the test data. See CMakeLists in test dirs for more info
  #TEST_DATA_DIR is also used by QgsRenderChecker currently in core
  SET (TEST_DATA_DIR "${CMAKE_BUILD_DIR}/output/testdata")

  IF (WITH_CORE)
    ADD_SUBDIRECTORY(src)
    ADD_SUBDIRECTORY(bench)
  ENDIF (WITH_CORE)

  # Testdata files to install
  INCLUDE(CopyTestdata)

  FILE(GLOB_RECURSE TESTDATA_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/testdata testdata/*)
  ADD_QGIS_TESTDATA("${CMAKE_CURRENT_SOURCE_DIR}" "${TESTDATA_FILES}" DEST_TESTDATA_FILES)
  ADD_CUSTOM_TARGET(testdata ALL DEPENDS ${DEST_TESTDATA_FILES})


  ADD_SUBDIRECTORY(code_layout)
ENDIF (ENABLE_TESTS)


