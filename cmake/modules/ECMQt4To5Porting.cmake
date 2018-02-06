#=============================================================================
# Copyright 2005-2011 Kitware, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the name of Kitware, Inc. nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

# The automoc_qt4 macro is superseded by CMAKE_AUTOMOC from CMake 2.8.6
# A Qt 5 version is not provided by CMake or Qt.

include(MacroAddFileDependencies)

# Portability helpers.

set(QT_QTGUI_LIBRARIES
  ${Qt5Gui_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${Qt5PrintSupport_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
)

set(QT_INCLUDES
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5PrintSupport_INCLUDE_DIRS}
    ${Qt5Svg_INCLUDE_DIRS}
)
set(QT_QTGUI_LIBRARY ${QT_QTGUI_LIBRARIES})

set(_qt_modules
  Core
  Declarative
  Widgets
  Script
  ScriptTools
  Network
  Test
  Designer
  Concurrent
  Xml
  XmlPatterns
  UiTools
  Qml
  Quick1
  WebKit
  WebKitWidgets
  Sql
  OpenGL
)

foreach(_module ${_qt_modules})
    string(TOUPPER ${_module} _module_upper)
    set(QT_QT${_module_upper}_LIBRARIES ${Qt5${_module}_LIBRARIES})
    set(QT_QT${_module_upper}_LIBRARY ${QT_QT${_module_upper}_LIBRARIES})
    list(APPEND QT_INCLUDES ${Qt5${_module}_INCLUDE_DIRS})
    set(QT_QT${_module_upper}_FOUND ${Qt5${_module}_FOUND})
endforeach()

list(APPEND QT_QTCORE_LIBRARY ${Qt5Concurrent_LIBRARIES})

list(APPEND QT_QTWEBKIT_LIBRARY ${Qt5WebKitWidgets_LIBRARIES})

Find_Program(QT_LRELEASE_EXECUTABLE NAMES lrelease-qt5 lrelease)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0)

# get the Qt plugins directory
GET_TARGET_PROPERTY(QMAKE_EXECUTABLE Qt5::qmake LOCATION)
EXEC_PROGRAM(${QMAKE_EXECUTABLE} ARGS "-query QT_INSTALL_PLUGINS" RETURN_VALUE return_code OUTPUT_VARIABLE QT_PLUGINS_DIR )
