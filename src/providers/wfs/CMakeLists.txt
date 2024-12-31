
########################################################
# Files

set(WFS_SRCS
  qgswfsprovider.cpp
  qgswfsprovidermetadata.cpp
  qgswfscapabilities.cpp
  qgswfsdataitems.cpp
  qgswfsfeatureiterator.cpp
  qgswfsgetfeature.cpp
  qgswfsrequest.cpp
  qgswfsconnection.cpp
  qgswfsdatasourceuri.cpp
  qgswfsconstants.cpp
  qgswfsdescribefeaturetype.cpp
  qgswfsshareddata.cpp
  qgswfstransactionrequest.cpp
  qgswfsutils.cpp
  qgsbackgroundcachedfeatureiterator.cpp
  qgsbackgroundcachedshareddata.cpp
  qgscachedirectorymanager.cpp
  qgsbasenetworkrequest.cpp

  oapif/qgsoapiflandingpagerequest.cpp
  oapif/qgsoapifapirequest.cpp
  oapif/qgsoapifcollection.cpp
  oapif/qgsoapifconformancerequest.cpp
  oapif/qgsoapifcreatefeaturerequest.cpp
  oapif/qgsoapifcql2textexpressioncompiler.cpp
  oapif/qgsoapifdeletefeaturerequest.cpp
  oapif/qgsoapifpatchfeaturerequest.cpp
  oapif/qgsoapifputfeaturerequest.cpp
  oapif/qgsoapifitemsrequest.cpp
  oapif/qgsoapifoptionsrequest.cpp
  oapif/qgsoapifprovider.cpp
  oapif/qgsoapifqueryablesrequest.cpp
  oapif/qgsoapifsingleitemrequest.cpp
  oapif/qgsoapifutils.cpp
)

if (WITH_GUI)
  set(WFS_GUI_SRCS
    qgswfsprovidergui.cpp
    qgswfsdataitemguiprovider.cpp
    qgswfssourceselect.cpp
    qgswfsnewconnection.cpp
    qgswfsguiutils.cpp
    qgswfssubsetstringeditor.cpp
  )

  set(WFS_UIS ${CMAKE_SOURCE_DIR}/src/ui/qgswfssourceselectbase.ui)
endif()

########################################################
# Static
add_library(provider_wfs_a STATIC ${WFS_SRCS})

set_target_properties(provider_wfs_a PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

target_include_directories(provider_wfs_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/wfs
  ${CMAKE_SOURCE_DIR}/src/providers/wfs/oapif
)

target_link_libraries(provider_wfs_a
  qgis_core
  GDAL::GDAL
)

# require c++17
target_compile_features(provider_wfs_a PRIVATE cxx_std_17)


# We use private headers from core that need this
target_compile_definitions(provider_wfs_a PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
target_compile_definitions(provider_wfs_a PRIVATE "-DQT_NO_FOREACH")

if (WITH_GUI)
  qt_wrap_ui(WFS_UIS_H ${WFS_UIS})

  add_library(provider_wfs_gui_a STATIC ${WFS_GUI_SRCS} ${WFS_UIS_H})

  set_target_properties(provider_wfs_gui_a PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

  target_include_directories(provider_wfs_gui_a PUBLIC
    ${CMAKE_SOURCE_DIR}/src/providers/wfs
    ${CMAKE_SOURCE_DIR}/src/providers/wfs/oapif
  )

  target_link_libraries(provider_wfs_a
    qgis_gui
  )
  target_link_libraries(provider_wfs_gui_a
    qgis_gui
  )

  # require c++17
  target_compile_features(provider_wfs_gui_a PRIVATE cxx_std_17)

  target_compile_definitions(provider_wfs_gui_a PRIVATE "-DQT_NO_FOREACH")

  add_dependencies(provider_wfs_gui_a ui)

  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/src/ui
  )
endif()

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_wfs_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS provider_wfs_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  add_library(provider_wfs MODULE ${WFS_SRCS} ${WFS_GUI_SRCS})

  set_target_properties(provider_wfs PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

  # require c++17
  target_compile_features(provider_wfs PRIVATE cxx_std_17)

  # We use private headers from core that need this
  target_compile_definitions(provider_wfs PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
  target_compile_definitions(provider_wfs PRIVATE "-DQT_NO_FOREACH")

  target_link_libraries (provider_wfs
    qgis_core
    GDAL::GDAL
  )

  if (WITH_GUI)
    target_link_libraries (provider_wfs
      qgis_gui
    )
    add_dependencies(provider_wfs ui)
  endif()

  target_include_directories(provider_wfs PUBLIC
    ${CMAKE_SOURCE_DIR}/src/providers/wfs
    ${CMAKE_SOURCE_DIR}/src/providers/wfs/oapif
  )

  install(TARGETS provider_wfs
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
endif()
