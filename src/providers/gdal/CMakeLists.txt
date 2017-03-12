SET(GDAL_SRCS
  qgsgdalproviderbase.cpp
  qgsgdalprovider.cpp
  qgsgdaldataitems.cpp
)
SET(GDAL_MOC_HDRS
  qgsgdalprovider.h
  qgsgdaldataitems.h
)

INCLUDE_DIRECTORIES (
  ../../core
  ../../core/raster
  ../../core/geometry
  ../../gui
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
)
INCLUDE_DIRECTORIES (SYSTEM
  ${GDAL_INCLUDE_DIR}
#  ${PROJ_INCLUDE_DIR}
#  ${GEOS_INCLUDE_DIR}
)

QT5_WRAP_CPP(GDAL_MOC_SRCS ${GDAL_MOC_HDRS})
ADD_LIBRARY (gdalprovider MODULE ${GDAL_SRCS} ${GDAL_MOC_SRCS})

TARGET_LINK_LIBRARIES (gdalprovider
  qgis_core
  qgis_gui
)

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    gdalprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

INSTALL(TARGETS gdalprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

