file (MAKE_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY})

# Python plugins and support packages can be staged to PYTHON_OUTPUT_DIRECTORY
# so plugins will function when app is run from build directory

# When staging all plugins, use the following make targets:
#   staged-plugins - stage plugins (usually after repo pull/build and project make)
#   staged-plugins-pyc - stage and byte-compile all
#   clean-staged-plugins - removes the staged plugins' directories
#
# NOTE: regular project 'make install' is unaffected

# Other target dependencies will be added, per staged resource
add_custom_target(staged-plugins)

# Plugins can also be staged with CMake option at build time
if(WITH_STAGED_PLUGINS)
  add_custom_target(staged-plugins-on-build ALL DEPENDS staged-plugins)
endif()

# Non-default/non-option-controlled target to stage and compile plugins and extras
add_custom_target(staged-plugins-pyc DEPENDS staged-plugins
  COMMAND ${Python_EXECUTABLE} -m compileall -q "${PYTHON_OUTPUT_DIRECTORY}"
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMENT "Byte-compiling build output/python directory..."
)

# Other custom commands will be added, per staged resource
add_custom_target(clean-staged-plugins
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${PYTHON_OUTPUT_DIRECTORY}/plugins"
)

# Macro to byte-compile a target's staged Python resource(s)
macro(PY_COMPILE TARGET_NAME RESOURCE_PATHS)
  if(WITH_PY_COMPILE)
    add_custom_command(TARGET ${TARGET_NAME}
      POST_BUILD
      COMMAND ${Python_EXECUTABLE} -m compileall -q ${RESOURCE_PATHS}
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      COMMENT "Byte-compiling staged resource..."
    )
  endif()
endmacro(PY_COMPILE)

# Macro to auto migrate resources
macro(PY_2TO3 TARGET_NAME RESOURCE_PATHS)
  add_custom_command(TARGET ${TARGET_NAME}
    POST_BUILD
    COMMAND "${Python_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/2to3" --no-diffs -w ${RESOURCE_PATHS}
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Porting ${RESOURCE_PATHS} to Python 3 and Qt5"
  )
endmacro(PY_2TO3)

add_subdirectory(plugins)
if (WITH_GUI)
  add_subdirectory(qsci_apis)
  add_subdirectory(console)
  add_subdirectory(pyplugin_installer)
endif()
add_subdirectory(PyQt)
add_subdirectory(ext-libs)
add_subdirectory(testing)
add_subdirectory(processing)

include_directories(SYSTEM
  ${Python_INCLUDE_DIRS}
  ${SIP_INCLUDE_DIR}
  ${QT_QTCORE_INCLUDE_DIR}
  ${QT_QTGUI_INCLUDE_DIR}
  ${QT_QTNETWORK_INCLUDE_DIR}
  ${QT_QTSVG_INCLUDE_DIR}
  ${QT_QTXML_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${SPATIALINDEX_INCLUDE_DIR}
)

if (${SIP_VERSION_STR} VERSION_LESS 5)
  include_directories(SYSTEM
    ${SIP_INCLUDE_DIR}
  )
endif()

if (WITH_GUI)
  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins

  ${CMAKE_BINARY_DIR}  # qgsconfig.h, qgsversion.h
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/analysis
)

if (WITH_GUI)
  include_directories(

  )
endif()

if(NOT ENABLE_TESTS)
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} TESTS)
endif()

if(NOT ANDROID)
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} ANDROID)
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
else()
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} ARM)
endif()

# SIP 4.19.7+ can:
# * prepend auto-generated Python signature to existing Docstrings
# * document template based classes
set(DOCSTRINGSTEMPLATE "//")
if(${SIP_VERSION_STR} VERSION_GREATER 4.19.6)
  set(DEFAULTDOCSTRINGSIGNATURE "%DefaultDocstringSignature \"prepended\"")
  set(DOCSTRINGSTEMPLATE "")
endif()

if(${SIP_VERSION_STR} VERSION_GREATER 4.19.0)
  set(SIP_FINAL "final")
else()
  set(SIP_FINAL "")
endif()

set(QGIS_PYTHON_DIR ${Python_SITEARCH}/qgis)

# core module
file(GLOB_RECURSE sip_files_core core/*.sip core/*.sip.in)
set(SIP_EXTRA_FILES_DEPEND ${sip_files_core})
set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.core.api)
if((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
  set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n ${PYQT5_SIP_IMPORT})
endif()
if(SIP_BUILD_EXECUTABLE)
  install(FILES ${CMAKE_BINARY_DIR}/python/core/build/_core/_core.pyi DESTINATION ${QGIS_PYTHON_DIR})
elseif((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
  set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_core.pyi)
  install(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_core.pyi DESTINATION ${QGIS_PYTHON_DIR})
endif()

if((${PYQT5_VERSION_STR} VERSION_EQUAL 5.15) OR (${PYQT5_VERSION_STR} VERSION_GREATER 5.15))
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} VECTOR_MAPPED_TYPE)
endif()

if(${QT_VERSION_BASE}Core_VERSION VERSION_LESS 5.13.0)
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} CONCATENATED_TABLES_MODEL)
endif()

if(NOT WITH_GUI)
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} HAVE_GUI)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/core/project.py.in ${CMAKE_CURRENT_BINARY_DIR}/core/project.py @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/core/pyproject.toml.in ${CMAKE_CURRENT_BINARY_DIR}/core/pyproject.toml @ONLY)
GENERATE_SIP_PYTHON_MODULE_CODE(qgis._core core/core.sip "${sip_files_core}" cpp_files)
BUILD_SIP_PYTHON_MODULE(qgis._core core/core.sip ${cpp_files} "" qgis_core)
set(SIP_CORE_CPP_FILES ${cpp_files})

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # Bad hack to fix compilation with gcc 11 - for some reason it's ignoring
  # the c++ standard version set for the target in BUILD_SIP_PYTHON_MODULE!
  add_definitions(-std=c++14)
endif()

if((UNIX OR MINGW) AND SIP_VERSION_STR VERSION_GREATER_EQUAL 4.10 AND SIP_VERSION_STR VERSION_LESS 5.0)
  set(SIP_EXTRA_OPTIONS -P ${SIP_EXTRA_OPTIONS})
  add_definitions(-Dprotected=public)
elseif((UNIX OR MINGW) AND SIP_VERSION_STR VERSION_GREATER_EQUAL 5.0)
  add_definitions(-Dprotected=public)
endif()

set (PY_MODULES core)

if(WITH_ANALYSIS)
  set(PY_MODULES ${PY_MODULES} analysis)
endif()

# gui module
if (WITH_GUI)
  set(PY_MODULES ${PY_MODULES} gui)

  file(GLOB_RECURSE sip_files_gui gui/*.sip gui/*.sip.in)
  set(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_gui})
  set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.gui.api)
  if((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n ${PYQT5_SIP_IMPORT})
  endif()
  if(SIP_BUILD_EXECUTABLE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gui/build/_gui/_gui.pyi DESTINATION ${QGIS_PYTHON_DIR})
  elseif((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_gui.pyi)
    install(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_gui.pyi DESTINATION ${QGIS_PYTHON_DIR})
  endif()
  if(QSCI_SIP_DIR)
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -I ${QSCI_SIP_DIR})
  else()
    message(STATUS "Qsci sip file not found - disabling bindings for derived classes")
    set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} HAVE_QSCI_SIP)
  endif()

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gui/project.py.in ${CMAKE_CURRENT_BINARY_DIR}/gui/project.py @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gui/pyproject.toml.in ${CMAKE_CURRENT_BINARY_DIR}/gui/pyproject.toml @ONLY)
  GENERATE_SIP_PYTHON_MODULE_CODE(qgis._gui gui/gui.sip "${sip_files_gui}" cpp_files)
  BUILD_SIP_PYTHON_MODULE(qgis._gui gui/gui.sip ${cpp_files} "" qgis_core qgis_gui)
endif()

# 3D module
if (WITH_3D)
  include_directories(
    ${QT5_3DEXTRA_INCLUDE_DIR}
  )
  set(PY_MODULES ${PY_MODULES} 3d)

  file(GLOB_RECURSE sip_files_3d 3d/*.sip 3d/*.sip.in)
  set(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_3d})
  set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.qgis3d.api)
  if((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n ${PYQT5_SIP_IMPORT})
  endif()
  if(SIP_BUILD_EXECUTABLE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/3d/build/_3d/_3d.pyi DESTINATION ${QGIS_PYTHON_DIR})
  elseif((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_qgis3d.pyi)
    install(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_qgis3d.pyi DESTINATION ${QGIS_PYTHON_DIR})
  endif()

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/3d/project.py.in ${CMAKE_CURRENT_BINARY_DIR}/3d/project.py @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/3d/pyproject.toml.in ${CMAKE_CURRENT_BINARY_DIR}/3d/pyproject.toml @ONLY)
  GENERATE_SIP_PYTHON_MODULE_CODE(qgis._3d 3d/3d.sip "${sip_files_3d}" cpp_files)
  BUILD_SIP_PYTHON_MODULE(qgis._3d 3d/3d.sip ${cpp_files} "" qgis_core qgis_3d)
endif()

# server module
if (WITH_SERVER AND WITH_SERVER_PLUGINS)
  include_directories(
    ../src/server
   ${CMAKE_BINARY_DIR}/src/server
  )

  set(PY_MODULES ${PY_MODULES} server)

  file(GLOB_RECURSE sip_files_server server/*.sip server/*.sip.in)
  set(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_server})
  set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.server.api)
  if((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n ${PYQT5_SIP_IMPORT})
  endif()
  if(SIP_BUILD_EXECUTABLE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/server/build/_server/_server.pyi DESTINATION ${QGIS_PYTHON_DIR})
  elseif((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_server.pyi)
    install(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_server.pyi DESTINATION ${QGIS_PYTHON_DIR})
  endif()

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/server/project.py.in ${CMAKE_CURRENT_BINARY_DIR}/server/project.py @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/server/pyproject.toml.in ${CMAKE_CURRENT_BINARY_DIR}/server/pyproject.toml @ONLY)
  GENERATE_SIP_PYTHON_MODULE_CODE(qgis._server server/server.sip "${sip_files_server}" cpp_files)
  BUILD_SIP_PYTHON_MODULE(qgis._server server/server.sip ${cpp_files} "" qgis_core qgis_server)
endif()

# additional analysis includes
if(WITH_ANALYSIS)
  include_directories(BEFORE
    ${CMAKE_BINARY_DIR}/src/analysis/processing
    ${CMAKE_BINARY_DIR}/src/analysis/georeferencing
    ${CMAKE_BINARY_DIR}/src/analysis/vector
    ${CMAKE_BINARY_DIR}/src/analysis/mesh
    ${CMAKE_BINARY_DIR}/src/analysis/raster
    ${CMAKE_BINARY_DIR}/src/analysis/network
    ${CMAKE_BINARY_DIR}/src/analysis/interpolation
  )

  # analysis module
  file(GLOB_RECURSE sip_files_analysis analysis/*.sip analysis/*.sip.in)

  set(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_analysis})
  set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.analysis.api)
  if((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n ${PYQT5_SIP_IMPORT})
  endif()
  if(SIP_BUILD_EXECUTABLE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/analysis/build/_analysis/_analysis.pyi DESTINATION ${QGIS_PYTHON_DIR})
  elseif((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_analysis.pyi)
    install(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_analysis.pyi DESTINATION ${QGIS_PYTHON_DIR})
  endif()

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/analysis/project.py.in ${CMAKE_CURRENT_BINARY_DIR}/analysis/project.py @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/analysis/pyproject.toml.in ${CMAKE_CURRENT_BINARY_DIR}/analysis/pyproject.toml @ONLY)
  GENERATE_SIP_PYTHON_MODULE_CODE(qgis._analysis analysis/analysis.sip "${sip_files_analysis}" cpp_files)
  BUILD_SIP_PYTHON_MODULE(qgis._analysis analysis/analysis.sip ${cpp_files} "" qgis_core qgis_analysis)
endif()

if(WITH_QSCIAPI)
  # wait until after python module builds for api files to be available
  set(QGIS_PYTHON_API_FILE "${CMAKE_BINARY_DIR}/python/qsci_apis/PyQGIS.api")

  add_custom_target(qsci-api ALL
    DEPENDS python_module_qgis__gui python_module_qgis__core python_module_qgis__analysis)

  # run update/concatenate command
  add_custom_command(TARGET qsci-api
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -P "${CMAKE_SOURCE_DIR}/cmake/QsciAPI.cmake"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating pyqgis api file" VERBATIM)

  install(FILES ${QGIS_PYTHON_API_FILE} DESTINATION "${QGIS_DATA_DIR}/python/qsci_apis")

  # create target for generating console auto-completion *.pap binary file
  # takes too long to build (> 1 minute) for targets to have ALL property
  set(APIS_SRC_DIR "${CMAKE_SOURCE_DIR}/python/qsci_apis")
  set(APIS_BIN_DIR "${CMAKE_BINARY_DIR}/python/qsci_apis")

  # generate a .pap file to be immediately installed in QGIS source tree (the default .pap)
  add_custom_target(qsci-pap-src
    DEPENDS qsci-api ${QGIS_PYTHON_API_FILE})

  set(PAP_NAME "pyqgis.pap")
  add_custom_command(TARGET qsci-pap-src
    POST_BUILD
    COMMAND ${Python_EXECUTABLE} "${APIS_SRC_DIR}/generate_console_pap.py" "${APIS_SRC_DIR}/${PAP_NAME}" "${APIS_SRC_DIR}" "${APIS_BIN_DIR}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating ${PAP_NAME} for console auto-completion (MAY TAKE > 1 MINUTE!)" VERBATIM)
endif()

if(WITH_CUSTOM_WIDGETS)
  install(FILES custom_widgets/qgis_customwidgets.py DESTINATION "${PYUIC_WIDGET_PLUGIN_DIRECTORY}")
endif()

# Plugin utilities files to copy to staging or install
set(PY_FILES
  __init__.py
  utils.py
  user.py
)

add_custom_target(pyutils ALL)
install(FILES ${PY_FILES} DESTINATION "${QGIS_PYTHON_DIR}")

# stage to output to make available when QGIS is run from build directory
foreach(pyfile ${PY_FILES})
  add_custom_command(TARGET pyutils
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${QGIS_PYTHON_OUTPUT_DIRECTORY}"
    COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${pyfile}
  )
  PY_COMPILE(pyutils "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${pyfile}")
endforeach(pyfile)

foreach(module ${PY_MODULES})
  add_custom_target(py${module} ALL)
  add_dependencies(py${module} python_module_qgis__${module})

  # concat auto_additions/*.py in _module_/__init__.py
  file(GLOB PY_FILES_AUTO_ADDITIONS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${module}/auto_additions/*.py)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${module}/__init__.py.in ${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module}/__init__.py COPYONLY)
  foreach(pyfile ${PY_FILES_AUTO_ADDITIONS})
    file(READ ${pyfile} CONTENTS)
    file(APPEND ${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module}/__init__.py "${CONTENTS}")
  endforeach(pyfile)
  install(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module}/__init__.py DESTINATION "${QGIS_PYTHON_DIR}/${module}")

  file(GLOB PY_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${module}/*.py)
  install(FILES ${PY_FILES} DESTINATION "${QGIS_PYTHON_DIR}/${module}")
  file(GLOB PY_FILES_ADDITIONS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${module}/additions/*.py)
  install(FILES ${PY_FILES_ADDITIONS} DESTINATION "${QGIS_PYTHON_DIR}/${module}/additions")
  set(PY_FILES ${PY_FILES} ${PY_FILES_ADDITIONS})
  foreach(pyfile ${PY_FILES})
    get_filename_component(subdir ${pyfile} DIRECTORY)
    add_custom_command(TARGET py${module}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${subdir}"
      COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${subdir}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS ${PY_FILES}
    )
  endforeach(pyfile)
  PY_COMPILE(py${module} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module}")

  # install source sip files
  if(SIP_GLOBAL_INSTALL)
    file(GLOB sip_files ${CMAKE_CURRENT_BINARY_DIR}/${module}/*.sip)
    install(FILES ${sip_files} DESTINATION ${SIP_DEFAULT_SIP_DIR}/qgis/${module})
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${module}/auto_generated DESTINATION ${SIP_DEFAULT_SIP_DIR}/qgis/${module})
  endif()
endforeach(module)
