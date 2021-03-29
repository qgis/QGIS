########################################################
# Files

set(PG_SRCS
  qgspostgresprovider.cpp
  qgspostgresconn.cpp
  qgspostgresconnpool.cpp
  qgspostgresdataitems.cpp
  qgspostgresfeatureiterator.cpp
  qgspostgresprojectstorage.cpp
  qgspostgrestransaction.cpp
  qgspgtablemodel.cpp
  qgscolumntypethread.cpp
  qgspostgresexpressioncompiler.cpp
  qgspostgreslistener.cpp
  qgspostgresproviderconnection.cpp
)

if (WITH_GUI)
  set(PG_GUI_SRCS
    qgspostgresprovidergui.cpp
    qgspostgresdataitemguiprovider.cpp
    qgspgsourceselect.cpp
    qgspgnewconnection.cpp
    qgspostgresprojectstoragedialog.cpp
    raster/qgspostgresrastertemporalsettingswidget.cpp
  )

  set(PG_UIS qgspostgresrastertemporalsettingswidgetbase.ui)
endif()

set(PG_HDRS
  qgspostgresexpressioncompiler.h
  qgspostgresproviderconnection.h
)

########################################################
# Build

include_directories(SYSTEM
  ${POSTGRES_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${SPATIALINDEX_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WITH_GUI)
  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
  include_directories (
      ${CMAKE_BINARY_DIR}/src/providers/postgres
      ${CMAKE_SOURCE_DIR}/src/providers/postgres
      ${CMAKE_BINARY_DIR}/src/providers/postgres/raster
      ${CMAKE_SOURCE_DIR}/src/providers/postgres/raster
      ${CMAKE_CURRENT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/src/ui
  )
endif()

# static library
add_library (postgresprovider_a STATIC ${PG_SRCS} ${PG_HDRS})

# require c++17
target_compile_features(postgresprovider_a PRIVATE cxx_std_17)

target_link_libraries (postgresprovider_a
  qgis_core
  ${POSTGRES_LIBRARY}
  ${Qt5Xml_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Sql_LIBRARIES}
  ${Qt5Concurrent_LIBRARIES}
)

if (WITH_GUI)
  QT5_WRAP_UI(PG_UIS_H ${PG_UIS})

  add_library(postgresprovider_gui_a STATIC ${PG_GUI_SRCS} ${PG_UIS_H})

  # require c++17
  target_compile_features(postgresprovider_gui_a PRIVATE cxx_std_17)

  target_link_libraries(postgresprovider_gui_a
    qgis_gui
    postgresprovider_a
    ${POSTGRES_LIBRARY}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
  )
  add_dependencies(postgresprovider_gui_a ui)
endif()

#################################################################
# Postgres Raster

set(PGRASTER_SRCS
  raster/qgspostgresrasterprovider.cpp
  raster/qgspostgresrastershareddata.cpp
  raster/qgspostgresrasterutils.cpp
  qgspostgresconn.cpp
  qgspostgresconnpool.cpp
)

# static library
add_library (postgresrasterprovider_a STATIC ${PGRASTER_SRCS} ${PG_HDRS})

# require c++17
target_compile_features(postgresrasterprovider_a PRIVATE cxx_std_17)

target_link_libraries (postgresrasterprovider_a
    qgis_core
    ${POSTGRES_LIBRARY}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Sql_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
)

if (WITH_GUI)
  target_link_libraries (postgresrasterprovider_a
    qgis_gui
  )
  add_dependencies(postgresrasterprovider_a ui)
endif()

#################################################################

if (FORCE_STATIC_PROVIDERS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS postgresprovider_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  install (TARGETS postgresrasterprovider_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS postgresprovider_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  # dynamically loaded module
  add_library(postgresprovider MODULE ${PG_SRCS} ${PG_GUI_SRCS} ${PG_HDRS})
  add_library(postgresrasterprovider MODULE ${PGRASTER_SRCS} ${PG_HDRS})

  # require c++17
  target_compile_features(postgresprovider PRIVATE cxx_std_17)
  target_compile_features(postgresrasterprovider PRIVATE cxx_std_17)

  target_link_libraries(postgresprovider
    ${POSTGRES_LIBRARY}
    qgis_core
  )

  target_link_libraries(postgresrasterprovider
    ${POSTGRES_LIBRARY}
    qgis_core
  )

  if (WITH_GUI)
    target_link_libraries (postgresprovider
      qgis_gui
    )
    add_dependencies(postgresprovider ui)
  endif()


  # clang-tidy
  if(CLANG_TIDY_EXE)
    set_target_properties(
      postgresprovider PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
    set_target_properties(
      postgresrasterprovider PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
  endif()

  install(TARGETS postgresprovider
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

  install(TARGETS postgresrasterprovider
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})


endif()
