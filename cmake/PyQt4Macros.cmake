

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
