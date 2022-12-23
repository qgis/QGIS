# TESTSRC is relative to ${CMAKE_SOURCE_DIR}
macro (add_qgis_test_script TESTNAME TESTSRC)
  add_test(${TESTNAME} ${CMAKE_SOURCE_DIR}/${TESTSRC})
  set_tests_properties(${TESTNAME} PROPERTIES FIXTURES_REQUIRED SOURCETREE)
endmacro()

if(WITH_ASTYLE)
  add_qgis_test_script(qgis_indentation scripts/verify_indentation.sh)
endif()

add_qgis_test_script(qgis_banned_keywords tests/code_layout/test_banned_keywords.sh)
add_qgis_test_script(qgis_licenses tests/code_layout/test_licenses.sh)
add_qgis_test_script(qgis_spelling scripts/spell_check/spell_test.sh)
add_qgis_test_script(qgis_defwindowtitle tests/code_layout/test_defwindowtitle.sh)
add_qgis_test_script(qgis_qvariant_no_brace_init tests/code_layout/test_qvariant_no_brace_init.sh)

add_qgis_test_script(qgis_shellcheck tests/code_layout/test_shellcheck.sh)
add_qgis_test_script(qgis_sipify tests/code_layout/sipify/test_sipify.sh)
add_qgis_test_script(qgis_sip_include tests/code_layout/sipify/test_sip_include.sh)
add_qgis_test_script(qgis_sip_uptodate tests/code_layout/sipify/test_sipfiles.sh)  # spellok

add_qgis_test_script(qgis_doxygen_order tests/code_layout/test_doxygen_layout.sh)

if (WITH_APIDOC)
  include(UsePythonTest)
  ADD_PYTHON_TEST(PyQgsDocCoverage ${CMAKE_SOURCE_DIR}/tests/code_layout/test_qgsdoccoverage.py)
  if (WITH_CORE)
    ADD_PYTHON_TEST(PyQgsSipCoverage ${CMAKE_SOURCE_DIR}/tests/code_layout/test_qgssipcoverage.py)
  endif()
endif()
