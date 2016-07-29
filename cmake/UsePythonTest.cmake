# Add a python test from a python file # One cannot simply do:
# SET(ENV{PYTHONPATH} ${LIBRARY_OUTPUT_PATH})
# SET(my_test "from test_mymodule import *\;test_mymodule()")
# ADD_TEST(PYTHON-TEST-MYMODULE  python -c ${my_test})
# Since cmake is only transmitting the ADD_TEST line to ctest thus you are loosing
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

# Need python interpreter:
FIND_PACKAGE(PythonInterp ${PYTHON_VER} REQUIRED)
MARK_AS_ADVANCED(PYTHON_EXECUTABLE)

MACRO(ADD_PYTHON_TEST TESTNAME FILENAME)
  GET_SOURCE_FILE_PROPERTY(loc ${FILENAME} LOCATION)
  GET_SOURCE_FILE_PROPERTY(pyenv ${FILENAME} PYTHONPATH)

  IF(WIN32)
    STRING(REGEX REPLACE ":" " " wo_semicolon "${ARGN}")
  ELSE(WIN32)
    STRING(REGEX REPLACE ";" " " wo_semicolon "${ARGN}")
  ENDIF(WIN32)

  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake "
IF(WIN32)
  SET(ENV{QGIS_PREFIX_PATH} \"${QGIS_OUTPUT_DIRECTORY}/bin/\${CMAKE_BUILD_TYPE}\")
  SET(ENV{PATH} \"${QGIS_OUTPUT_DIRECTORY}/bin/\${CMAKE_BUILD_TYPE};\$ENV{PATH}\")
  SET(ENV{PYTHONPATH} \"${QGIS_OUTPUT_DIRECTORY}/python/;${QGIS_OUTPUT_DIRECTORY}/python/plugins;${CMAKE_SOURCE_DIR}/tests/src/python;\$ENV{PYTHONPATH}\")
  MESSAGE(\"PATH:\$ENV{PATH}\")
ELSE(WIN32)
  SET(ENV{QGIS_PREFIX_PATH} \"${QGIS_OUTPUT_DIRECTORY}\")
  SET(ENV{LD_LIBRARY_PATH} \"${pyenv}:${QGIS_OUTPUT_DIRECTORY}/lib:\$ENV{LD_LIBRARY_PATH}\")
  SET(ENV{PYTHONPATH} \"${QGIS_OUTPUT_DIRECTORY}/python/:${QGIS_OUTPUT_DIRECTORY}/python/plugins:${CMAKE_SOURCE_DIR}/tests/src/python:\$ENV{PYTHONPATH}\")
  MESSAGE(\"export LD_LIBRARY_PATH=\$ENV{LD_LIBRARY_PATH}\")
ENDIF(WIN32)
")

  FOREACH(_in ${ARGN})
    STRING(REGEX MATCH "^([^=]+)=(.*)$" _out ${_in})
    MESSAGE(STATUS "ENV: SET(ENV{${CMAKE_MATCH_1}} \"${CMAKE_MATCH_2}\")")
    FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake "
  SET(ENV{${CMAKE_MATCH_1}} \"${CMAKE_MATCH_2}\")
")
  ENDFOREACH(_in)

  FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake "
MESSAGE(\"export PYTHONPATH=\$ENV{PYTHONPATH}\")
MESSAGE(STATUS \"Running ${PYTHON_EXECUTABLE} ${loc} ${wo_semicolon}\")
EXECUTE_PROCESS(
  COMMAND ${PYTHON_EXECUTABLE} ${loc} ${wo_semicolon}
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
ENDMACRO(ADD_PYTHON_TEST)

# Byte compile recursively a directory (DIRNAME)
MACRO(ADD_PYTHON_COMPILEALL_TEST DIRNAME)
  # First get the path:
  GET_FILENAME_COMPONENT(temp_path "${PYTHON_LIBRARIES}" PATH)
  # Find the python script:
  GET_FILENAME_COMPONENT(PYTHON_COMPILE_ALL_PY "${temp_path}/../compileall.py" ABSOLUTE)
  # add test, use DIRNAME to create unique name for the test:
  ADD_TEST(COMPILE_ALL-${DIRNAME} ${PYTHON_EXECUTABLE} "${PYTHON_COMPILE_ALL_PY}" -q ${DIRNAME})
ENDMACRO(ADD_PYTHON_COMPILEALL_TEST)
