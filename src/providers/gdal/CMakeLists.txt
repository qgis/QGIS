SET(GDAL_SRCS
  qgsgdalproviderbase.cpp
  qgsgdalprovider.cpp
  qgsgdaldataitems.cpp
)
SET(GDAL_MOC_HDRS
  qgsgdalprovider.h
  qgsgdaldataitems.h
)

IF (WITH_GUI)
  SET(GDAL_SRCS ${GDAL_SRCS}
    qgsgdalsourceselect.cpp
  )
  SET(GDAL_MOC_HDRS ${GDAL_MOC_HDRS}
    qgsgdalsourceselect.h
  )
ENDIF ()

########################################################

INCLUDE_DIRECTORIES (
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
INCLUDE_DIRECTORIES (SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
)

QT5_WRAP_CPP(GDAL_MOC_SRCS ${GDAL_MOC_HDRS})
ADD_LIBRARY (gdalprovider MODULE ${GDAL_SRCS} ${GDAL_MOC_SRCS})

TARGET_LINK_LIBRARIES (gdalprovider
  qgis_core
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (gdalprovider
    qgis_gui
  )
ENDIF ()

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

