if(WIN32)
  add_definitions(-DQWT_DLL)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann
  ${CMAKE_SOURCE_DIR}/tests/core #for render checker class
  ${CMAKE_SOURCE_DIR}/src/3d
  ${CMAKE_SOURCE_DIR}/src/3d/chunks
  ${CMAKE_SOURCE_DIR}/src/3d/materials
  ${CMAKE_SOURCE_DIR}/src/3d/mesh
  ${CMAKE_SOURCE_DIR}/src/3d/symbols
  ${CMAKE_SOURCE_DIR}/src/3d/terrain
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/3d
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/labeling
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/mesh
  ${CMAKE_SOURCE_DIR}/src/core/pointcloud
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/fieldformatter
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_SOURCE_DIR}/src/native

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/3d
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/native
  ${CMAKE_CURRENT_BINARY_DIR}

  ${CMAKE_SOURCE_DIR}/src/app/3d
  ${CMAKE_BINARY_DIR}/src/app
  ${QWT_INCLUDE_DIR}
)

include_directories(SYSTEM
  ${QT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${QT5_3DEXTRA_INCLUDE_DIR}
)

add_executable(qgis_3d_sandbox
  qgis_3d_sandbox.cpp
  ${CMAKE_SOURCE_DIR}/src/app/3d/qgs3dmapcanvas.cpp
  ${CMAKE_SOURCE_DIR}/src/app/3d/qgs3dnavigationwidget.cpp
)
set_target_properties(qgis_3d_sandbox PROPERTIES AUTORCC TRUE)
target_link_libraries(qgis_3d_sandbox
  ${Qt5Xml_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Test_LIBRARIES}
  ${PROJ_LIBRARY}
  ${GEOS_LIBRARY}
  ${GDAL_LIBRARY}
  ${QWT_LIBRARY}
  qgis_core
  qgis_3d
  qgis_native)
