########################################################
# Files

SET(PG_SRCS
  qgspostgresprovider.cpp
  qgspostgresconn.cpp
  qgspostgresconnpool.cpp
  qgspostgresdataitems.cpp
  qgspostgresfeatureiterator.cpp
  qgspostgrestransaction.cpp
  qgspgtablemodel.cpp
  qgscolumntypethread.cpp
  qgspostgresexpressioncompiler.cpp
  qgspostgreslistener.cpp
)

SET(PG_MOC_HDRS
  qgscolumntypethread.h
  qgspgtablemodel.h
  qgspostgresconn.h
  qgspostgresconnpool.h
  qgspostgresdataitems.h
  qgspostgresprovider.h
  qgspostgrestransaction.h
  qgspostgreslistener.h
)

IF (WITH_GUI)
  SET(PG_SRCS ${PG_SRCS}
    qgspgsourceselect.cpp
    qgspgnewconnection.cpp
  )
  SET(PG_MOC_HDRS ${PG_MOC_HDRS}
    qgspgnewconnection.h
    qgspgsourceselect.h
  )
ENDIF ()

SET(PG_HDRS
  qgspostgresexpressioncompiler.h
)

########################################################
# Build

QT5_WRAP_CPP(PG_MOC_SRCS ${PG_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)

INCLUDE_DIRECTORIES(SYSTEM
  ${POSTGRES_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

IF (WITH_GUI)
  INCLUDE_DIRECTORIES(SYSTEM
    ${QSCINTILLA_INCLUDE_DIR}
  )
ENDIF ()

ADD_LIBRARY (postgresprovider_a STATIC ${PG_SRCS} ${PG_HDRS} ${PG_MOC_SRCS})
ADD_LIBRARY (postgresprovider MODULE ${PG_SRCS} ${PG_HDRS} ${PG_MOC_SRCS})

TARGET_LINK_LIBRARIES (postgresprovider_a
  ${POSTGRES_LIBRARY}
  qgis_core
)
TARGET_LINK_LIBRARIES (postgresprovider
  ${POSTGRES_LIBRARY}
  qgis_core
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (postgresprovider_a
    qgis_gui
  )
  TARGET_LINK_LIBRARIES (postgresprovider
    qgis_gui
  )
ENDIF ()


# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    postgresprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

########################################################
# Install

INSTALL(TARGETS postgresprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
