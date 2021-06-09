########################################################
# Files

set(VRP_SRCS
    qgsvirtualrasterprovider.cpp
)
set(VRP_HDRS
    qgsvirtualrasterprovider.h
)

#[[
if (WITH_GUI)
  set(DB2_SRCS ${DB2_SRCS}
    qgsdb2providergui.cpp
    qgsdb2dataitemguiprovider.cpp
    qgsdb2newconnection.cpp
    qgsdb2sourceselect.cpp
  )
  set(DB2_HDRS ${DB2_HDRS}
    qgsdb2dataitemguiprovider.h
    qgsdb2newconnection.h
    qgsdb2sourceselect.h
  )
endif()
#]]

########################################################
# Build

include_directories(
  SYSTEM
)

add_library (virtualrasterprovider MODULE ${VRP_SRCS} ${VRP_HDRS})

# require c++17
target_compile_features(virtualrasterprovider PRIVATE cxx_std_17)

target_link_libraries (virtualrasterprovider
  qgis_core
  qgis_analysis
)

#[[
if (WITH_GUI)
  target_link_libraries (db2provider
    qgis_gui
  )
  add_dependencies(db2provider ui)
endif()
]]

target_compile_definitions(virtualrasterprovider PRIVATE "-DQT_NO_FOREACH")

# clang-tidy
if(CLANG_TIDY_EXE)
  set_target_properties(
    virtualrasterprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
endif()

########################################################
# Install

install(TARGETS virtualrasterprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
