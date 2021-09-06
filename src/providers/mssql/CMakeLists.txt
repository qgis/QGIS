set(MSSQL_SRCS
  qgsmssqlconnection.cpp
  qgsmssqlprovider.cpp
  qgsmssqlproviderconnection.cpp
  qgsmssqlgeometryparser.cpp
  qgsmssqltablemodel.cpp
  qgsmssqldataitems.cpp
  qgsmssqlexpressioncompiler.cpp
  qgsmssqlfeatureiterator.cpp
  qgsmssqlgeomcolumntypethread.cpp
  qgsmssqltransaction.cpp
  qgsmssqldatabase.cpp
)

if (WITH_GUI)
  set(MSSQL_SRCS ${MSSQL_SRCS}
    qgsmssqlprovidergui.cpp
    qgsmssqldataitemguiprovider.cpp
    qgsmssqlsourceselect.cpp
    qgsmssqlnewconnection.cpp
  )
endif()

set(MSSQL_HDRS
  qgsmssqlexpressioncompiler.h
)

########################################################
# Build

include_directories(

  ${CMAKE_BINARY_DIR}/src/ui
)


add_library(provider_mssql MODULE ${MSSQL_SRCS} ${MSSQL_HDRS})

# require c++17
target_compile_features(provider_mssql PRIVATE cxx_std_17)

target_link_libraries(provider_mssql
  qgis_core
  ${QT_VERSION_BASE}::Sql
)

if (WITH_GUI)
  target_link_libraries (provider_mssql
    qgis_gui
  )
  add_dependencies(provider_mssql ui)
endif()

# static library
add_library (provider_mssql_a STATIC ${MSSQL_SRCS} ${MSSQL_HDRS})

# require c++17
target_compile_features(provider_mssql_a PRIVATE cxx_std_17)

target_link_libraries (provider_mssql_a
  qgis_core
  ${POSTGRES_LIBRARY}
  ${Qt5Xml_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Svg_LIBRARIES}
  ${Qt5Network_LIBRARIES}
  ${Qt5Sql_LIBRARIES}
  ${Qt5Concurrent_LIBRARIES}
)

if (WITH_GUI)
  target_link_libraries (provider_mssql_a
    qgis_gui
  )
  add_dependencies(provider_mssql_a ui)
endif()

########################################################
# Install

install (TARGETS provider_mssql
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

