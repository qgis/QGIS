########################################################
# Files
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(RS_SRCS
  qgsredshiftprovider.cpp
  qgsredshiftconn.cpp
  qgsredshiftconnpool.cpp
  qgsredshiftdataitems.cpp
  qgsredshiftfeatureiterator.cpp
  qgsredshiftprojectstorage.cpp
  qgsredshifttablemodel.cpp
  qgscolumntypethread.cpp
  qgsredshiftexpressioncompiler.cpp
  qgsredshiftproviderconnection.cpp
)

if (WITH_GUI)
  set(RS_GUI_SRCS
    qgsredshiftprovidergui.cpp
    qgsredshiftdataitemguiprovider.cpp
    qgsredshiftsourceselect.cpp
    qgsredshiftnewconnection.cpp
    qgsredshiftprojectstoragedialog.cpp
  )
endif ()

set(RS_HDRS
  qgsredshiftexpressioncompiler.h
  qgsredshiftproviderconnection.h
)

########################################################
# Build

include_directories(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)

include_directories(SYSTEM
  ${POSTGRES_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${SPATIALINDEX_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WITH_GUI)
  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
endif ()


add_library(redshiftprovider MODULE ${RS_SRCS} ${RS_GUI_SRCS} ${RS_HDRS})

target_compile_features(redshiftprovider PRIVATE cxx_std_17)

target_link_libraries(redshiftprovider
  ${POSTGRES_LIBRARY}
  qgis_core
)

if (WITH_GUI)
  target_link_libraries (redshiftprovider
    qgis_gui
  )
  add_dependencies(redshiftprovider ui)
endif (WITH_GUI)


# clang-tidy
if (CLANG_TIDY_EXE)
  set_target_properties(
    redshiftprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )

endif(CLANG_TIDY_EXE)

install(TARGETS redshiftprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
