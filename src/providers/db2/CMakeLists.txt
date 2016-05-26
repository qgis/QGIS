
########################################################
# Files

SET(DB2_SRCS
  qgsdb2geometrycolumns.cpp
  qgsdb2expressioncompiler.cpp
  qgsdb2provider.cpp
  qgsdb2dataitems.cpp
  qgsdb2newconnection.cpp
  qgsdb2tablemodel.cpp
  qgsdb2featureiterator.cpp
  qgsdb2sourceselect.cpp
)
SET(DB2_MOC_HDRS
  qgsdb2provider.h
  qgsdb2dataitems.h
  qgsdb2newconnection.h
  qgsdb2tablemodel.h
  qgsdb2sourceselect.h
)

########################################################
# Build

QT4_WRAP_CPP(DB2_MOC_SRCS ${DB2_MOC_HDRS})

INCLUDE_DIRECTORIES(
  .
  ../../core
  ../../core/auth  
  ../../core/geometry
  ../../gui
  ../../gui/auth
  ../../ui  
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
)

INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
  ${QT_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
)

ADD_LIBRARY (db2provider MODULE ${DB2_SRCS} ${DB2_HDRS} ${DB2_MOC_SRCS})

TARGET_LINK_LIBRARIES (db2provider
  qgis_core
  qgis_gui
  ${QT_QTSQL_LIBRARY}
)


########################################################
# Install

INSTALL(TARGETS db2provider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
