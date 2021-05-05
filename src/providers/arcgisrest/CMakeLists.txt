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

add_library (arcgisfeatureserverprovider_a STATIC ${AFS_SRCS})
add_library(arcgisfeatureserverprovider MODULE ${AFS_SRCS})

# require c++17
target_compile_features(arcgisfeatureserverprovider_a PRIVATE cxx_std_17)
target_compile_features(arcgisfeatureserverprovider PRIVATE cxx_std_17)

target_link_libraries(arcgisfeatureserverprovider
  qgis_core
  ${QCA_LIBRARY}
)

target_link_libraries (arcgisfeatureserverprovider_a
  qgis_core
  ${QCA_LIBRARY}
)

if (WITH_GUI)
  target_link_libraries(arcgisfeatureserverprovider
    qgis_gui
  )
  target_link_libraries(arcgisfeatureserverprovider_a
    qgis_gui
  )
  add_dependencies(arcgisfeatureserverprovider ui)
  add_dependencies(arcgisfeatureserverprovider_a ui)
endif()

install (TARGETS arcgisfeatureserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

###############################################################################

set (AMS_SRCS
  qgsamsprovider.cpp
)

add_library(arcgismapserverprovider MODULE ${AMS_SRCS})

# require c++17
target_compile_features(arcgismapserverprovider PRIVATE cxx_std_17)

target_link_libraries(arcgismapserverprovider
  qgis_core
  ${QCA_LIBRARY}
)

install (TARGETS arcgismapserverprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
