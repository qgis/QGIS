
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

set(GEONODE_HDRS
  qgsgeonodeprovider.h
)

########################################################
# Build

include_directories(

  ${CMAKE_BINARY_DIR}/src/ui
)

add_library(provider_geonode MODULE ${GEONODE_SRCS} ${GEONODE_HDRS})

# require c++17
target_compile_features(provider_geonode PRIVATE cxx_std_17)

target_link_libraries(provider_geonode
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (provider_geonode
    qgis_gui
  )
  add_dependencies(provider_geonode ui)
endif()

target_compile_definitions(provider_geonode PRIVATE "-DQT_NO_FOREACH")

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    provider_geonode PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

install (TARGETS provider_geonode
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
