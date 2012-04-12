ADD_SUBDIRECTORY(plugins)

SET (PYTHON_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/python)
SET (QGIS_PYTHON_OUTPUT_DIRECTORY ${PYTHON_OUTPUT_DIRECTORY}/qgis)

IF (WITH_PYSPATIALITE)
  ADD_SUBDIRECTORY(pyspatialite)
ENDIF (WITH_PYSPATIALITE)

IF (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(
    ../src/core/spatialite/headers
    ../src/core/spatialite/headers/spatialite
  )
ENDIF (WITH_INTERNAL_SPATIALITE)

SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY})
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${QGIS_PYTHON_OUTPUT_DIRECTORY})

INCLUDE_DIRECTORIES(
    ${PYTHON_INCLUDE_PATH}
    ${SIP_INCLUDE_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTNETWORK_INCLUDE_DIR}
    ${QT_QTSVG_INCLUDE_DIR}
    ${QT_QTXML_INCLUDE_DIR}
    ${GDAL_INCLUDE_DIR}
    ${GEOS_INCLUDE_DIR}

    ../src/core
    ../src/core/composer
    ../src/core/gps
    ../src/core/gps/qextserialport
    ../src/core/raster
    ../src/core/renderer
    ../src/core/spatialindex
    ../src/core/symbology
    ../src/core/symbology-ng
    ${CMAKE_BINARY_DIR}  # qgsconfig.h, qgsversion.h
)


# In Windef.h there are min() and max() macros that interfere with the usage
# of std::numeric_limits<T>::min() and :max() in qgsrasterbands.h.
IF(MSVC)
  ADD_DEFINITIONS(-DNOMINMAX)
ENDIF(MSVC)

IF(PYQT4_VERSION_NUM LESS 263680)	# 0x040600
  SET(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} PROXY_FACTORY)
ENDIF(PYQT4_VERSION_NUM LESS 263680)

IF(NOT PYQT4_VERSION_NUM LESS 263941)	# 0x040705
  SET(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} QSETINT_CONVERSION)
ENDIF(NOT PYQT4_VERSION_NUM LESS 263941)

IF(NOT PYQT4_VERSION_NUM LESS 264194)	# 0x040802
  SET(SIP_DISABLE_FEATURES ${SIP_DISABLE_FEATURES} QSETTYPE_CONVERSION)
ENDIF(NOT PYQT4_VERSION_NUM LESS 264194)


# core module
FILE(GLOB sip_files_core core/*.sip)
SET(SIP_EXTRA_FILES_DEPEND ${sip_files_core})
SET(SIP_EXTRA_OPTIONS ${PYQT4_SIP_FLAGS} -o -a ${CMAKE_BINARY_DIR}/python/qgis.core.api)
ADD_SIP_PYTHON_MODULE(qgis.core core/core.sip qgis_core)

# additional gui includes
INCLUDE_DIRECTORIES(
    ../src/gui
    ../src/gui/symbology-ng
    ../src/plugins
    ${CMAKE_BINARY_DIR}/src/gui
    ${CMAKE_BINARY_DIR}/src/ui
)

# gui module
FILE(GLOB sip_files_gui gui/*.sip)
SET(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_gui})
SET(SIP_EXTRA_OPTIONS ${PYQT4_SIP_FLAGS} -o -a ${CMAKE_BINARY_DIR}/python/qgis.gui.api)
ADD_SIP_PYTHON_MODULE(qgis.gui gui/gui.sip qgis_core qgis_gui)

# additional analysis includes
INCLUDE_DIRECTORIES(
    ../src/analysis/vector
    ../src/analysis/network
    ${CMAKE_BINARY_DIR}/src/analysis/vector
    ${CMAKE_BINARY_DIR}/src/analysis/network
)

# analysis module
FILE(GLOB sip_files_analysis analysis/*.sip)
SET(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_analysis})
SET(SIP_EXTRA_OPTIONS ${PYQT4_SIP_FLAGS} -o -a ${CMAKE_BINARY_DIR}/python/qgis.analysis.api)
ADD_SIP_PYTHON_MODULE(qgis.analysis analysis/analysis.sip qgis_core qgis_analysis)

# network-analysis module
FILE(GLOB sip_files_network_analysis analysis/network/*.sip)
SET(SIP_EXTRA_FILES_DEPEND ${sip_files_core} ${sip_files_network_analysis})
SET(SIP_EXTRA_OPTIONS ${PYQT4_SIP_FLAGS} -o -a ${CMAKE_BINARY_DIR}/python/qgis.networkanalysis.api)
ADD_SIP_PYTHON_MODULE(qgis.networkanalysis analysis/network/networkanalysis.sip qgis_core qgis_networkanalysis)

SET (QGIS_PYTHON_DIR ${PYTHON_SITE_PACKAGES_DIR}/qgis)

ADD_CUSTOM_TARGET(compile_python_files ALL)

ADD_CUSTOM_COMMAND(TARGET compile_python_files
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E make_directory ${QGIS_PYTHON_OUTPUT_DIRECTORY}
       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

FOREACH(file __init__.py utils.py console.py)
  ADD_CUSTOM_COMMAND(TARGET compile_python_files
         POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy ${file} ${QGIS_PYTHON_OUTPUT_DIRECTORY}
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	 DEPENDS ${file}
  )
ENDFOREACH(file)

PYTHON_INSTALL(__init__.py ${QGIS_PYTHON_DIR})
PYTHON_INSTALL(utils.py    ${QGIS_PYTHON_DIR})
PYTHON_INSTALL(console.py  ${QGIS_PYTHON_DIR})
