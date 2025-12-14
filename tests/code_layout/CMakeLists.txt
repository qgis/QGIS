# TESTSRC is relative to ${CMAKE_SOURCE_DIR}
macro (add_qgis_test_script TESTNAME TESTSRC)
  add_test(${TESTNAME} ${CMAKE_SOURCE_DIR}/${TESTSRC})
  set_tests_properties(${TESTNAME} PROPERTIES FIXTURES_REQUIRED SOURCETREE)
endmacro()

add_qgis_test_script(qgis_class_names tests/code_layout/test_class_names.sh)
add_qgis_test_script(qgis_licenses tests/code_layout/test_licenses.sh)
add_qgis_test_script(qgis_defwindowtitle tests/code_layout/test_defwindowtitle.sh)
add_qgis_test_script(qgis_qvariant_no_brace_init tests/code_layout/test_qvariant_no_brace_init.sh)

add_qgis_test_script(qgis_sip_include tests/code_layout/sipify/test_sip_include.sh)
add_qgis_test_script(qgis_sip_uptodate tests/code_layout/sipify/test_sipfiles.sh)  # spellok

if (WITH_APIDOC)
  include(UsePythonTest)
  ADD_PYTHON_TEST(PyQgsDocCoverage ${CMAKE_SOURCE_DIR}/tests/code_layout/test_qgsdoccoverage.py)
  if (WITH_CORE)
    ADD_PYTHON_TEST(PyQgsSipCoverage ${CMAKE_SOURCE_DIR}/tests/code_layout/test_qgssipcoverage.py)
  endif()
endif()
