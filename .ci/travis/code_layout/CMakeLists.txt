

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.6)

SET(CMAKE_CXX_STANDARD 11)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../../cmake ${CMAKE_MODULE_PATH})

ADD_SUBDIRECTORY(../../../src/astyle qgisstyle)

ENABLE_TESTING()

ADD_TEST(qgis_indentation ${CMAKE_SOURCE_DIR}/../../../scripts/verify-indentation.sh)
ADD_TEST(qgis_spelling ${CMAKE_SOURCE_DIR}/../../../scripts/spell_check/spell_test.sh)
ADD_TEST(qgis_sipify ${CMAKE_SOURCE_DIR}/../../../tests/scripts/test_sipify.sh)
ADD_TEST(qgis_sip_uptodate ${CMAKE_SOURCE_DIR}/../../../tests/scripts/test_sipfiles_uptodate.sh)

IF (WITH_APIDOC)
  ADD_SUBDIRECTORY(../../../doc doc)

  INCLUDE(CreateQgsVersion)
  CREATE_QGSVERSION()

  set(QGIS_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/doc")

  INCLUDE(UsePythonTest)
  ADD_PYTHON_TEST(PyQgsDocCoverage ${CMAKE_SOURCE_DIR}/../../../tests/src/python/test_qgsdoccoverage.py)
ENDIF (WITH_APIDOC)
