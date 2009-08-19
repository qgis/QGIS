
# CMake module which checks for python and some its modules
# there is a two-stage support for python:
# - 


FIND_PACKAGE(PythonLibs) # MapServer export tool
FIND_PACKAGE(PythonInterp) # test for sip and PyQt4

IF(NOT PYUIC4_PROGRAM)
  IF (MSVC)
    FIND_PROGRAM(PYUIC4_PROGRAM
      NAMES pyuic4.bat
      PATHS $ENV{LIB_DIR}/bin
    )
  ELSE(MSVC)
    FIND_PROGRAM(PYUIC4_PROGRAM pyuic4)
  ENDIF (MSVC)

  IF (NOT PYUIC4_PROGRAM)
    MESSAGE(FATAL_ERROR "pyuic4 not found - aborting")
  ENDIF (NOT PYUIC4_PROGRAM)
ENDIF(NOT PYUIC4_PROGRAM)

# Adapted from QT4_WRAP_UI
MACRO(PYQT4_WRAP_UI outfiles )
  FOREACH(it ${ARGN})
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/ui_${outfile}.py)
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${PYUIC4_PROGRAM} ${infile} -o ${outfile}
      MAIN_DEPENDENCY ${infile}                       
    )
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH(it)
ENDMACRO(PYQT4_WRAP_UI)

IF(NOT PYRCC4_PROGRAM)
  IF (MSVC)
    FIND_PROGRAM(PYRCC4_PROGRAM
      NAMES pyrcc4.exe
      PATHS $ENV{LIB_DIR}/bin
    )
  ELSE(MSVC)
    FIND_PROGRAM(PYRCC4_PROGRAM pyrcc4)
  ENDIF (MSVC)

  IF (NOT PYRCC4_PROGRAM)
    MESSAGE(FATAL_ERROR "pyrcc4 not found - aborting")
  ENDIF (NOT PYRCC4_PROGRAM)
ENDIF(NOT PYRCC4_PROGRAM)

# Adapted from QT4_ADD_RESOURCES
MACRO (PYQT4_ADD_RESOURCES outfiles )
  FOREACH (it ${ARGN})
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    GET_FILENAME_COMPONENT(rc_path ${infile} PATH)
    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}_rc.py)
    #  parse file for dependencies 
    #  all files are absolute paths or relative to the location of the qrc file
    FILE(READ "${infile}" _RC_FILE_CONTENTS)
    STRING(REGEX MATCHALL "<file[^<]+" _RC_FILES "${_RC_FILE_CONTENTS}")
    SET(_RC_DEPENDS)
    FOREACH(_RC_FILE ${_RC_FILES})
      STRING(REGEX REPLACE "^<file[^>]*>" "" _RC_FILE "${_RC_FILE}")
      STRING(REGEX MATCH "^/|([A-Za-z]:/)" _ABS_PATH_INDICATOR "${_RC_FILE}")
      IF(NOT _ABS_PATH_INDICATOR)
        SET(_RC_FILE "${rc_path}/${_RC_FILE}")
      ENDIF(NOT _ABS_PATH_INDICATOR)
      SET(_RC_DEPENDS ${_RC_DEPENDS} "${_RC_FILE}")
    ENDFOREACH(_RC_FILE)
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${PYRCC4_PROGRAM} -name ${outfile} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile}
      DEPENDS ${_RC_DEPENDS})
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH (it)
ENDMACRO (PYQT4_ADD_RESOURCES)

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

IF(MSVC)
  FIND_PROGRAM(SIP_MAKE_PROGRAM
                NAMES nmake.exe
                PATHS "$ENV{VCINSTALLDIR}/bin" "$ENV{PROGRAMFILES}/Microsoft Visual 9.0/VC/bin"
        )
  IF(NOT SIP_MAKE_PROGRAM)
    MESSAGE(FATAL_ERROR "nmake not found")
  ENDIF(NOT SIP_MAKE_PROGRAM)
ELSE (MSVC)
  SET(SIP_MAKE_PROGRAM ${CMAKE_MAKE_PROGRAM})
ENDIF (MSVC)


# enable/disable python support (mapserver export tool and bindings)
IF (PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
  SET (PYTHON_FOUND TRUE)
  MESSAGE(STATUS "Python libraries found")

  # TODO: should not be needed, report it to CMake devs
  IF (UNIX AND NOT APPLE)
    SET (PYTHON_LIBRARIES ${PYTHON_LIBRARIES} util)
  ENDIF (UNIX AND NOT APPLE)
  
  IF (WITH_BINDINGS)
    
    # check for SIP (3 steps)
    # 1. can import python module?
    TRY_RUN_PYTHON (HAVE_SIP_MODULE "from sip import wrapinstance")

    IF (APPLE)
      SET (SIP_MAC_PATH
      /System/Library/Frameworks/Python.framework/Versions/2.5/bin
      /System/Library/Frameworks/Python.framework/Versions/2.4/bin
      /System/Library/Frameworks/Python.framework/Versions/2.3/bin)
    ENDIF (APPLE)

    # 2. is there sip binary? (for creating wrappers)
    FIND_PROGRAM (SIP_BINARY_PATH sip PATHS ${SIP_MAC_PATH})
    
    # 3. is there sip include file? (necessary for compilation of bindings)
    FIND_PATH (SIP_INCLUDE_DIR sip.h ${PYTHON_INCLUDE_PATH})
    
    IF (HAVE_SIP_MODULE AND SIP_BINARY_PATH AND SIP_INCLUDE_DIR)
      # check for SIP version
      # minimal version is 4.7 (to support universal builds)
      SET (SIP_MIN_VERSION 040700)
      TRY_RUN_PYTHON (RES "import sip\nprint '%x' % sip.SIP_VERSION" SIP_VERSION)
      IF (SIP_VERSION EQUAL "${SIP_MIN_VERSION}" OR SIP_VERSION GREATER "${SIP_MIN_VERSION}")
        SET (SIP_IS_GOOD TRUE)
      ENDIF (SIP_VERSION EQUAL "${SIP_MIN_VERSION}" OR SIP_VERSION GREATER "${SIP_MIN_VERSION}")
    
      IF (NOT SIP_IS_GOOD)
        MESSAGE (STATUS "SIP is required in version 4.7 or later!")
      ENDIF (NOT SIP_IS_GOOD)
    ELSE (HAVE_SIP_MODULE AND SIP_BINARY_PATH AND SIP_INCLUDE_DIR)
      IF (NOT HAVE_SIP_MODULE)
        MESSAGE (STATUS "SIP python module is missing!")
      ENDIF (NOT HAVE_SIP_MODULE)
      IF (NOT SIP_BINARY_PATH)
        MESSAGE (STATUS "SIP executable is missing!")
      ENDIF (NOT SIP_BINARY_PATH)
      IF (NOT SIP_INCLUDE_DIR)
        MESSAGE (STATUS "SIP header file is missing!")
      ENDIF (NOT SIP_INCLUDE_DIR)
    ENDIF (HAVE_SIP_MODULE AND SIP_BINARY_PATH AND SIP_INCLUDE_DIR)
     
    # check for PyQt4
    TRY_RUN_PYTHON (HAVE_PYQT4 "from PyQt4 import QtCore, QtGui, QtNetwork, QtSvg, QtXml")
    
    # check whether directory with PyQt4 sip files exists
    IF (HAVE_PYQT4)
      TRY_RUN_PYTHON (RES "import PyQt4.pyqtconfig\nprint PyQt4.pyqtconfig._pkg_config['pyqt_sip_dir']" PYQT_SIP_DIR)
      IF (IS_DIRECTORY ${PYQT_SIP_DIR})
        SET (HAVE_PYQT4_SIP_DIR TRUE)
      ENDIF (IS_DIRECTORY ${PYQT_SIP_DIR})
    ENDIF (HAVE_PYQT4)
    
    IF (HAVE_PYQT4 AND HAVE_PYQT4_SIP_DIR)
      # check for PyQt4 version
      # minimal version is 4.1
      SET (PYQT_MIN_VERSION 040100)
      TRY_RUN_PYTHON (RES "from PyQt4 import QtCore\nprint '%x' % QtCore.PYQT_VERSION" PYQT_VERSION)
      IF (PYQT_VERSION EQUAL "${PYQT_MIN_VERSION}" OR PYQT_VERSION GREATER "${PYQT_MIN_VERSION}")
        SET (PYQT_IS_GOOD TRUE)
      ENDIF (PYQT_VERSION EQUAL "${PYQT_MIN_VERSION}" OR PYQT_VERSION GREATER "${PYQT_MIN_VERSION}")
    
      IF (NOT PYQT_IS_GOOD)
        MESSAGE (STATUS "PyQt4 is needed in version 4.1 or later!")
      ENDIF (NOT PYQT_IS_GOOD)
    ELSE (HAVE_PYQT4 AND HAVE_PYQT4_SIP_DIR)
      IF (HAVE_PYQT4)
        MESSAGE (STATUS "PyQt4 development files are missing!")
      ELSE (HAVE_PYQT4)
        MESSAGE (STATUS "PyQt4 not found!")
      ENDIF (HAVE_PYQT4)
    ENDIF (HAVE_PYQT4 AND HAVE_PYQT4_SIP_DIR)
    
    # if SIP and PyQt4 are found, enable bindings
    IF (SIP_IS_GOOD AND PYQT_IS_GOOD)
      SET (HAVE_PYTHON TRUE)
      MESSAGE(STATUS "Python bindings enabled")
    ELSE (SIP_IS_GOOD AND PYQT_IS_GOOD)
      SET (HAVE_PYTHON FALSE)
      MESSAGE(STATUS "Python bindings disabled due dependency problems!")
    ENDIF (SIP_IS_GOOD AND PYQT_IS_GOOD)
    
  ELSE (WITH_BINDINGS)
    MESSAGE(STATUS "Python bindings disabled")
  ENDIF (WITH_BINDINGS)

ENDIF (PYTHON_LIBRARIES AND PYTHON_INCLUDE_PATH)
