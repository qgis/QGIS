include_directories(

  ${CMAKE_BINARY_DIR}/src/ui
)

if (WITH_GUI)
  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
endif()

###############################################################################

set (AFS_SRCS
  qgsafsfeatureiterator.cpp
  qgsafsprovider.cpp
  qgsafsshareddata.cpp
  qgsarcgisrestdataitems.cpp
)
if (WITH_GUI)
  set(AFS_SRCS ${AFS_SRCS}
    qgsarcgisrestdataitemguiprovider.cpp
    qgsarcgisrestprovidergui.cpp
    qgsarcgisrestsourceselect.cpp
    qgsarcgisrestsourcewidget.cpp
    qgsnewarcgisrestconnection.cpp
  )
endif()

add_library (provider_arcgisfeatureserver_a STATIC ${AFS_SRCS})
add_library(provider_arcgisfeatureserver MODULE ${AFS_SRCS})

# require c++17
target_compile_features(provider_arcgisfeatureserver_a PRIVATE cxx_std_17)
target_compile_features(provider_arcgisfeatureserver PRIVATE cxx_std_17)

target_link_libraries(provider_arcgisfeatureserver
  qgis_core
  ${QCA_LIBRARY}
)

target_link_libraries (provider_arcgisfeatureserver_a
  qgis_core
  ${QCA_LIBRARY}
)

if (WITH_GUI)
  target_link_libraries(provider_arcgisfeatureserver
    qgis_gui
  )
  target_link_libraries(provider_arcgisfeatureserver_a
    qgis_gui
  )
  add_dependencies(provider_arcgisfeatureserver ui)
  add_dependencies(provider_arcgisfeatureserver_a ui)
endif()

install (TARGETS provider_arcgisfeatureserver
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

###############################################################################

set (AMS_SRCS
  qgsamsprovider.cpp
)

add_library(provider_arcgismapserver MODULE ${AMS_SRCS})

# require c++17
target_compile_features(provider_arcgismapserver PRIVATE cxx_std_17)

target_link_libraries(provider_arcgismapserver
  qgis_core
  ${QCA_LIBRARY}
)

install (TARGETS provider_arcgismapserver
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
