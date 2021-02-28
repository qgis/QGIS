
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

  ${CMAKE_BINARY_DIR}/src/ui
)

add_library(geonodeprovider MODULE ${GEONODE_SRCS})

# require c++17
target_compile_features(geonodeprovider PRIVATE cxx_std_17)

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

