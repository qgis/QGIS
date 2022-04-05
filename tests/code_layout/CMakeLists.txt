if(WITH_ASTYLE)
  add_test(qgis_indentation ${CMAKE_SOURCE_DIR}/scripts/verify_indentation.sh)
endif()

add_test(qgis_banned_keywords ${CMAKE_SOURCE_DIR}/tests/code_layout/test_banned_keywords.sh)
add_test(qgis_licenses ${CMAKE_SOURCE_DIR}/tests/code_layout/test_licenses.sh)
add_test(qgis_spelling ${CMAKE_SOURCE_DIR}/scripts/spell_check/spell_test.sh)
add_test(qgis_defwindowtitle ${CMAKE_SOURCE_DIR}/tests/code_layout/test_defwindowtitle.sh)
add_test(qgis_qvariant_no_brace_init ${CMAKE_SOURCE_DIR}/tests/code_layout/test_qvariant_no_brace_init.sh)

add_test(qgis_shellcheck ${CMAKE_SOURCE_DIR}/tests/code_layout/test_shellcheck.sh)
add_test(qgis_sipify ${CMAKE_SOURCE_DIR}/tests/code_layout/sipify/test_sipify.sh)
add_test(qgis_sip_include ${CMAKE_SOURCE_DIR}/tests/code_layout/sipify/test_sip_include.sh)
add_test(qgis_sip_uptodate ${CMAKE_SOURCE_DIR}/tests/code_layout/sipify/test_sipfiles.sh)  # spellok

add_test(qgis_doxygen_order ${CMAKE_SOURCE_DIR}/tests/code_layout/test_doxygen_layout.sh)

if (WITH_APIDOC)
  include(UsePythonTest)
  ADD_PYTHON_TEST(PyQgsDocCoverage ${CMAKE_SOURCE_DIR}/tests/code_layout/test_qgsdoccoverage.py)
  if (WITH_CORE)
    ADD_PYTHON_TEST(PyQgsSipCoverage ${CMAKE_SOURCE_DIR}/tests/code_layout/test_qgssipcoverage.py)
  endif()
endif()
