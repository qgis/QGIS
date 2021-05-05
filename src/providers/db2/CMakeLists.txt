
########################################################
# Files

set(DB2_SRCS
  qgsdb2geometrycolumns.cpp
  qgsdb2expressioncompiler.cpp
  qgsdb2provider.cpp
  qgsdb2dataitems.cpp
  qgsdb2tablemodel.cpp
  qgsdb2featureiterator.cpp
)
set(DB2_HDRS
  qgsdb2provider.h
  qgsdb2dataitems.h
  qgsdb2tablemodel.h
)

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

########################################################
# Build

include_directories(
  ${CMAKE_SOURCE_DIR}/src/ui

  ${CMAKE_BINARY_DIR}/src/ui
)

add_library (db2provider MODULE ${DB2_SRCS} ${DB2_HDRS})

# require c++17
target_compile_features(db2provider PRIVATE cxx_std_17)

target_link_libraries (db2provider
  qgis_core
  ${Qt5Sql_LIBRARIES}
)

if (WITH_GUI)
  target_link_libraries (db2provider
    qgis_gui
  )
  add_dependencies(db2provider ui)
endif()

########################################################
# Install

install(TARGETS db2provider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
