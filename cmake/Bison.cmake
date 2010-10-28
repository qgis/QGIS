# Macros for Bison
# ~~~~~~~~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# use bison for .yy files

# search for bison
MACRO(FIND_BISON)
  IF(NOT BISON_EXECUTABLE)
    IF (MSVC)
      FIND_PROGRAM(BISON_EXECUTABLE PATHS
      		   NAMES bison.exe
	           PATHS $ENV{LIB_DIR}/bin $ENV{PROGRAMFILES}/GnuWin32/bin
	)
    ELSE (MSVC)
      FIND_PROGRAM(BISON_EXECUTABLE bison)
    ENDIF (MSVC)
    IF (NOT BISON_EXECUTABLE)

      MESSAGE(FATAL_ERROR "Bison not found - aborting")

    ELSE (NOT BISON_EXECUTABLE)

      EXEC_PROGRAM(${BISON_EXECUTABLE} ARGS --version OUTPUT_VARIABLE BISON_VERSION_STR)
      # get first line in case it's multiline
      STRING(REGEX REPLACE "([^\n]+).*" "\\1" FIRST_LINE "${BISON_VERSION_STR}")
      # get version information
      STRING(REGEX REPLACE ".* ([0-9]+)\\.([0-9]+)" "\\1" BISON_VERSION_MAJOR "${FIRST_LINE}")
      STRING(REGEX REPLACE ".* ([0-9]+)\\.([0-9]+)" "\\2" BISON_VERSION_MINOR "${FIRST_LINE}")
      IF (BISON_VERSION_MAJOR LESS 2)
        MESSAGE (FATAL_ERROR "Bison version is too old (${BISON_VERSION_MAJOR}.${BISON_VERSION_MINOR}). Use 2.0 or higher.")
      ENDIF (BISON_VERSION_MAJOR LESS 2)

    ENDIF (NOT BISON_EXECUTABLE)
  ENDIF(NOT BISON_EXECUTABLE)

ENDMACRO(FIND_BISON)

MACRO(ADD_BISON_FILES _sources )
  FIND_BISON()

  FOREACH (_current_FILE ${ARGN})
    GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
    GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

    SET(_out ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)


    # bison options:
    # -t add debugging facilities
    # -d produce additional header file (used in parser.l)
    # -v produce additional *.output file with parser states

    ADD_CUSTOM_COMMAND(
      OUTPUT ${_out}
      COMMAND ${BISON_EXECUTABLE}
      ARGS
      -o${_out} -d -v -t
      ${_in}
      DEPENDS ${_in}
      )

    SET(${_sources} ${${_sources}} ${_out} )
  ENDFOREACH (_current_FILE)
ENDMACRO(ADD_BISON_FILES)

MACRO(ADD_BISON_FILES_PREFIX _sources prefix)
  FIND_BISON()

  FOREACH (_current_FILE ${ARGN})
    GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
    GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

    SET(_out ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)


    # bison options:
    # -t add debugging facilities
    # -d produce additional header file (used in parser.l)
    # -v produce additional *.output file with parser states

    ADD_CUSTOM_COMMAND(
      OUTPUT ${_out}
      COMMAND ${BISON_EXECUTABLE}
      ARGS
      -p ${prefix}
      -o${_out} -d -v -t
      ${_in}
      DEPENDS ${_in}
      )

    SET(${_sources} ${${_sources}} ${_out} )
  ENDFOREACH (_current_FILE)
ENDMACRO(ADD_BISON_FILES)
