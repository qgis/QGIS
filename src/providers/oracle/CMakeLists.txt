########################################################
# Files

add_subdirectory(ocispatial)

set(ORACLE_SRCS
  qgsoracleprovider.cpp
  qgsoracleconn.cpp
  qgsoracledataitems.cpp
  qgsoraclenewconnection.cpp
  qgsoracletablecache.cpp
  qgsoracletablemodel.cpp
  qgsoraclecolumntypethread.cpp
  qgsoraclefeatureiterator.cpp
  qgsoracleconnpool.cpp
  qgsoracleexpressioncompiler.cpp
  qgsoracletransaction.cpp
  qgsoracleproviderconnection.cpp
)

if (WITH_GUI)
  set(ORACLE_SRCS ${ORACLE_SRCS}
    qgsoraclesourceselect.cpp
  )
endif()

set(ORACLE_HDRS
  qgsoracleexpressioncompiler.h
)


########################################################
# Build

include_directories(

  ${CMAKE_BINARY_DIR}/src/ui
)
include_directories(SYSTEM
  ${GEOS_INCLUDE_DIR}
  ${QT_QTSQL_INCLUDEDIR}
)

if (WITH_GUI)
  include_directories(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
endif()

add_library (provider_oracle MODULE ${ORACLE_SRCS} ${ORACLE_HDRS})

# require c++17
target_compile_features(provider_oracle PRIVATE cxx_std_17)

target_link_libraries (provider_oracle
  qgis_core
  ${Qt5Sql_LIBRARIES}
)

if (WITH_GUI)
  target_link_libraries (provider_oracle
    qgis_gui
  )
  add_dependencies(provider_oracle ui)
endif()


########################################################
# Install

install(TARGETS provider_oracle
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
