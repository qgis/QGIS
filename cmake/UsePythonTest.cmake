# Add a python test from a python file
# One cannot simply do:
# set(ENV{PYTHONPATH} ${LIBRARY_OUTPUT_PATH})
# set(my_test "from test_mymodule import *\;test_mymodule()")
# add_test(PYTHON-TEST-MYMODULE  python -c ${my_test})
# Since cmake is only transmitting the ADD_TEST line to ctest thus you are losing
# the env var. The only way to store the env var is to physically write in the cmake script
# whatever PYTHONPATH you want and then add the test as 'cmake -P python_test.cmake'
#
# Usage:
# ADD_PYTHON_TEST(PYTHON-TEST test.py)
#
#
#  Copyright (c) 2006-2010 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS
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
    set(QGIS_PYTEST_SEPARATOR ";")
    set(PATH_VAR_NAME "PATH")
    if(USING_NINJA OR USING_NMAKE)
      set(QGIS_PYTEST_PREFIX_PATH "${QGIS_OUTPUT_DIRECTORY}/bin")
      set(QGIS_PYTEST_LIBRARY_PATH "${QGIS_OUTPUT_DIRECTORY}/bin;")
      set(QGIS_PYTEST_PYTHONPATH "${_QGIS_PYTEST_PYTHONPATHS};")
    else()
      set(QGIS_PYTEST_PREFIX_PATH "${QGIS_OUTPUT_DIRECTORY}/bin/${CMAKE_BUILD_TYPE}")
      set(QGIS_PYTEST_LIBRARY_PATH "${QGIS_OUTPUT_DIRECTORY}/bin/${CMAKE_BUILD_TYPE};")
      set(QGIS_PYTEST_PYTHONPATH "${_QGIS_PYTEST_PYTHONPATHS};")
    endif()
  else()
    set(QGIS_PYTEST_SEPARATOR ":")
    set(QGIS_PYTEST_PATH_VAR_NAME "LD_LIBRARY_PATH")
    set(QGIS_PYTEST_PREFIX_PATH "${QGIS_OUTPUT_DIRECTORY}")
    set(QGIS_PYTEST_LIBRARY_PATH "${QGIS_PYTEST_PYENV}${QGIS_OUTPUT_DIRECTORY}/lib")
    string(REPLACE ";" ":" QGIS_PYTEST_PYTHONPATH "${_QGIS_PYTEST_PYTHONPATHS};")

  endif()

  configure_file(${CMAKE_SOURCE_DIR}/cmake_templates/PyQgsTest.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake @ONLY)


  set(TEST_TIMEOUT 0)

  set (PYTHON_TEST_WRAPPER "" CACHE STRING "Wrapper command for python tests (e.g. `timeout -sSIGSEGV 55s` to segfault after 55 seconds)")
  set (PYTHON_TEST_WRAPPER_PROCESSED ${PYTHON_TEST_WRAPPER})
  if (${TEST_TIMEOUT} GREATER 0 AND (NOT ${PYTHON_TEST_WRAPPER} STREQUAL ""))
    string(REGEX REPLACE "timeout -sSIGSEGV ([0-9]+)s" "timeout -sSIGSEGV ${TEST_TIMEOUT}s" PYTHON_TEST_WRAPPER_PROCESSED ${PYTHON_TEST_WRAPPER})
  endif()

  if(CMAKE_CONFIGURATION_TYPES)
    add_test(NAME ${TESTNAME} COMMAND ${CMAKE_COMMAND} -D CMAKE_BUILD_TYPE=$<CONFIGURATION> -P ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake)
  else()
    add_test(NAME ${TESTNAME} COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake)
  endif()

  if (${TEST_TIMEOUT} GREATER 0)
    set_tests_properties(${TESTNAME} PROPERTIES TIMEOUT ${TEST_TIMEOUT})
  endif()

endmacro(ADD_PYTHON_TEST)
