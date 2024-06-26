# Add a python test from a python file
#
# Since cmake is only transmitting the ADD_TEST line to ctest thus you are losing
# the env var. The only way to store the env var is to physically write in the cmake script
# whatever PYTHONPATH or variable you want and then add the test as 'cmake -P python_test.cmake'
#
# Usage:
# ADD_PYTHON_TEST(PYTHON-TEST test.py)
#
# Optionally pass environment variables to your test
# ADD_PYTHON_TEST(PYTHON-TEST test.py FOO="bar" BAZ="quux")
#
# Timeout can be set by doing ADD_PYTHON_TEST(PYTHON-TEST test.py TIMEOUT=10)
#
#
#  Copyright (c) 2006-2010 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#


macro(ADD_PYTHON_TEST TESTNAME FILENAME)
  get_source_file_property(QGIS_PYTEST_FILE_LOC ${FILENAME} LOCATION)
  get_source_file_property(QGIS_PYTEST_PYENV ${FILENAME} PYTHONPATH)

  #Avoid "NOTFOUND" string when setting LD_LIBRARY_PATH later
  if(EXISTS "${QGIS_PYTEST_PYENV}")
    set(QGIS_PYTEST_PYENV "${QGIS_PYTEST_PYENV}:")
  else()
    set(QGIS_PYTEST_PYENV "")
  endif()

  set(_QGIS_PYTEST_PYTHONPATHS "${PYTHON_OUTPUT_DIRECTORY}" "${PYTHON_OUTPUT_DIRECTORY}/plugins" "${CMAKE_SOURCE_DIR}/tests/src/python")

  if(WIN32)
    if(NOT CMAKE_CONFIGURATION_TYPES)
      set(QGIS_PYTEST_PREFIX_PATH "${QGIS_OUTPUT_DIRECTORY}/bin")
      set(QGIS_PYTEST_LIBRARY_PATH "${QGIS_OUTPUT_DIRECTORY}/bin")
    else()
      set(QGIS_PYTEST_PREFIX_PATH "${QGIS_OUTPUT_DIRECTORY}/bin/$<CONFIG>")
      set(QGIS_PYTEST_LIBRARY_PATH "${QGIS_OUTPUT_DIRECTORY}/bin/$<CONFIG>")
    endif()
    set(QGIS_PYTEST_PYTHONPATH "${_QGIS_PYTEST_PYTHONPATHS}")
  else()
    set(QGIS_PYTEST_PREFIX_PATH "${QGIS_OUTPUT_DIRECTORY}")
    set(QGIS_PYTEST_LIBRARY_PATH "${QGIS_PYTEST_PYENV}${QGIS_OUTPUT_DIRECTORY}/lib")
    string(REPLACE ";" ":" QGIS_PYTEST_PYTHONPATH "${_QGIS_PYTEST_PYTHONPATHS}")
  endif()

  set(TEST_TIMEOUT 0)
  set(QGIS_PYTEST_ADDITIONAL_ENV_VARS "")
  foreach(_in ${ARGN})
    string(REGEX MATCH "^([^=]+)=(.*)$" _out ${_in})
    if("${CMAKE_MATCH_1}" STREQUAL "TEST_TIMEOUT")
      set(TEST_TIMEOUT ${CMAKE_MATCH_2})
    else()
      set(QGIS_PYTEST_ADDITIONAL_ENV_VARS "${QGIS_PYTEST_ADDITIONAL_ENV_VARS}\nset(ENV{${CMAKE_MATCH_1}} \"${CMAKE_MATCH_2}\")")
    endif()
  endforeach(_in)

  set (PYTHON_TEST_WRAPPER "" CACHE STRING "Wrapper command for python tests (e.g. `timeout -sSIGSEGV 55s` to segfault after 55 seconds)")
  set (PYTHON_TEST_WRAPPER_PROCESSED ${PYTHON_TEST_WRAPPER})
  if (${TEST_TIMEOUT} GREATER 0 AND (NOT ${PYTHON_TEST_WRAPPER} STREQUAL ""))
    string(REGEX REPLACE "timeout -sSIGSEGV ([0-9]+)s" "timeout -sSIGSEGV ${TEST_TIMEOUT}s" PYTHON_TEST_WRAPPER_PROCESSED ${PYTHON_TEST_WRAPPER})
  endif()

  configure_file(${CMAKE_SOURCE_DIR}/cmake_templates/PyQgsTest.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake @ONLY NEWLINE_STYLE "LF")

  if(CMAKE_CONFIGURATION_TYPES)
    add_test(NAME ${TESTNAME} COMMAND ${CMAKE_COMMAND} -D CMAKE_BUILD_TYPE=$<CONFIG> -P ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake)
  else()
    add_test(NAME ${TESTNAME} COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake)
  endif()

  if (${TEST_TIMEOUT} GREATER 0)
    set_tests_properties(${TESTNAME} PROPERTIES TIMEOUT ${TEST_TIMEOUT})
  endif()

  set_tests_properties(${TESTNAME} PROPERTIES FIXTURES_REQUIRED SOURCETREE)

endmacro(ADD_PYTHON_TEST)
