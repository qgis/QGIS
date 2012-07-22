# Add a python test from a python file
# One cannot simply do:
# SET(ENV{PYTHONPATH} ${LIBRARY_OUTPUT_PATH})
# SET(my_test "from test_mymodule import *\;test_mymodule()")
# ADD_TEST(PYTHON-TEST-MYMODULE  python -c ${my_test})
# Since cmake is only transmitting the ADD_TEST line to ctest thus you are loosing
# the env var. The only way to store the env var is to physically write in the cmake script
# whatever PYTHONPATH you want and then add the test as 'cmake -P python_test.cmake'
#
# Usage:
# SET_SOURCE_FILES_PROPERTIES(test.py PROPERTIES PYTHONPATH
#   "${LIBRARY_OUTPUT_PATH}:${VTK_DIR}")
# ADD_PYTHON_TEST(PYTHON-TEST test.py)
#
#  Copyright (c) 2006-2010 Mathieu Malaterre <mathieu.malaterre@gmail.com>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS
#

# Need python interpreter:
FIND_PACKAGE(PythonInterp REQUIRED)
MARK_AS_ADVANCED(PYTHON_EXECUTABLE)

MACRO(ADD_PYTHON_TEST TESTNAME FILENAME)
  GET_SOURCE_FILE_PROPERTY(loc ${FILENAME} LOCATION)
  GET_SOURCE_FILE_PROPERTY(pyenv ${FILENAME} PYTHONPATH)
  IF(CMAKE_CONFIGURATION_TYPES)
    # I cannot use CMAKE_CFG_INTDIR since it expand to "$(OutDir)"
    IF(pyenv)
      SET(pyenv "${pyenv};${LIBRARY_OUTPUT_PATH}/${CMAKE_BUILD_TYPE}")
    ELSE(pyenv)
      SET(pyenv ${LIBRARY_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
      #SET(pyenv ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})
      #SET(pyenv ${LIBRARY_OUTPUT_PATH}/${CMAKE_CONFIG_TYPE})
      #SET(pyenv ${LIBRARY_OUTPUT_PATH}/\${CMAKE_CONFIG_TYPE})
    ENDIF(pyenv)
  ELSE(CMAKE_CONFIGURATION_TYPES)
    IF(pyenv)
      SET(pyenv ${pyenv}:${LIBRARY_OUTPUT_PATH})
    ELSE(pyenv)
      SET(pyenv ${LIBRARY_OUTPUT_PATH})
    ENDIF(pyenv)
   ENDIF(CMAKE_CONFIGURATION_TYPES)
  STRING(REGEX REPLACE ";" " " wo_semicolumn "${ARGN}")
  FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake
"
  SET(ENV{PYTHONPATH} ${pyenv}:\$ENV{PYTHONPATH})
  SET(ENV{LD_LIBRARY_PATH} ${pyenv}:\$ENV{LD_LIBRARY_PATH})
  MESSAGE(\"${pyenv}\")
  EXECUTE_PROCESS(
    COMMAND ${PYTHON_EXECUTABLE} ${loc} ${wo_semicolumn}
    #WORKING_DIRECTORY @LIBRARY_OUTPUT_PATH@
    RESULT_VARIABLE import_res
    OUTPUT_VARIABLE import_output
    ERROR_VARIABLE  import_output
    )

  # Pass the output back to ctest
  IF(import_output)
    MESSAGE("\${import_output}")
  ENDIF(import_output)
  IF(import_res)
    MESSAGE(SEND_ERROR "\${import_res}")
  ENDIF(import_res)
"
)
  ADD_TEST(${TESTNAME} ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME}.cmake)
ENDMACRO(ADD_PYTHON_TEST)

# Byte compile recursively a directory (DIRNAME)
MACRO(ADD_PYTHON_COMPILEALL_TEST DIRNAME)
  # First get the path:
  GET_FILENAME_COMPONENT(temp_path "${PYTHON_LIBRARIES}" PATH)
  # Find the python script:
  GET_FILENAME_COMPONENT(PYTHON_COMPILE_ALL_PY "${temp_path}/../compileall.py" ABSOLUTE)
  # add test, use DIRNAME to create uniq name for the test:
  ADD_TEST(COMPILE_ALL-${DIRNAME} ${PYTHON_EXECUTABLE} "${PYTHON_COMPILE_ALL_PY}" -q ${DIRNAME})
ENDMACRO(ADD_PYTHON_COMPILEALL_TEST)
