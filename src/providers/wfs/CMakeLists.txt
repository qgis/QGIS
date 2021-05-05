
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
  set(WFS_SRCS ${WFS_SRCS}
    qgswfsprovidergui.cpp
    qgswfsdataitemguiprovider.cpp
    qgswfssourceselect.cpp
    qgswfsnewconnection.cpp
    qgswfsguiutils.cpp
    qgswfssubsetstringeditor.cpp
  )
endif()

########################################################
# Build

include_directories (

  ${CMAKE_BINARY_DIR}/src/ui
)

if (WITH_GUI)
  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
endif()

add_library (wfsprovider MODULE ${WFS_SRCS})

# require c++17
target_compile_features(wfsprovider PRIVATE cxx_std_17)

target_link_libraries (wfsprovider
  ${EXPAT_LIBRARY}
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (wfsprovider
    qgis_gui
  )
  add_dependencies(wfsprovider ui)
endif()

########################################################
# Install

install(TARGETS wfsprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
