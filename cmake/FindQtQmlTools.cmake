# Qt QML Tools
# ~~~~~~~~~~~~
#
# To generate qmltypes files required by Qt Creator to allow QML code inspection
# (http://doc.qt.io/qtcreator/creator-qml-modules-with-plugins.html#generating-qmltypes-files)
# we need to have installed qmlplugindump unity (shipped with Qt 4.8 and later)
# http://doc.qt.io/qtcreator/creator-qml-modules-with-plugins.html#dumping-plugins-automatically
#
# Find the installed version of qmlplugindump utility.
# FindQtQmlTools should be called after Qt5 has been found
#
# This file defines the following variables:
#
# QMLPLUGINDUMP_FOUND - system has qmlplugindump
# QMLPLUGINDUMP_EXECUTABLE - Path to qmlplugindump executable
#
# Also defines MACRO to create qmltypes file, when QML directory is supplied
#
# Copyright (c) 2017, Peter Petrik <zilolv at gmail dot com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

MACRO(FIND_QMLPLUGINDUMP)
  get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)

  IF(NOT QMLPLUGINDUMP_EXECUTABLE)
    IF (MSVC)
      FIND_PROGRAM(QMLPLUGINDUMP_EXECUTABLE qmlplugindump.exe PATHS ${QT_BIN_DIR} NO_DEFAULT_PATH)
    ELSE (MSVC)
      FIND_PROGRAM(QMLPLUGINDUMP_EXECUTABLE qmlplugindump PATHS ${QT_BIN_DIR} NO_DEFAULT_PATH)
    ENDIF (MSVC)
  ENDIF(NOT QMLPLUGINDUMP_EXECUTABLE)

  IF (QMLPLUGINDUMP_EXECUTABLE)
    SET(QMLPLUGINDUMP_FOUND TRUE)
    MESSAGE(STATUS "Found qmlplugindump: ${QMLPLUGINDUMP_EXECUTABLE}")
  ELSE()
    SET(QMLPLUGINDUMP_FOUND FALSE)
    IF (QMLPLUGINDUMP_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find qmlplugindump")
    ELSE (QMLPLUGINDUMP_FIND_REQUIRED)
      MESSAGE(WARNING "Could not find qmlplugindump")
    ENDIF (QMLPLUGINDUMP_FIND_REQUIRED)
  ENDIF (QMLPLUGINDUMP_EXECUTABLE)
ENDMACRO(FIND_QMLPLUGINDUMP)

IF (NOT QMLPLUGINDUMP_FOUND)
  FIND_QMLPLUGINDUMP()
ENDIF (NOT QMLPLUGINDUMP_FOUND)
