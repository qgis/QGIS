
########################################################
# Files

SET (wms_SRCS
  qgswms.cpp
  qgswmsutils.cpp
  qgsdxfwriter.cpp
  qgswmsdescribelayer.cpp
  qgswmsgetcapabilities.cpp
  qgswmsgetcontext.cpp
  qgswmsgetfeatureinfo.cpp
  qgswmsgetlegendgraphics.cpp
  qgswmsgetmap.cpp
  qgswmsgetprint.cpp
  qgswmsgetschemaextension.cpp
  qgswmsgetstyle.cpp
  qgswmsgetstyles.cpp
  qgswmsservertransitional.cpp
  qgsmaprendererjobproxy.cpp
  qgsfilterrestorer.cpp
)

########################################################
# Build

ADD_LIBRARY (wms MODULE ${wms_SRCS})


INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/server
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../../core
  ../../../core/dxf
  ../../../core/geometry
  ../../../core/raster
  ../../../core/symbology-ng
  ../../../core/composer
  ../../../core/layertree
  ../../../gui
  ../../../gui/editorwidgets
  ../../../gui/editorwidgets/core
  ../..
  ..
  .
)


#endif
TARGET_LINK_LIBRARIES(wms
  qgis_core
  qgis_server
)


########################################################
# Install

INSTALL(TARGETS wms
    RUNTIME DESTINATION ${QGIS_SERVER_MODULE_DIR}
    LIBRARY DESTINATION ${QGIS_SERVER_MODULE_DIR}
)

