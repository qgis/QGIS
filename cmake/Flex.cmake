# Macros for Bison
# ~~~~~~~~~~~~~~~~
# Copyright (c) 2007, Martin Dobias <wonder.sk at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# flex a .ll file

# search flex
MACRO(FIND_FLEX)
    IF(NOT FLEX_EXECUTABLE)
      IF (MSVC)
        FIND_PROGRAM(FLEX_EXECUTABLE
                     NAMES flex.exe
                     PATHS $ENV{LIB_DIR}/bin $ENV{PROGRAMFILES}/GnuWin32/bin
                    )
      ELSE(MSVC)
        FIND_PROGRAM(FLEX_EXECUTABLE flex)
      ENDIF (MSVC)
        IF (NOT FLEX_EXECUTABLE)
          MESSAGE(FATAL_ERROR "flex not found - aborting")
        ENDIF (NOT FLEX_EXECUTABLE)
    ENDIF(NOT FLEX_EXECUTABLE)
ENDMACRO(FIND_FLEX)

MACRO(ADD_FLEX_FILES _sources )
    FIND_FLEX()

    FOREACH (_current_FILE ${ARGN})
      GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
      GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

      SET(_out ${CMAKE_CURRENT_BINARY_DIR}/flex_${_basename}.cpp)


      # -d option for flex means that it will produce output to stderr while analyzing 

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_out}
         COMMAND ${FLEX_EXECUTABLE}
         ARGS
         -o${_out} -d
         ${_in}
         DEPENDS ${_in}
      )

      SET(${_sources} ${${_sources}} ${_out} )
   ENDFOREACH (_current_FILE)
ENDMACRO(ADD_FLEX_FILES)


MACRO(ADD_FLEX_FILES_PREFIX _sources prefix )
    FIND_FLEX()

    FOREACH (_current_FILE ${ARGN})
      GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
      GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

      SET(_out ${CMAKE_CURRENT_BINARY_DIR}/flex_${_basename}.cpp)


      # -d option for flex means that it will produce output to stderr while analyzing 

      ADD_CUSTOM_COMMAND(
         OUTPUT ${_out}
         COMMAND ${FLEX_EXECUTABLE}
         ARGS
	 -P ${prefix}
         -o${_out} -d
         ${_in}
         DEPENDS ${_in}
      )

      SET(${_sources} ${${_sources}} ${_out} )
   ENDFOREACH (_current_FILE)
ENDMACRO(ADD_FLEX_FILES_PREFIX)
