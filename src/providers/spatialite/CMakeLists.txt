########################################################
# Files

if(WITH_QSPATIALITE)
  SUBDIRS(${CMAKE_SOURCE_DIR}/external/qspatialite)
endif()

set(SPATIALITE_SRCS
  qgsspatialiteprovider.cpp
  qgsspatialitedataitems.cpp
  qgsspatialiteconnection.cpp
  qgsspatialiteconnpool.cpp
  qgsspatialitefeatureiterator.cpp
  qgsspatialitetablemodel.cpp
  qgsspatialiteproviderconnection.cpp
  qgsspatialitetransaction.cpp
  qgsspatialiteexpressioncompiler.cpp
)

if (WITH_GUI)
  set(SPATIALITE_GUI_SRCS
    qgsspatialiteprovidergui.cpp
    qgsspatialitedataitemguiprovider.cpp
    qgsspatialitesourceselect.cpp
  )

  set(SPATIALITE_UIS ${CMAKE_SOURCE_DIR}/src/ui/qgsdbsourceselectbase.ui)
endif()


########################################################
# Static
add_library(provider_spatialite_a STATIC ${SPATIALITE_SRCS})

target_include_directories(provider_spatialite_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/spatialite
)

target_link_libraries(provider_spatialite_a
  qgis_core
  ${SPATIALITE_LIBRARY}
)

# require c++17
target_compile_features(provider_spatialite_a PRIVATE cxx_std_17)

target_compile_definitions(provider_spatialite_a PRIVATE "-DQT_NO_FOREACH")

if (WITH_GUI)
  if (WITH_QT6)
    QT6_WRAP_UI(SPATIALITE_UIS_H ${SPATIALITE_UIS})
  else()
    QT5_WRAP_UI(SPATIALITE_UIS_H ${SPATIALITE_UIS})
  endif()

  add_library(provider_spatialite_gui_a STATIC ${SPATIALITE_GUI_SRCS} ${SPATIALITE_UIS_H})

  target_include_directories(provider_spatialite_gui_a PUBLIC
    ${CMAKE_BINARY_DIR}/src/providers/spatialite
  )

  target_link_libraries(provider_spatialite_a
    qgis_gui
  )
  target_link_libraries(provider_spatialite_gui_a
    qgis_gui
  )

  # require c++17
  target_compile_features(provider_spatialite_gui_a PRIVATE cxx_std_17)

  target_compile_definitions(provider_spatialite_gui_a PRIVATE "-DQT_NO_FOREACH")

  add_dependencies(provider_spatialite_gui_a ui)

  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/src/ui
  )
endif()

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_spatialite_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS provider_spatialite_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  add_library(provider_spatialite MODULE ${SPATIALITE_SRCS} ${SPATIALITE_GUI_SRCS} ${SPATIALITE_UIS_H})

  # require c++17
  target_compile_features(provider_spatialite PRIVATE cxx_std_17)

  target_compile_definitions(provider_spatialite PRIVATE "-DQT_NO_FOREACH")
  
  target_link_libraries(provider_spatialite
    qgis_core
    ${SPATIALITE_LIBRARY}
  )

  if (WITH_GUI)
    target_link_libraries(provider_spatialite
      qgis_gui
    )
    add_dependencies(provider_spatialite ui)
  endif()

  # clang-tidy
  if(CLANG_TIDY_EXE)
    set_target_properties(
      provider_spatialite PROPERTIES
      CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
  endif()

  install(TARGETS provider_spatialite
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
endif()
