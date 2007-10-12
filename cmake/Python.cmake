
# CMake module which checks for python and some its modules
# there is a two-stage support for python:
# - 


FIND_PACKAGE(PythonLibs) # MapServer export tool
FIND_PACKAGE(PythonInterp) # test for sip and PyQt4


MACRO (TRY_RUN_PYTHON RESULT CMD)
  IF (PYTHONINTERP_FOUND)
    
    EXEC_PROGRAM(${PYTHON_EXECUTABLE} ARGS -c "\"${CMD}\""
                 OUTPUT_VARIABLE out
                 RETURN_VALUE retval)
    
    # optional last parameter to save the output
    SET (OUTPUT ${ARGV2})
    IF (OUTPUT)
      SET(${OUTPUT} ${out})
    ENDIF (OUTPUT)
    
    IF (retval EQUAL 0)
      SET (${RESULT} TRUE)
    ELSE (retval EQUAL 0)
      SET (${RESULT} FALSE)
    ENDIF (retval EQUAL 0)
  
  ELSE (PYTHONINTERP_FOUND)
    SET (${RESULT} FALSE)
  ENDIF (PYTHONINTERP_FOUND)
ENDMACRO (TRY_RUN_PYTHON)

# enable/disable python support (mapserver export tool and bindings)
IF (PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
  SET (PYTHON_FOUND TRUE)
  MESSAGE(STATUS "Python libraries found")

  # TODO: should not be needed, report it to CMake devs
  IF (UNIX AND NOT APPLE)
    SET (PYTHON_LIBRARIES ${PYTHON_LIBRARIES} util)
  ENDIF (UNIX AND NOT APPLE)

ENDIF (PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
