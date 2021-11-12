
########################################################
# Files

set(WFS_SRCS
  ${CMAKE_SOURCE_DIR}/external/nlohmann/json.hpp
  qgswfsprovider.cpp
  qgswfscapabilities.cpp
  qgswfsdataitems.cpp
  qgswfsfeatureiterator.cpp
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
  qgsoapiflandingpagerequest.cpp
  qgsoapifapirequest.cpp
  qgsoapifcollection.cpp
  qgsoapifitemsrequest.cpp
  qgsoapifprovider.cpp
  qgsoapifutils.cpp
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

target_include_directories(provider_wfs_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/wfs
)

target_link_libraries(provider_wfs_a
  ${EXPAT_LIBRARY}
  qgis_core
)

# require c++17
target_compile_features(provider_wfs_a PRIVATE cxx_std_17)

target_compile_definitions(provider_wfs_a PRIVATE "-DQT_NO_FOREACH")

if (WITH_GUI)
  if (WITH_QT6)
    QT6_WRAP_UI(WFS_UIS_H ${WFS_UIS})
  else()
    QT5_WRAP_UI(WFS_UIS_H ${WFS_UIS})
  endif()

  add_library(provider_wfs_gui_a STATIC ${WFS_GUI_SRCS} ${WFS_UIS_H})

  target_include_directories(provider_wfs_gui_a PUBLIC
    ${CMAKE_BINARY_DIR}/src/providers/wfs
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

  # require c++17
  target_compile_features(provider_wfs PRIVATE cxx_std_17)

  target_compile_definitions(provider_wfs PRIVATE "-DQT_NO_FOREACH")

  target_link_libraries (provider_wfs
    ${EXPAT_LIBRARY}
    qgis_core
  )

  if (WITH_GUI)
    target_link_libraries (provider_wfs
      qgis_gui
    )
    add_dependencies(provider_wfs ui)
  endif()
  
  install(TARGETS provider_wfs
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
endif()
