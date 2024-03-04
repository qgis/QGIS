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
  qgspostgreslayermetadataprovider.cpp
  qgspostgresprovidermetadatautils.cpp
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

  set(PG_UIS ${CMAKE_SOURCE_DIR}/src/ui/qgspostgresrastertemporalsettingswidgetbase.ui)
endif()

set(PG_HDRS
  qgspostgresexpressioncompiler.h
  qgspostgresproviderconnection.h
)

########################################################
# Build

# static library
add_library (provider_postgres_a STATIC ${PG_SRCS} ${PG_HDRS})

target_include_directories(provider_postgres_a SYSTEM PUBLIC
  ${POSTGRES_INCLUDE_DIR}
)

target_include_directories(provider_postgres_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/postgres
)

# require c++17
target_compile_features(provider_postgres_a PRIVATE cxx_std_17)
target_compile_definitions(provider_postgres_a PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

target_link_libraries (provider_postgres_a
  qgis_core
  ${POSTGRES_LIBRARY}
)

if (WITH_GUI)
  if (BUILD_WITH_QT6)
    QT6_WRAP_UI(PG_UIS_H ${PG_UIS})
  else()
    QT5_WRAP_UI(PG_UIS_H ${PG_UIS})
  endif()

  add_library(provider_postgres_gui_a STATIC ${PG_GUI_SRCS} ${PG_UIS_H})

  target_include_directories(provider_postgres_gui_a SYSTEM PRIVATE
    ${POSTGRES_INCLUDE_DIR}
  )

  target_include_directories(provider_postgres_gui_a PUBLIC
    ${CMAKE_BINARY_DIR}/src/providers/postgres
  )

  # require c++17
  target_compile_features(provider_postgres_gui_a PRIVATE cxx_std_17)
  target_compile_definitions(provider_postgres_gui_a PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

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
  qgspostgresprovidermetadatautils.cpp
)

# static library
add_library (provider_postgresraster_a STATIC ${PGRASTER_SRCS} ${PG_HDRS})

target_include_directories(provider_postgresraster_a SYSTEM PRIVATE
  ${POSTGRES_INCLUDE_DIR}
)
target_include_directories(provider_postgresraster_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/postgres/raster
)

# require c++17
target_compile_features(provider_postgresraster_a PRIVATE cxx_std_17)
target_compile_definitions(provider_postgresraster_a PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

target_link_libraries (provider_postgresraster_a
    qgis_core
    provider_postgres_a
    ${POSTGRES_LIBRARY}
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

  target_include_directories(provider_postgres SYSTEM PRIVATE
    ${POSTGRES_INCLUDE_DIR}
  )
  target_include_directories(provider_postgresraster SYSTEM PRIVATE
    ${POSTGRES_INCLUDE_DIR}
  )
  target_include_directories(provider_postgresraster PRIVATE
    ${CMAKE_SOURCE_DIR}/src/providers/postgres
  )

  # require c++17
  target_compile_features(provider_postgres PRIVATE cxx_std_17)
  target_compile_features(provider_postgresraster PRIVATE cxx_std_17)
  target_compile_definitions(provider_postgres PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
  target_compile_definitions(provider_postgresraster PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

  target_link_libraries(provider_postgres
    qgis_core
    ${POSTGRES_LIBRARY}
  )

  target_link_libraries(provider_postgresraster
    qgis_core
    ${POSTGRES_LIBRARY}
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
