# Add a python test from a python file # One cannot simply do:
# SET(ENV{PYTHONPATH} ${LIBRARY_OUTPUT_PATH})
# SET(my_test "from test_mymodule import *\;test_mymodule()")
# ADD_TEST(PYTHON-TEST-MYMODULE  python -c ${my_test})
# Since cmake is only transmitting the ADD_TEST line to ctest thus you are losing
# the env var. The only way to store the env var is to physically write in the cmake script
# whatever PYTHONPATH you want and then add the test as 'cmake -P python_test.cmake'
#
# Usage:
# ADD_PYTHON_TEST(PYTHON-TEST test.py)
#
# Optionally pass environment variables to your test
# ADD_PYTHON_TEST(PYTHON-TEST test.py FOO="bar" BAZ="quux")
#
#  Copyright (c) 2006-2010 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS
#


MACRO(ADD_PYTHON_TEST TESTNAME FILENAME)
  GET_SOURCE_FILE_PROPERTY(loc ${FILENAME} LOCATION)
  GET_SOURCE_FILE_PROPERTY(pyenv ${FILENAME} PYTHONPATH)
  #Avoid "NOTFOUND" string when setting LD_LIBRARY_PATH later
  if(EXISTS "${pyenv}")
    set(pyenv "${pyenv}:")
  else()
    set(pyenv "")
  endif()
  IF(WIN32)
    STRING(REGEX REPLACE ":" " " wo_semicolon "${ARGN}")
    IF(USING_NINJA OR USING_NMAKE)
      FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake "
SET(ENV{QGIS_PREFIX_PATH} \"${QGIS_OUTPUT_DIRECTORY}/bin\")
SET(ENV{PATH} \"${QGIS_OUTPUT_DIRECTORY}/bin;\$ENV{PATH}\")
SET(ENV{PYTHONPATH} \"${PYTHON_OUTPUT_DIRECTORY};${PYTHON_OUTPUT_DIRECTORY}/plugins;${CMAKE_SOURCE_DIR}/tests/src/python;\$ENV{PYTHONPATH}\")
MESSAGE(\"PATH:\$ENV{PATH}\")
")
    ELSE(USING_NINJA OR USING_NMAKE)
      FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake "
SET(ENV{QGIS_PREFIX_PATH} \"${QGIS_OUTPUT_DIRECTORY}/bin/\${CMAKE_BUILD_TYPE}\")
SET(ENV{PATH} \"${QGIS_OUTPUT_DIRECTORY}/bin/\${CMAKE_BUILD_TYPE};\$ENV{PATH}\")
SET(ENV{PYTHONPATH} \"${PYTHON_OUTPUT_DIRECTORY};${PYTHON_OUTPUT_DIRECTORY}/plugins;${CMAKE_SOURCE_DIR}/tests/src/python;\$ENV{PYTHONPATH}\")
MESSAGE(\"PATH:\$ENV{PATH}\")
")
    ENDIF(USING_NINJA OR USING_NMAKE)
  ELSE(WIN32)
    STRING(REGEX REPLACE ";" " " wo_semicolon "${ARGN}")
    FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake "
SET(ENV{QGIS_PREFIX_PATH} \"${QGIS_OUTPUT_DIRECTORY}\")
SET(ENV{LD_LIBRARY_PATH} \"${pyenv}${QGIS_OUTPUT_DIRECTORY}/lib:\$ENV{LD_LIBRARY_PATH}\")
SET(ENV{PYTHONPATH} \"${PYTHON_OUTPUT_DIRECTORY}:${PYTHON_OUTPUT_DIRECTORY}/plugins:${CMAKE_SOURCE_DIR}/tests/src/python:\$ENV{PYTHONPATH}\")
MESSAGE(\"export LD_LIBRARY_PATH=\$ENV{LD_LIBRARY_PATH}\")
")
  ENDIF(WIN32)

  SET(TEST_TIMEOUT 0)

  FOREACH(_in ${ARGN})
    STRING(REGEX MATCH "^([^=]+)=(.*)$" _out ${_in})

    IF("${CMAKE_MATCH_1}" STREQUAL "TEST_TIMEOUT")
      SET(TEST_TIMEOUT ${CMAKE_MATCH_2})
      # Remove TEST_TIMEOUT=VALUE from the list of optional parameters
      STRING(REPLACE "${CMAKE_MATCH_1}=${CMAKE_MATCH_2}" "" wo_semicolon ${wo_semicolon})
    ELSE()
      MESSAGE(STATUS "ENV: SET(ENV{${CMAKE_MATCH_1}} \"${CMAKE_MATCH_2}\")")
      FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake "
      SET(ENV{${CMAKE_MATCH_1}} \"${CMAKE_MATCH_2}\")
")
    ENDIF()
  ENDFOREACH(_in)

  SET (PYTHON_TEST_WRAPPER "" CACHE STRING "Wrapper command for python tests (e.g. `timeout -sSIGSEGV 55s` to segfault after 55 seconds)")
  SET (PYTHON_TEST_WRAPPER_PROCESSED ${PYTHON_TEST_WRAPPER})
  IF (${TEST_TIMEOUT} GREATER 0 AND (NOT ${PYTHON_TEST_WRAPPER} STREQUAL ""))
    STRING(REGEX REPLACE "timeout -sSIGSEGV ([0-9]+)s" "timeout -sSIGSEGV ${TEST_TIMEOUT}s" PYTHON_TEST_WRAPPER_PROCESSED ${PYTHON_TEST_WRAPPER})
  ENDIF()

  FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake "
MESSAGE(\"export PYTHONPATH=\$ENV{PYTHONPATH}\")
MESSAGE(STATUS \"Running ${PYTHON_TEST_WRAPPER_PROCESSED} ${Python_EXECUTABLE} ${loc} ${wo_semicolon}\")
EXECUTE_PROCESS(
  COMMAND ${PYTHON_TEST_WRAPPER_PROCESSED} ${Python_EXECUTABLE} ${loc} ${wo_semicolon}
  RESULT_VARIABLE import_res
)
# Pass the output back to ctest
IF(import_res)
  MESSAGE(FATAL_ERROR \"Test failed: \${import_res}\")
ENDIF(import_res)
"
)
  IF(CMAKE_CONFIGURATION_TYPES)
    ADD_TEST(NAME ${TESTNAME} COMMAND ${CMAKE_COMMAND} -D CMAKE_BUILD_TYPE=$<CONFIGURATION> -P ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake)
  ELSE(CMAKE_CONFIGURATION_TYPES)
    ADD_TEST(NAME ${TESTNAME} COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake)
  ENDIF(CMAKE_CONFIGURATION_TYPES)

  IF (${TEST_TIMEOUT} GREATER 0)
    SET_TESTS_PROPERTIES(${TESTNAME} PROPERTIES TIMEOUT ${TEST_TIMEOUT})
  ENDIF()

ENDMACRO(ADD_PYTHON_TEST)

# Byte compile recursively a directory (DIRNAME)
MACRO(ADD_PYTHON_COMPILEALL_TEST DIRNAME)
  # First get the path:
  GET_FILENAME_COMPONENT(temp_path "${PYTHON_LIBRARIES}" PATH)
  # Find the python script:
  GET_FILENAME_COMPONENT(PYTHON_COMPILE_ALL_PY "${temp_path}/../compileall.py" ABSOLUTE)
  # add test, use DIRNAME to create unique name for the test:
  ADD_TEST(COMPILE_ALL-${DIRNAME} ${Python_EXECUTABLE} "${PYTHON_COMPILE_ALL_PY}" -q ${DIRNAME})
ENDMACRO(ADD_PYTHON_COMPILEALL_TEST)
