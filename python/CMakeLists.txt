SUBDIRS(plugins)
IF (WIN32)
  SET(BINDINGS_CORE_LIB     ${CMAKE_CURRENT_BINARY_DIR}/core/core.pyd)
  SET(BINDINGS_GUI_LIB      ${CMAKE_CURRENT_BINARY_DIR}/gui/gui.pyd)
  SET(BINDINGS_ANALYSIS_LIB ${CMAKE_CURRENT_BINARY_DIR}/analysis/analysis.pyd)
  IF (NOT MSVC)
    SET(QGIS_CORE_LIB     ${CMAKE_BINARY_DIR}/src/core/libqgis_core.dll)
    SET(QGIS_GUI_LIB      ${CMAKE_BINARY_DIR}/src/gui/libqgis_gui.dll)
    SET(QGIS_ANALYSIS_LIB ${CMAKE_BINARY_DIR}/src/analysis/libqgis_analysis.dll)
  ELSE (NOT MSVC)
    SET(QGIS_CORE_LIB     ${CMAKE_BINARY_DIR}/src/core/${CMAKE_CFG_INTDIR}/qgis_core.lib)
    SET(QGIS_GUI_LIB      ${CMAKE_BINARY_DIR}/src/gui/${CMAKE_CFG_INTDIR}/qgis_gui.lib)
    SET(QGIS_ANALYSIS_LIB ${CMAKE_BINARY_DIR}/src/analysis/${CMAKE_CFG_INTDIR}/qgis_analysis.lib)
  ENDIF (NOT MSVC)
ELSE (WIN32)
  SET(BINDINGS_CORE_LIB     ${CMAKE_CURRENT_BINARY_DIR}/core/core.so)
  SET(BINDINGS_GUI_LIB      ${CMAKE_CURRENT_BINARY_DIR}/gui/gui.so)
  SET(BINDINGS_ANALYSIS_LIB ${CMAKE_CURRENT_BINARY_DIR}/analysis/analysis.so)
  IF (APPLE)
    SET(QGIS_CORE_LIB     ${CMAKE_BINARY_DIR}/src/core/libqgis_core.dylib)
    SET(QGIS_GUI_LIB      ${CMAKE_BINARY_DIR}/src/gui/libqgis_gui.dylib)
    SET(QGIS_ANALYSIS_LIB ${CMAKE_BINARY_DIR}/src/analysis/libqgis_analysis.dylib)
  ELSE (APPLE)
    SET(QGIS_CORE_LIB     ${CMAKE_BINARY_DIR}/src/core/libqgis_core.so)
    SET(QGIS_GUI_LIB      ${CMAKE_BINARY_DIR}/src/gui/libqgis_gui.so)
    SET(QGIS_ANALYSIS_LIB ${CMAKE_BINARY_DIR}/src/analysis/libqgis_analysis.so)
  ENDIF (APPLE)
ENDIF (WIN32)

SET (BINDINGS_LIBS ${BINDINGS_CORE_LIB} ${BINDINGS_GUI_LIB} ${BINDINGS_ANALYSIS_LIB})
SET (BINDINGS_CORE_MAKEFILE     ${CMAKE_CURRENT_BINARY_DIR}/core/Makefile)
SET (BINDINGS_GUI_MAKEFILE      ${CMAKE_CURRENT_BINARY_DIR}/gui/Makefile)
SET (BINDINGS_ANALYSIS_MAKEFILE ${CMAKE_CURRENT_BINARY_DIR}/analysis/Makefile)

# 'python' target will force to build bindings libs for core and gui
ADD_CUSTOM_TARGET (python ALL DEPENDS ${BINDINGS_CORE_LIB} ${BINDINGS_GUI_LIB} ${BINDINGS_ANALYSIS_LIB})

# don't run python before the libs are built
ADD_DEPENDENCIES (python qgis_core qgis_gui qgis_analysis)

FILE(GLOB CORE_SIP_FILES     "${CMAKE_CURRENT_SOURCE_DIR}/core/*.sip")
FILE(GLOB GUI_SIP_FILES      "${CMAKE_CURRENT_SOURCE_DIR}/gui/*.sip")
FILE(GLOB ANALYSIS_SIP_FILES "${CMAKE_CURRENT_SOURCE_DIR}/analysis/*.sip")

# Step 1: during configuration
# create file configure.py from configure.py.in
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/configure.py.in
               ${CMAKE_CURRENT_BINARY_DIR}/configure.py)
               
IF (MSVC)
  SET(EXPORT "__declspec(dllimport)")
ELSE (MSVC)
  SET(EXPORT "")
ENDIF (MSVC)

# Step 2: during make
# run python configure.py
# it will run SIP utility to generate sources and will prepare makefiles
# should be run everytime core or gui library has been changed
ADD_CUSTOM_COMMAND(OUTPUT ${BINDINGS_CORE_MAKEFILE} ${BINDINGS_GUI_MAKEFILE} ${BINDINGS_ANALYSIS_MAKEFILE} PRE_BUILD
                   COMMAND ${PYTHON_EXECUTABLE}
                   ARGS ${CMAKE_CURRENT_BINARY_DIR}/configure.py ${CMAKE_CFG_INTDIR} ${EXPORT}
                   DEPENDS ${QGIS_CORE_LIB} ${QGIS_GUI_LIB} ${QGIS_ANALYSIS_LIB}
                           ${CMAKE_CURRENT_BINARY_DIR}/configure.py
                           ${CORE_SIP_FILES} ${GUI_SIP_FILES} ${ANALYSIS_SIP_FILES})

# Step 3: run make in core and gui subdirs
ADD_CUSTOM_COMMAND(OUTPUT ${BINDINGS_CORE_LIB} PRE_LINK
                   COMMAND ${SIP_MAKE_PROGRAM}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/core
                   DEPENDS ${BINDINGS_CORE_MAKEFILE})
ADD_CUSTOM_COMMAND(OUTPUT ${BINDINGS_GUI_LIB} PRE_LINK
                   COMMAND ${SIP_MAKE_PROGRAM}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gui
                   DEPENDS ${BINDINGS_GUI_MAKEFILE})
ADD_CUSTOM_COMMAND(OUTPUT ${BINDINGS_ANALYSIS_LIB} PRE_LINK
                   COMMAND ${SIP_MAKE_PROGRAM}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/analysis
                   DEPENDS ${BINDINGS_ANALYSIS_MAKEFILE})

IF (BINDINGS_GLOBAL_INSTALL)

# python's site-packages dir: bindings will be installed here
IF (UNIX)
  SET (CMD "from distutils.sysconfig import get_python_lib; print get_python_lib(1)") 
ELSE (UNIX)
  SET (CMD "
import sys
print sys.exec_prefix + '/lib/site-packages'
")
ENDIF (UNIX)

EXEC_PROGRAM(${PYTHON_EXECUTABLE} ARGS -c "\"${CMD}\"" OUTPUT_VARIABLE SITE_PKG_PATH)

ELSE (BINDINGS_GLOBAL_INSTALL)

  SET (SITE_PKG_PATH ${QGIS_DATA_DIR}/python)
  
ENDIF (BINDINGS_GLOBAL_INSTALL)


# Step 4: install built libs to python's site packages
INSTALL(FILES __init__.py utils.py ${CMAKE_CURRENT_BINARY_DIR}/qgisconfig.py ${BINDINGS_LIBS} DESTINATION ${SITE_PKG_PATH}/qgis)

