
set (GEONODE_SRCS
  qgsgeonodeprovider.cpp
  qgsgeonodedataitems.cpp
)

if (WITH_GUI)
  set(GEONODE_SRCS ${GEONODE_SRCS}
    qgsgeonodeprovidergui.cpp
    qgsgeonodedataitemguiprovider.cpp
    qgsgeonodenewconnection.cpp
    qgsgeonodesourceselect.cpp
  )
endif()

########################################################
# Build

include_directories(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
include_directories(SYSTEM
)

add_library(geonodeprovider MODULE ${GEONODE_SRCS})

target_link_libraries(geonodeprovider
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (geonodeprovider
    qgis_gui
  )
  add_dependencies(geonodeprovider ui)
endif()

target_compile_definitions(geonodeprovider PRIVATE "-DQT_NO_FOREACH")

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    geonodeprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

install (TARGETS geonodeprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

