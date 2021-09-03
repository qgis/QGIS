if (WITH_GUI)
  include_directories(
    ${CMAKE_BINARY_DIR}/src/ui
  )
endif()

########################################################
# Files

set (GPX_SRCS
  qgsgpxprovider.cpp
  qgsgpxfeatureiterator.cpp
  gpsdata.cpp
)
if (WITH_GUI)
  set(GPX_SRCS ${GPX_SRCS}
    qgsgpxprovidergui.cpp
    qgsgpxsourceselect.cpp
  )
endif()

########################################################
# Build

include_directories(

)
include_directories (SYSTEM
)

add_library(provider_gpx MODULE ${GPX_SRCS})

# require c++17
target_compile_features(provider_gpx PRIVATE cxx_std_17)

target_link_libraries(provider_gpx
  ${EXPAT_LIBRARY}
  qgis_core
)

if (WITH_GUI)
  target_link_libraries(provider_gpx
    qgis_gui
  )
  add_dependencies(provider_gpx ui)
endif()

########################################################
# Install

install (TARGETS provider_gpx
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
