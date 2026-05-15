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

set_target_properties(provider_arcgisfeatureserver_a PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

target_include_directories(provider_arcgisfeatureserver_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/arcgisrest
)

# We use private headers from core that need this
target_compile_definitions(provider_arcgisfeatureserver_a PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

target_link_libraries (provider_arcgisfeatureserver_a
  qgis_core
  ${QCA_LIBRARY}
)

if (WITH_GUI)
  qt_wrap_ui(AFS_UIS_H ${AFS_UIS})

  add_library(provider_arcgisfeatureserver_gui_a STATIC ${AFS_GUI_SRCS} ${AFS_UIS_H})

  set_target_properties(provider_arcgisfeatureserver_gui_a PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

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

  set_target_properties(provider_arcgisfeatureserver PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

  target_link_libraries(provider_arcgisfeatureserver
    qgis_core
    ${QCA_LIBRARY}
  )

  # We use private headers from core that need this
  target_compile_definitions(provider_arcgisfeatureserver PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

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

# We use private headers from core that need this
target_compile_definitions(provider_arcgismapserver_a PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

target_link_libraries (provider_arcgismapserver_a
  qgis_core
  ${QCA_LIBRARY}
)

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_arcgismapserver_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
else()
  add_library(provider_arcgismapserver MODULE ${AMS_SRCS})

  # We use private headers from core that need this
  target_compile_definitions(provider_arcgismapserver PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

  target_link_libraries(provider_arcgismapserver
    qgis_core
    ${QCA_LIBRARY}
  )

  install (TARGETS provider_arcgismapserver
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()


###############################################################################

set (IMS_SRCS
  qgsimageserverprovider.cpp
)
if (WITH_GUI)
  set(IMS_GUI_SRCS
    qgsimageserverprovidergui.cpp
    qgsarcgisimageserversourcewidget.cpp
  )

  set(IMS_UIS
    ${CMAKE_SOURCE_DIR}/src/ui/qgsarcgisimageserversourcewidgetbase.ui
  )
endif()

add_library (provider_arcgisimageserver_a STATIC ${IMS_SRCS})

target_include_directories(provider_arcgisimageserver_a PUBLIC
  ${CMAKE_SOURCE_DIR}/src/providers/arcgisrest
)

# We use private headers from core that need this
target_compile_definitions(provider_arcgisimageserver_a PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

target_link_libraries (provider_arcgisimageserver_a
  qgis_core
  ${QCA_LIBRARY}
)

set_target_properties(provider_arcgisimageserver_a PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

if (WITH_GUI)
  qt_wrap_ui(IMS_UIS_H ${IMS_UIS})

  add_library(provider_arcgisimageserver_gui_a STATIC ${IMS_GUI_SRCS} ${IMS_UIS_H})

  set_target_properties(provider_arcgisimageserver_gui_a PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

  target_link_libraries(provider_arcgisimageserver_a
    qgis_gui
  )
  target_link_libraries(provider_arcgisimageserver_gui_a
    qgis_gui
  )

  include_directories(SYSTEM
    ${CMAKE_BINARY_DIR}/src/ui
    ${QSCINTILLA_INCLUDE_DIR}
  )

  add_dependencies(provider_arcgisimageserver_gui_a ui)
endif()

if (FORCE_STATIC_LIBS)
  # for (external) mobile apps to be able to pick up provider for linking
  install (TARGETS provider_arcgisimageserver_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  if (WITH_GUI)
    install (TARGETS provider_arcgisimageserver_gui_a ARCHIVE DESTINATION ${QGIS_PLUGIN_DIR})
  endif()
else()
  add_library(provider_arcgisimageserver MODULE ${IMS_SRCS} ${IMS_GUI_SRCS} ${IMS_UIS_H})

  # We use private headers from core that need this
  target_compile_definitions(provider_arcgisimageserver PRIVATE "CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

  set_target_properties(provider_arcgisimageserver PROPERTIES UNITY_BUILD ${ENABLE_UNITY_BUILDS})

  target_link_libraries(provider_arcgisimageserver
    qgis_core
    ${QCA_LIBRARY}
  )

  if (WITH_GUI)
    target_link_libraries(provider_arcgisimageserver
      qgis_gui
    )
    add_dependencies(provider_arcgisimageserver ui)
  endif()

  install (TARGETS provider_arcgisimageserver
    RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
    LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
  )
endif()