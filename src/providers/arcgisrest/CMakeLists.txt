###############################################################################

set (AFS_SRCS
  qgsafsfeatureiterator.cpp
  qgsafsprovider.cpp
  qgsafsshareddata.cpp
  qgsarcgisrestdataitems.cpp
)
if (WITH_GUI)
  set(AFS_GUI_SRCS
    qgsarcgisrestdataitemguiprovider.cpp
    qgsarcgisrestprovidergui.cpp
    qgsarcgisrestsourceselect.cpp
    qgsarcgisrestsourcewidget.cpp
    qgsnewarcgisrestconnection.cpp
  )
  
  set(AFS_UIS
    ${CMAKE_SOURCE_DIR}/src/ui/qgsarcgisrestsourcewidgetbase.ui
    ${CMAKE_SOURCE_DIR}/src/ui/qgsarcgisservicesourceselectbase.ui
    ${CMAKE_SOURCE_DIR}/src/ui/qgsnewarcgisrestconnectionbase.ui
  )
endif()

add_library (provider_arcgisfeatureserver_a STATIC ${AFS_SRCS})

target_include_directories(provider_arcgisfeatureserver_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/arcgisrest
)

# require c++17
target_compile_features(provider_arcgisfeatureserver_a PRIVATE cxx_std_17)

target_link_libraries (provider_arcgisfeatureserver_a
  qgis_core
  ${QCA_LIBRARY}
)

if (WITH_GUI)
  if (BUILD_WITH_QT6)
    QT6_WRAP_UI(AFS_UIS_H ${AFS_UIS})
  else()
    QT5_WRAP_UI(AFS_UIS_H ${AFS_UIS})
  endif()
  
  add_library(provider_arcgisfeatureserver_gui_a STATIC ${AFS_GUI_SRCS} ${AFS_UIS_H})

  # require c++17
  target_compile_features(provider_arcgisfeatureserver_gui_a PRIVATE cxx_std_17)
  
  target_link_libraries(provider_arcgisfeatureserver_a
    qgis_gui
  )
  target_link_libraries(provider_arcgisfeatureserver_gui_a
    qgis_gui
  )

  include_directories(SYSTEM
    ${CMAKE_BINARY_DIR}/src/ui
    ${QSCINTILLA_INCLUDE_DIR}
  )

  add_dependencies(provider_arcgisfeatureserver_gui_a ui)
endif()

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_arcgisfeatureserver_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS provider_arcgisfeatureserver_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  add_library(provider_arcgisfeatureserver MODULE ${AFS_SRCS} ${AFS_GUI_SRCS} ${AFS_UIS_H})
  
  target_link_libraries(provider_arcgisfeatureserver
    qgis_core
    ${QCA_LIBRARY}
  )
  
  # require c++17
  target_compile_features(provider_arcgisfeatureserver PRIVATE cxx_std_17)
  
  if (WITH_GUI)
    target_link_libraries(provider_arcgisfeatureserver
      qgis_gui
    )
    add_dependencies(provider_arcgisfeatureserver ui)
  endif()
  
  install (TARGETS provider_arcgisfeatureserver
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()

###############################################################################

set (AMS_SRCS
  qgsamsprovider.cpp
)

add_library (provider_arcgismapserver_a STATIC ${AMS_SRCS})

target_include_directories(provider_arcgismapserver_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/arcgisrest
)

# require c++17
target_compile_features(provider_arcgismapserver_a PRIVATE cxx_std_17)

target_link_libraries (provider_arcgismapserver_a
  qgis_core
  ${QCA_LIBRARY}
)

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_arcgismapserver_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
else()
  add_library(provider_arcgismapserver MODULE ${AMS_SRCS})
  
  # require c++17
  target_compile_features(provider_arcgismapserver PRIVATE cxx_std_17)
  
  target_link_libraries(provider_arcgismapserver
    qgis_core
    ${QCA_LIBRARY}
  )
  
  install (TARGETS provider_arcgismapserver
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()
