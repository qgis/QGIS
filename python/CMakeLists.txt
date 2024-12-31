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

IF(BUILD_WITH_QT6)
  SET(QSCI_SIP_MOD_NAME Qsci/qscimod6.sip)
  SET(BINDING_FILES_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/PyQt6)
ELSE()
  SET(QSCI_SIP_MOD_NAME Qsci/qscimod5.sip)
  SET(BINDING_FILES_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF()

set(QGIS_PYTHON_DIR ${Python_SITEARCH}/qgis)

# core module
file(GLOB_RECURSE sip_files_core ${BINDING_FILES_ROOT_DIR}/core/*.sip ${BINDING_FILES_ROOT_DIR}/core/*.sip.in)
set(SIP_EXTRA_FILES_DEPEND ${sip_files_core})
set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.core.api)
if((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
  set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n ${PYQT_SIP_IMPORT})
endif()
if(SIP_BUILD_EXECUTABLE)
  install(FILES ${CMAKE_BINARY_DIR}/python/core/build/_core/_core.pyi DESTINATION ${QGIS_PYTHON_DIR})
elseif((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
  set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_core.pyi)
  install(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_core.pyi DESTINATION ${QGIS_PYTHON_DIR})
endif()

if((${PYQT_VERSION_STR} VERSION_EQUAL 5.15) OR (${PYQT_VERSION_STR} VERSION_GREATER 5.15))
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} VECTOR_MAPPED_TYPE)
endif()

if(NOT WITH_GUI)
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} HAVE_GUI)
endif()

if(NOT WITH_QTSERIALPORT)
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} HAVE_QTSERIALPORT)
endif()

if(NOT WITH_QTPRINTER)
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} HAVE_QTPRINTER)
endif()

if(NOT WITH_QTWEBENGINE)
  set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} HAVE_WEBENGINE_SIP)
endif()

# Deprecated annotation supports message only since version 6.9.0 with abi-version 12.16 / 13.9 and above
if(${SIP_VERSION_STR} VERSION_LESS 6.9.0)
  set(sipabi "")
elseif(BUILD_WITH_QT6)
  set(sipabi "\n[tool.sip.project]\nabi-version = \"13.9\"")
else()
  set(sipabi "\n[tool.sip.project]\nabi-version = \"12.16\"")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/core/project.py.in ${CMAKE_CURRENT_BINARY_DIR}/core/project.py @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/core/pyproject.toml.in ${CMAKE_CURRENT_BINARY_DIR}/core/pyproject.toml @ONLY)
GENERATE_SIP_PYTHON_MODULE_CODE(qgis._core core/core.sip "${sip_files_core}" cpp_files)
BUILD_SIP_PYTHON_MODULE(qgis._core core/core.sip ${cpp_files} "" qgis_core)
set(SIP_CORE_CPP_FILES ${cpp_files})

# TODO QGIS 4 : remove this hack when we switch completely to Qt 6 which supports only c++17
if( ${QT_VERSION_MAJOR} EQUAL 5 AND ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" ) )
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

  if (NOT WITH_PDAL OR (WITH_PDAL AND NOT PDAL_2_5_OR_HIGHER))
    set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} HAVE_PDAL_PROVIDER)
  endif()
endif()

# gui module
if (WITH_GUI)
  set(PY_MODULES ${PY_MODULES} gui)

  file(GLOB_RECURSE sip_files_gui ${BINDING_FILES_ROOT_DIR}/gui/*.sip ${BINDING_FILES_ROOT_DIR}/gui/*.sip.in)
  set(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_gui})
  set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.gui.api)
  if((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n ${PYQT_SIP_IMPORT})
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

  if(NOT WITH_QTGAMEPAD)
    message(STATUS "QtGamepad not enabled - disabling bindings for derived classes")
    set(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} HAVE_QTGAMEPAD)
  endif()

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gui/project.py.in ${CMAKE_CURRENT_BINARY_DIR}/gui/project.py @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gui/pyproject.toml.in ${CMAKE_CURRENT_BINARY_DIR}/gui/pyproject.toml @ONLY)
  GENERATE_SIP_PYTHON_MODULE_CODE(qgis._gui gui/gui.sip "${sip_files_gui}" cpp_files)
  BUILD_SIP_PYTHON_MODULE(qgis._gui gui/gui.sip ${cpp_files} "" qgis_core qgis_gui)
endif()

# 3D module
if (WITH_3D)
  set(PY_MODULES ${PY_MODULES} 3d)

  file(GLOB_RECURSE sip_files_3d ${BINDING_FILES_ROOT_DIR}/3d/*.sip ${BINDING_FILES_ROOT_DIR}/3d/*.sip.in)
  set(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_3d})
  set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.3d_p.api)
  if((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n ${PYQT_SIP_IMPORT})
  endif()
  if(SIP_BUILD_EXECUTABLE)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/3d/build/_3d_p/_3d_p.pyi DESTINATION ${QGIS_PYTHON_DIR})
  elseif((${SIP_VERSION_STR} VERSION_EQUAL 4.18) OR (${SIP_VERSION_STR} VERSION_GREATER 4.18))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -y ${QGIS_PYTHON_OUTPUT_DIRECTORY}/3d_p.pyi)
    install(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/_3d_p.pyi DESTINATION ${QGIS_PYTHON_DIR})
  endif()

  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/3d/project.py.in ${CMAKE_CURRENT_BINARY_DIR}/3d/project.py @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/3d/pyproject.toml.in ${CMAKE_CURRENT_BINARY_DIR}/3d/pyproject.toml @ONLY)
  GENERATE_SIP_PYTHON_MODULE_CODE(qgis._3d_p 3d/3d.sip "${sip_files_3d}" cpp_files)
  BUILD_SIP_PYTHON_MODULE(qgis._3d_p 3d/3d.sip ${cpp_files} "" qgis_core qgis_3d)
endif()

# server module
if (WITH_SERVER AND WITH_SERVER_PLUGINS)
  include_directories(
    ../src/server
   ${CMAKE_BINARY_DIR}/src/server
  )

  set(PY_MODULES ${PY_MODULES} server)

  file(GLOB_RECURSE sip_files_server ${BINDING_FILES_ROOT_DIR}/server/*.sip ${BINDING_FILES_ROOT_DIR}/server/*.sip.in)
  set(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_server})
  set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.server.api)
  if((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n ${PYQT_SIP_IMPORT})
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

    if (WITH_PDAL AND NOT PDAL_2_5_OR_HIGHER)
       ${CMAKE_SOURCE_DIR}/src/analysis/processing/pdal
    endif()
  )

  # analysis module
  file(GLOB_RECURSE sip_files_analysis ${BINDING_FILES_ROOT_DIR}/analysis/*.sip ${BINDING_FILES_ROOT_DIR}/analysis/*.sip.in)

  set(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_analysis})
  set(SIP_EXTRA_OPTIONS ${PYQT_SIP_FLAGS} -g -o -a ${CMAKE_BINARY_DIR}/python/qgis.analysis.api)
  if((${SIP_VERSION_STR} VERSION_EQUAL 4.19.11) OR (${SIP_VERSION_STR} VERSION_GREATER 4.19.11))
    set(SIP_EXTRA_OPTIONS ${SIP_EXTRA_OPTIONS} -n ${PYQT_SIP_IMPORT})
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
  set(APIS_SRC_DIR "${CMAKE_SOURCE_DIR}/python/qsci_apis")
  set(APIS_OUT_DIR "${CMAKE_BINARY_DIR}/python/qsci_apis")

  # wait until after python module builds for api files to be available
  set(QGIS_PYTHON_API_FILE "${APIS_OUT_DIR}/PyQGIS.api")
  set(QGIS_PYTHON_PAP_FILE "${APIS_OUT_DIR}/PyQGIS.pap")

  set(api_modules)
  set(api_files)
  foreach(module ${PY_MODULES})
    set(module_out ${module})
    if(${module} EQUAL "3d")
      set(module_out "3d_p")
    endif()
    set(api_modules ${api_modules} python_module_qgis__${module_out})
    if(NOT SIP_BUILD_EXECUTABLE)
      set(api_files ${api_files} "${CMAKE_BINARY_DIR}/python/qgis.${module}.api")
    else()
      set(api_files ${api_files} "${CMAKE_BINARY_DIR}/python/${module}/build/QGIS.api")
    endif()
  endforeach()

  # run update/concatenate command
  add_custom_command(
    OUTPUT "${QGIS_PYTHON_API_FILE}"
    DEPENDS ${api_modules}
    COMMAND "${CMAKE_COMMAND}" -D "APIS=${api_files}" -P "${CMAKE_SOURCE_DIR}/cmake/QsciAPI.cmake"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Generating pyqgis api file" VERBATIM)

  # generate a .pap file to be immediately installed in QGIS source tree (the default .pap)
  if(NATIVE_Python_EXECUTABLE)
     set(_python ${NATIVE_Python_EXECUTABLE})
  else()
     set(_python ${Python_EXECUTABLE})
  endif()

  if (QSCI_SIP_DIR)
    configure_file("${APIS_SRC_DIR}/generate_console_pap.py.in" ${CMAKE_BINARY_DIR}/generate_console_pap.py)

    add_custom_command(
      OUTPUT "${QGIS_PYTHON_PAP_FILE}"
      DEPENDS "${QGIS_PYTHON_API_FILE}"
      COMMAND ${_python} "${CMAKE_BINARY_DIR}/generate_console_pap.py" -platform offscreen "${QGIS_PYTHON_PAP_FILE}" "${APIS_SRC_DIR}" "${APIS_OUT_DIR}"
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      COMMENT "Generating pap file for console auto-completion" VERBATIM)

    ADD_QGIS_RESOURCES("${APIS_OUT_DIR}" "python/qsci_apis" DEST_DATA_FILES "PyQGIS.api;PyQGIS.pap")

    add_custom_target(pyqgis-pap ALL DEPENDS ${QGIS_PYTHON_API_FILE} ${QGIS_PYTHON_PAP_FILE} ${DEST_DATA_FILES})
  endif()

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
    # DEPENDS ${pyfile}
  )
  PY_COMPILE(pyutils "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${pyfile}")
endforeach(pyfile)

foreach(module ${PY_MODULES})
  set(module_out ${module})
  if(${module} EQUAL "3d")
    set(module_out "3d_p")
  endif()
  set(module_name ${module})
  if(${module} EQUAL "3d")
    set(module_name "_3d")
  endif()
  add_custom_target(py${module} ALL)
  add_dependencies(py${module} python_module_qgis__${module_out})

  # concat auto_additions/*.py in _module_/__init__.py
  file(GLOB PY_FILES_AUTO_ADDITIONS ${BINDING_FILES_ROOT_DIR}/${module}/auto_additions/*.py)
  set(MONKEYPATCH_INJECTIONS "")
  foreach(pyfile ${PY_FILES_AUTO_ADDITIONS})
    file(READ ${pyfile} CONTENTS)
    string(APPEND MONKEYPATCH_INJECTIONS "${CONTENTS}")
  endforeach(pyfile)
  configure_file(${BINDING_FILES_ROOT_DIR}/${module}/__init__.py.in ${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module_name}/__init__.py @ONLY)
  install(FILES ${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module_name}/__init__.py DESTINATION "${QGIS_PYTHON_DIR}/${module_name}")

  file(GLOB PY_FILES ${BINDING_FILES_ROOT_DIR}/${module}/*.py)
  install(FILES ${PY_FILES} DESTINATION "${QGIS_PYTHON_DIR}/${module_name}")
  file(GLOB PY_FILES_ADDITIONS ${BINDING_FILES_ROOT_DIR}/${module}/additions/*.py)

  install(FILES ${PY_FILES_ADDITIONS} DESTINATION "${QGIS_PYTHON_DIR}/${module_name}/additions")
  set(PY_FILES ${PY_FILES} ${PY_FILES_ADDITIONS})
  foreach(pyfile ${PY_FILES})
    file(RELATIVE_PATH rel_pyfile ${BINDING_FILES_ROOT_DIR} ${pyfile})
    get_filename_component(subdir ${rel_pyfile} DIRECTORY)

    add_custom_command(TARGET py${module}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${subdir}"
      COMMAND ${CMAKE_COMMAND} -E copy ${pyfile} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${subdir}"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      # DEPENDS ${PY_FILES}
    )
  endforeach(pyfile)
  PY_COMPILE(py${module} "${QGIS_PYTHON_OUTPUT_DIRECTORY}/${module_name}")

  # install class map file
  install(FILES ${BINDING_FILES_ROOT_DIR}/${module}/class_map.yaml DESTINATION "${QGIS_PYTHON_DIR}/${module_name}")

  # install source sip files
  if(SIP_GLOBAL_INSTALL)
    file(GLOB sip_files ${CMAKE_CURRENT_BINARY_DIR}/${module}/*.sip)
    install(FILES ${sip_files} DESTINATION ${SIP_DEFAULT_SIP_DIR}/qgis/${module})
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${module}/auto_generated DESTINATION ${SIP_DEFAULT_SIP_DIR}/qgis/${module})
  endif()
endforeach(module)
