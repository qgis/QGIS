SET(PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python)

SET (QGIS_PYTHON_OUTPUT_DIRECTORY ${PYTHON_OUTPUT_DIRECTORY}/qgis)
FILE (MAKE_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY})
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY})
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY})

# Python plugins and support packages can be staged to PYTHON_OUTPUT_DIRECTORY
# so plugins will function when app is run from build directory

# When staging all plugins, use the following make targets:
#   staged-plugins - stage plugins (usually after repo pull/build and project make)
#   staged-plugins-pyc - stage and byte-compile all
#   clean-staged-plugins - removes the staged plugins' directories
#
# NOTE: regular project 'make install' is unaffected

# Other target dependencies will be added, per staged resource
ADD_CUSTOM_TARGET(staged-plugins)

# Plugins can also be staged with CMake option at build time
IF(WITH_STAGED_PLUGINS)
  ADD_CUSTOM_TARGET(staged-plugins-on-build ALL DEPENDS staged-plugins)
ENDIF(WITH_STAGED_PLUGINS)

# Non-default/non-option-controlled target to stage and compile plugins and extras
ADD_CUSTOM_TARGET(staged-plugins-pyc DEPENDS staged-plugins
  COMMAND ${PYTHON_EXECUTABLE} -m compileall -q "${PYTHON_OUTPUT_DIRECTORY}"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Byte-compiling build output/python directory..."
)

# Other custom commands will be added, per staged resource
ADD_CUSTOM_TARGET(clean-staged-plugins
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${PYTHON_OUTPUT_DIRECTORY}/plugins"
)

# Macro to byte-compile a target's staged Python resource(s)
MACRO(PY_COMPILE TARGET_NAME RESOURCE_PATHS)
  IF(WITH_PY_COMPILE)
    ADD_CUSTOM_COMMAND(TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND ${PYTHON_EXECUTABLE} -m compileall -q ${RESOURCE_PATHS}
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      COMMENT "Byte-compiling staged resource..."
    )
  ENDIF(WITH_PY_COMPILE)
ENDMACRO(PY_COMPILE)

# Macro to auto migrate resources
MACRO(PY_2TO3 TARGET_NAME RESOURCE_PATHS)
  ADD_CUSTOM_COMMAND(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/2to3" --no-diffs -w ${RESOURCE_PATHS}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Porting ${RESOURCE_PATHS} to Python 3 and Qt5"
  )
ENDMACRO(PY_2TO3)

ADD_SUBDIRECTORY(plugins)
IF (WITH_GUI)
  ADD_SUBDIRECTORY(qsci_apis)
  ADD_SUBDIRECTORY(console)
  ADD_SUBDIRECTORY(pyplugin_installer)
ENDIF ()
ADD_SUBDIRECTORY(PyQt)
ADD_SUBDIRECTORY(ext-libs)
ADD_SUBDIRECTORY(testing)

INCLUDE_DIRECTORIES(SYSTEM
  ${PYTHON_INCLUDE_PATH}
  ${SIP_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTNETWORK_INCLUDE_DIR}
  ${QT_QTSVG_INCLUDE_DIR}
  ${QT_QTXML_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
)

IF (WITH_GUI)
  INCLUDE_DIRECTORIES(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
ENDIF ()

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/3d
  ${CMAKE_SOURCE_DIR}/src/core/annotations
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/pal
  ${CMAKE_SOURCE_DIR}/src/core/diagram
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/fieldformatter
  ${CMAKE_SOURCE_DIR}/src/core/dxf
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/geocms
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/gps
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/locator
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/mesh
  ${CMAKE_SOURCE_DIR}/src/core/processing
  ${CMAKE_SOURCE_DIR}/src/core/processing/models
  ${CMAKE_SOURCE_DIR}/src/core/providers
  ${CMAKE_SOURCE_DIR}/src/core/providers/memory
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/scalebar
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/plugins

  ${CMAKE_BINARY_DIR}  # qgsconfig.h, qgsversion.h
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/analysis
)

IF (WITH_GUI)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src/gui
    ${CMAKE_SOURCE_DIR}/src/gui/symbology
    ${CMAKE_SOURCE_DIR}/src/gui/raster
    ${CMAKE_SOURCE_DIR}/src/gui/attributetable
    ${CMAKE_SOURCE_DIR}/src/gui/auth
    ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
    ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets/core
    ${CMAKE_SOURCE_DIR}/src/gui/effects
    ${CMAKE_SOURCE_DIR}/src/gui/layertree
    ${CMAKE_SOURCE_DIR}/src/gui/layout
    ${CMAKE_SOURCE_DIR}/src/gui/locator
    ${CMAKE_SOURCE_DIR}/src/gui/processing

    ${CMAKE_BINARY_DIR}/src/gui
  )
ENDIF ()

IF(NOT ENABLE_TESTS)
  SET(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} TESTS)
ENDIF(NOT ENABLE_TESTS)

IF(NOT ANDROID)
  SET(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} ANDROID)
ENDIF(NOT ANDROID)

IF(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
ELSE(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  SET(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} ARM)
ENDIF(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")

IF(NOT QT_MOBILITY_LOCATION_FOUND)
  SET(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} MOBILITY_LOCATION)
ENDIF(NOT QT_MOBILITY_LOCATION_FOUND)

# SIP 4.19.7+ can:
# * prepend auto-generated Python signature to existing Docstrings
# * document template based classes
SET(DOCSTRINGSTEMPLATE "//")
IF(${SIP_VERSION_STR} VERSION_GREATER 4.19.6)
  SET(DEFAULTDOCSTRINGSIGNATURE "%DefaultDocstringSignature \"prepended\"")
  SET(DOCSTRINGSTEMPLATE "")
ENDIF(${SIP_VERSION_STR} VERSION_GREATER 4.19.6)

IF(${SIP_VERSION_STR} VERSION_GREATER 4.19.0)
  SET(SIP_FINAL "final")
ELSE(${SIP_VERSION_STR} VERSION_GREATER 4.19.0)
  SET(SIP_FINAL "")
ENDIF(${SIP_VERSION_STR} VERSION_GREATER 4.19.0)

# core module
FILE(GLOB_RECURSE sip_files_core core/*.sip core/*.sip.in)
SET(SIP_EXTRA_FILES_DEPEND ${sip_files_core})
SET(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.core.api)
IF((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
  SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n sip)
ENDIF((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
IF((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
  SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_core.pyi)
ENDIF((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
GENERATE_SIP_PYTHON_MODULE_CODE(qgis._core core/core.sip "${sip_files_core}" cpp_files)
BUILD_SIP_PYTHON_MODULE(qgis._core core/core.sip ${cpp_files} "" qgis_core)
SET(SIP_CORE_CPP_FILES ${cpp_files})

IF(UNIX AND NOT SIP_VERSION_NUM LESS 265984)
  SET(SIP_EXTRA_OPTIONS -P ${SIP_EXTRA_OPTIONS})
  ADD_DEFINITIONS(-Dprotected=public)
ENDIF(UNIX AND NOT SIP_VERSION_NUM LESS 265984)

SET(PY_MODULES core analysis)

# gui module
IF (WITH_GUI)
  SET(PY_MODULES ${PY_MODULES} gui)

  FILE(GLOB_RECURSE sip_files_gui gui/*.sip gui/*.sip.in)
  SET(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_gui})
  SET(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.gui.api)
  IF((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n sip)
  ENDIF((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
  IF((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
    SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_gui.pyi)
  ENDIF((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
  IF(QSCI_SIP_DIR)
    SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -I ${QSCI_SIP_DIR})
  ELSE(QSCI_SIP_DIR)
    MESSAGE(STATUS "Qsci sip file not found - disabling bindings for derived classes")
    SET(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} HAVE_QSCI_SIP)
  ENDIF(QSCI_SIP_DIR)

  GENERATE_SIP_PYTHON_MODULE_CODE(qgis._gui gui/gui.sip "${sip_files_gui}" cpp_files)
  BUILD_SIP_PYTHON_MODULE(qgis._gui gui/gui.sip ${cpp_files} "" qgis_core qgis_gui)
ENDIF (WITH_GUI)

# 3D module
IF (WITH_3D)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src/3d
    ${CMAKE_SOURCE_DIR}/src/3d/chunks
    ${CMAKE_SOURCE_DIR}/src/3d/symbols
    ${CMAKE_SOURCE_DIR}/src/3d/terrain
    ${CMAKE_BINARY_DIR}/src/3d
  )
  SET(PY_MODULES ${PY_MODULES} 3d)

  FILE(GLOB_RECURSE sip_files_3d 3d/*.sip 3d/*.sip.in)
  SET(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_3d})
  SET(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.qgis3d.api)
  IF((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n sip)
  ENDIF((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
  IF((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
    SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_qgis3d.pyi)
  ENDIF((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))

  GENERATE_SIP_PYTHON_MODULE_CODE(qgis._3d 3d/3d.sip "${sip_files_3d}" cpp_files)
  BUILD_SIP_PYTHON_MODULE(qgis._3d 3d/3d.sip ${cpp_files} "" qgis_core qgis_3d)
ENDIF (WITH_3D)

# server module
IF (WITH_SERVER AND WITH_SERVER_PLUGINS)
  INCLUDE_DIRECTORIES(
    ../src/server
   ${CMAKE_BINARY_DIR}/src/server
  )

  SET(PY_MODULES ${PY_MODULES} server)

  FILE(GLOB_RECURSE sip_files_server server/*.sip server/*.sip.in)
  SET(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_server})
  SET(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.server.api)
  IF((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n sip)
  ENDIF((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
  IF((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
    SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_server.pyi)
  ENDIF((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
  GENERATE_SIP_PYTHON_MODULE_CODE(qgis._server server/server.sip "${sip_files_server}" cpp_files)
  BUILD_SIP_PYTHON_MODULE(qgis._server server/server.sip ${cpp_files} "" qgis_core qgis_server)
ENDIF (WITH_SERVER AND WITH_SERVER_PLUGINS)

# additional analysis includes
INCLUDE_DIRECTORIES(BEFORE
  ${CMAKE_SOURCE_DIR}/src/analysis
  ${CMAKE_SOURCE_DIR}/src/analysis/processing
  ${CMAKE_SOURCE_DIR}/src/analysis/vector
  ${CMAKE_SOURCE_DIR}/src/analysis/vector/geometry_checker
  ${CMAKE_SOURCE_DIR}/src/analysis/raster
  ${CMAKE_SOURCE_DIR}/src/analysis/network
  ${CMAKE_SOURCE_DIR}/src/analysis/interpolation
  ${CMAKE_SOURCE_DIR}/src/analysis/openstreetmap

  ${CMAKE_BINARY_DIR}/src/analysis/processing
  ${CMAKE_BINARY_DIR}/src/analysis/vector
  ${CMAKE_BINARY_DIR}/src/analysis/raster
  ${CMAKE_BINARY_DIR}/src/analysis/network
  ${CMAKE_BINARY_DIR}/src/analysis/interpolation
  ${CMAKE_BINARY_DIR}/src/analysis/openstreetmap
)

# analysis module
FILE(GLOB_RECURSE sip_files_analysis analysis/*.sip analysis/*.sip.in)
SET(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_analysis})
SET(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.analysis.api)
IF((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
  SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n sip)
ENDIF((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
IF((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
  SET(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_analysis.pyi)
ENDIF((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
GENERATE_SIP_PYTHON_MODULE_CODE(qgis._analysis analysis/analysis.sip "${sip_files_analysis}" cpp_files)
BUILD_SIP_PYTHON_MODULE(qgis._analysis analysis/analysis.sip ${cpp_files} "" qgis_core qgis_analysis)

SET(QGIS_PYTHON_DIR ${PYTHON_SITE_PACKAGES_DIR}/qgis)

IF(WITH_QSCIAPI)
  # wait until after python module builds for api files to be available
  SET(QGIS_PYTHON_API_FILE "${CMAKE_BINARY_DIR}/python/qsci_apis/PyQGIS.api")

  ADD_CUSTOM_TARGET(qsci-api ALL
    DEPENDS python_module_qgis__gui python_module_qgis__core python_module_qgis__analysis)

  # run update/concatenate command
  ADD_CUSTOM_COMMAND(TARGET qsci-api
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_SOURCE_DIR}/cmake/QsciAPI.cmake"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating pyqgis api file" VERBATIM)

  INSTALL(FILES ${QGIS_PYTHON_API_FILE} DESTINATION "${QGIS_DATA_DIR}/python/qsci_apis")

  # create target for generating console auto-completion *.pap binary file
  # takes too long to build (> 1 minute) for targets to have ALL property
  SET(APIS_SRC_DIR "${CMAKE_SOURCE_DIR}/python/qsci_apis")
  SET(APIS_BIN_DIR "${CMAKE_BINARY_DIR}/python/qsci_apis")

  # generate a .pap file to be immediately installed in QGIS source tree (the default .pap)
  ADD_CUSTOM_TARGET(qsci-pap-src
    DEPENDS qsci-api ${QGIS_PYTHON_API_FILE})

  SET(PAP_NAME "pyqgis.pap")
  ADD_CUSTOM_COMMAND(TARGET qsci-pap-src
    POST_BUILD
    COMMAND ${PYTHON_EXECUTABLE} "${APIS_SRC_DIR}/generate_console_pap.py" "${APIS_SRC_DIR}/${PAP_NAME}" "${APIS_SRC_DIR}" "${APIS_BIN_DIR}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating ${PAP_NAME} for console auto-completion (MAY TAKE > 1 MINUTE!)" VERBATIM)
ENDIF(WITH_QSCIAPI)

IF(WITH_CUSTOM_WIDGETS)
  INSTALL(FILES custom_widgets/qgis_customwidgets.py DESTINATION "${PYUIC_WIDGET_PLUGIN_DIRECTORY}")
ENDIF(WITH_CUSTOM_WIDGETS)

# Plugin utilities files to copy to staging or install
SET(PY_FILES
  __init__.py
  utils.py
  user.py
)

ADD_CUSTOM_TARGET(pyutils ALL)
INSTALL(FILES ${PY_FILES} DESTINATION "${QGIS_PYTHON_DIR}")

# stage to output to make available when QGIS is run from build directory
FOREACH(pyfile ${PY_FILES})
  ADD_CUSTOM_COMMAND(TARGET pyutils
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${QGIS_PYTHON_OUTPUT_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${pyfile}
  )
  PY_COMPILE(pyutils "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${pyfile}")
ENDFOREACH(pyfile)

FOREACH(module ${PY_MODULES})
  ADD_CUSTOM_TARGET(py${module} ALL)
  ADD_DEPENDENCIES(py${module} python_module_qgis__${module})

  # concat auto_additions/*.py in _module_/__init__.py
  FILE(GLOB PY_FILES_AUTO_ADDITIONS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${module}/auto_additions/*.py)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${module}/__init__.py.in ${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module}/__init__.py COPYONLY)
  FOREACH(pyfile ${PY_FILES_AUTO_ADDITIONS})
    file(READ ${pyfile} CONTENTS)
    file(APPEND ${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module}/__init__.py "${CONTENTS}")
  ENDFOREACH(pyfile)
  INSTALL(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module}/__init__.py DESTINATION "${QGIS_PYTHON_DIR}/${module}")

  FILE(GLOB PY_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${module}/*.py)
  INSTALL(FILES ${PY_FILES} DESTINATION "${QGIS_PYTHON_DIR}/${module}")
  FILE(GLOB PY_FILES_ADDITIONS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${module}/additions/*.py)
  INSTALL(FILES ${PY_FILES_ADDITIONS} DESTINATION "${QGIS_PYTHON_DIR}/${module}/additions")
  SET(PY_FILES ${PY_FILES} ${PY_FILES_ADDITIONS})
  FOREACH(pyfile ${PY_FILES})
    GET_FILENAME_COMPONENT(subdir ${pyfile} DIRECTORY)
    ADD_CUSTOM_COMMAND(TARGET py${module}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${subdir}"
      COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${subdir}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${PY_FILES}
    )
  ENDFOREACH(pyfile)
  PY_COMPILE(py${module} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module}")
ENDFOREACH(module)
