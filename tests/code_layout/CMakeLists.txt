IF(WITH_ASTYLE)
  ADD_TEST(qgis_indentation ${CMAKE_SOURCE_DIR}/scripts/verify-indentation.sh)
ENDIF(WITH_ASTYLE)

ADD_TEST(qgis_banned_keywords ${CMAKE_SOURCE_DIR}/tests/code_layout/test_banned_keywords.sh)
ADD_TEST(qgis_licenses ${CMAKE_SOURCE_DIR}/tests/code_layout/test_licenses.sh)
ADD_TEST(qgis_spelling ${CMAKE_SOURCE_DIR}/scripts/spell_check/spell_test.sh)

ADD_TEST(qgis_shellcheck ${CMAKE_SOURCE_DIR}/tests/code_layout/test_shellcheck.sh)
ADD_TEST(qgis_sipify ${CMAKE_SOURCE_DIR}/tests/code_layout/test_sipify.sh)
ADD_TEST(qgis_sip_include ${CMAKE_SOURCE_DIR}/tests/code_layout/test_sip_include.sh)
ADD_TEST(qgis_sip_uptodate ${CMAKE_SOURCE_DIR}/tests/code_layout/test_sipfiles_uptodate.sh)

ADD_TEST(qgis_doxygen_order ${CMAKE_SOURCE_DIR}/tests/code_layout/test_doxygen_layout.sh)

IF (WITH_APIDOC)
  INCLUDE(UsePythonTest)
  ADD_PYTHON_TEST(PyQgsDocCoverage ${CMAKE_SOURCE_DIR}/tests/code_layout/test_qgsdoccoverage.py)
  IF (WITH_CORE)
    ADD_PYTHON_TEST(PyQgsSipCoverage ${CMAKE_SOURCE_DIR}/tests/code_layout/test_qgssipcoverage.py)
  ENDIF(WITH_CORE)
ENDIF (WITH_APIDOC)
