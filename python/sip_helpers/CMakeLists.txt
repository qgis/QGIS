SET (QGIS_SIP_HELPER_SRCS
  qgissiphelper.cpp
)

SET (QGIS_SIP_HELPER_HDRS
  qgissiphelper.h
)

INCLUDE_DIRECTORIES(SYSTEM
  ${PYTHON_INCLUDE_PATH}
  ${SIP_INCLUDE_DIR}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Xml_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5PrintSupport_INCLUDE_DIRS}
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}/../core
  ../../src/core
  ../../src/core/auth
  ../../src/core/pal
  ../../src/core/composer
  ../../src/core/diagram
  ../../src/core/effects
  ../../src/core/dxf
  ../../src/core/geometry
  ../../src/core/gps
  ../../src/core/layertree
  ../../src/core/raster
  ../../src/core/symbology-ng
  .
)

ADD_LIBRARY(qgis_sip_helpers OBJECT ${QGIS_SIP_HELPER_SRCS} ${QGIS_SIP_HELPER_HDRS})
ADD_DEPENDENCIES(qgis_sip_helpers generate_sip_qgis._core_cpp_files)
SET_TARGET_PROPERTIES(qgis_sip_helpers PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
