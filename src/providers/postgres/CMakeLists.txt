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
add_library (provider_postgres_a STATIC ${PG_SRCS} ${PG_HDRS})

# require c++17
target_compile_features(provider_postgres_a PRIVATE cxx_std_17)

target_link_libraries (provider_postgres_a
  qgis_core
  ${POSTGRES_LIBRARY}
)

if (WITH_GUI)
  if (WITH_QT6)
    QT6_WRAP_UI(PG_UIS_H ${PG_UIS})
  else()
    QT5_WRAP_UI(PG_UIS_H ${PG_UIS})
  endif()

  add_library(provider_postgres_gui_a STATIC ${PG_GUI_SRCS} ${PG_UIS_H})

  # require c++17
  target_compile_features(provider_postgres_gui_a PRIVATE cxx_std_17)

  target_link_libraries(provider_postgres_gui_a
    qgis_gui
    provider_postgres_a
    ${POSTGRES_LIBRARY}
  )
  add_dependencies(provider_postgres_gui_a ui)
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
add_library (provider_postgresraster_a STATIC ${PGRASTER_SRCS} ${PG_HDRS})

# require c++17
target_compile_features(provider_postgresraster_a PRIVATE cxx_std_17)

target_link_libraries (provider_postgresraster_a
    qgis_core
    ${POSTGRES_LIBRARY}
    ${${QT_VERSION_BASE}Xml_LIBRARIES}
    ${${QT_VERSION_BASE}Core_LIBRARIES}
    ${${QT_VERSION_BASE}Svg_LIBRARIES}
    ${${QT_VERSION_BASE}Network_LIBRARIES}
    ${${QT_VERSION_BASE}Sql_LIBRARIES}
    ${${QT_VERSION_BASE}Concurrent_LIBRARIES}
    ${${QT_VERSION_BASE}Gui_LIBRARIES}
    ${${QT_VERSION_BASE}Widgets_LIBRARIES}
)

if (WITH_GUI)
  target_link_libraries (provider_postgresraster_a
    qgis_gui
  )
  add_dependencies(provider_postgresraster_a ui)
endif()

#################################################################

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_postgres_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  install (TARGETS provider_postgresraster_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS provider_postgres_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  # dynamically loaded module
  add_library(provider_postgres MODULE ${PG_SRCS} ${PG_GUI_SRCS} ${PG_HDRS})
  add_library(provider_postgresraster MODULE ${PGRASTER_SRCS} ${PG_HDRS})

  # require c++17
  target_compile_features(provider_postgres PRIVATE cxx_std_17)
  target_compile_features(provider_postgresraster PRIVATE cxx_std_17)

  target_link_libraries(provider_postgres
    ${POSTGRES_LIBRARY}
    qgis_core
  )

  target_link_libraries(provider_postgresraster
    ${POSTGRES_LIBRARY}
    qgis_core
  )

  if (WITH_GUI)
    target_link_libraries (provider_postgres
      qgis_gui
    )
    add_dependencies(provider_postgres ui)
  endif()


  # clang-tidy
  if(CLANG_TIDY_EXE)
    set_target_properties(
      provider_postgres PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
    set_target_properties(
      provider_postgresraster PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
  endif()

  install(TARGETS provider_postgres
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

  install(TARGETS provider_postgresraster
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})


endif()
