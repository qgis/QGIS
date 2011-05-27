# Macros for txt2tags
# ~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2011, Juergen E. Fischer <jef at norbit dot de>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# use txt2tags for .t2t files

# search for bison
MACRO(FIND_TXT2TAGS)
  IF(NOT TXT2TAGS_EXECUTABLE)
    IF (MSVC)
      FIND_PROGRAM(TXT2TAGS_EXECUTABLE PATHS
      		   NAMES txt2tags.exe
	           PATHS $ENV{LIB_DIR}/bin $ENV{PROGRAMFILES}/GnuWin32/bin
	)
    ELSE (MSVC)
      FIND_PROGRAM(TXT2TAGS_EXECUTABLE txt2tags)
    ENDIF (MSVC)
    IF (NOT TXT2TAGS_EXECUTABLE)
      MESSAGE(STATUS "txt2tags not found - disabled")
    ENDIF (NOT TXT2TAGS_EXECUTABLE)
  ENDIF(NOT TXT2TAGS_EXECUTABLE)

  IF (WITH_TXT2TAGS_PDF)
    IF (NOT PDFLATEX_EXECUTABLE)
      FIND_PROGRAM(PDFLATEX_EXECUTABLE pdflatex)
    ENDIF (NOT PDFLATEX_EXECUTABLE)
    IF (NOT PDFLATEX_EXECUTABLE)
      MESSAGE(ERROR "pdflatex not found - txt2tags documention pdf cannot be generated")
    ENDIF(NOT PDFLATEX_EXECUTABLE)
  ENDIF(WITH_TXT2TAGS_PDF)
ENDMACRO(FIND_TXT2TAGS)

MACRO(ADD_TXT2TAGS_FILES _sources)
  FOREACH (_current_FILE ${ARGN})
    GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
    GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

    SET(_out ${CMAKE_CURRENT_BINARY_DIR}/${_basename})

    ADD_CUSTOM_COMMAND(
      OUTPUT ${_out}
      COMMAND ${TXT2TAGS_EXECUTABLE}
      ARGS -o${_out} -t txt ${_in}
      DEPENDS ${_in}
      COMMENT "Building ${_out} from ${_in}"
      )

    ADD_CUSTOM_COMMAND(
      OUTPUT ${_out}.html
      COMMAND ${TXT2TAGS_EXECUTABLE}
      ARGS -o${_out}.html -t html ${_in}
      DEPENDS ${_in}
      COMMENT "Building ${_out}.html from ${_in}"
      )

    SET(${_sources} ${${_sources}} ${_out} ${_out}.html)
  ENDFOREACH (_current_FILE)
ENDMACRO(ADD_TXT2TAGS_FILES)

MACRO(ADD_TXT2TAGS_PDFS _sources)
  FOREACH (_current_FILE ${ARGN})
    GET_FILENAME_COMPONENT(_in ${_current_FILE} ABSOLUTE)
    GET_FILENAME_COMPONENT(_basename ${_current_FILE} NAME_WE)

    SET(_out ${CMAKE_CURRENT_BINARY_DIR}/${_basename})

    ADD_CUSTOM_COMMAND(
      OUTPUT ${_out}.tex
      COMMAND ${TXT2TAGS_EXECUTABLE}
      ARGS -o${_out}.tex -t tex ${_in}
      DEPENDS ${_in}
      COMMENT "Building ${_out}.tex from ${_in}"
      )

    ADD_CUSTOM_COMMAND(
      OUTPUT ${_out}.pdf
      COMMAND TEXINPUTS=.:${CMAKE_CURRENT_SOURCE_DIR}: ${PDFLATEX_EXECUTABLE}
      ARGS -output-directory=${CMAKE_CURRENT_BINARY_DIR} ${_out}.tex
      DEPENDS ${_out}.tex
      COMMENT "Building ${_out}.pdf from ${_out}.tex"
      )
    SET(${_sources} ${${_sources}} ${_out}.pdf)
  ENDFOREACH (_current_FILE)
ENDMACRO(ADD_TXT2TAGS_PDFS)
