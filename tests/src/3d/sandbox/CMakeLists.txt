if(WIN32)
  add_definitions(-DQWT_DLL)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tests/core #for render checker class
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_SOURCE_DIR}/src/native

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
# require c++17
target_compile_features(qgis_3d_sandbox PRIVATE cxx_std_17)

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
