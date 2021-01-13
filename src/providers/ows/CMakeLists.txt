set(OWS_SRCS
  qgsowsprovider.cpp
  qgsowsdataitems.cpp
)

include_directories (

  ${CMAKE_BINARY_DIR}/src/ui
)
include_directories(SYSTEM
)

add_library (owsprovider MODULE ${OWS_SRCS})

target_link_libraries (owsprovider
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (owsprovider
    qgis_gui
  )
endif()

install(TARGETS owsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
