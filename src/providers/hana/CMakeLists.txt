FIND_PACKAGE(ODBCCPP REQUIRED)

########################################################
# Files

SET (HANA_SRCS
  qgshanacolumntypethread.cpp
  qgshanaconnectionstringbuilder.cpp
  qgshanaconnection.cpp
  qgshanaconnectionpool.cpp
  qgshanadriver.cpp
  qgshanaexpressioncompiler.cpp
  qgshanafeatureiterator.cpp
  qgshanaprovider.cpp
  qgshanasettings.cpp
  qgshanatablemodel.cpp
  qgshanautils.cpp
)

SET (HANA_MOC_HDRS
  qgshanacolumntypethread.h
  qgshanaconnection.h
  qgshanaconnectionpool.h
  qgshanadataitems.h
  qgshanaprovider.h
  qgshanatablemodel.h
)

IF (WITH_GUI)
  SET(HANA_SRCS ${HANA_SRCS}
    qgshanadataitems.cpp
    qgshananewconnection.cpp
    qgshanasourceselect.cpp
  )
  SET(HANA_MOC_HDRS ${HANA_MOC_HDRS}
    qgshananewconnection.h
    qgshanasourceselect.h
  )
ENDIF ()

SET(HANA_HDRS
  qgshanaexception.h
  qgshanaexpressioncompiler.h
  qgshanadriver.h
)

########################################################
# Build

QT5_WRAP_CPP(HANA_MOC_SRCS ${HANA_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/external

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)

INCLUDE_DIRECTORIES (SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${ODBC_INCLUDE_DIRS}
  ${ODBCCPP_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

ADD_LIBRARY(hanaprovider MODULE ${HANA_SRCS} ${HANA_HDRS} ${HANA_MOC_SRCS})
ADD_LIBRARY(hanaprovider_a STATIC ${HANA_SRCS} ${HANA_HDRS} ${HANA_MOC_SRCS})

SET_TARGET_PROPERTIES(hanaprovider PROPERTIES COMPILE_DEFINITIONS ODBC_EXPORTS)
SET_TARGET_PROPERTIES(hanaprovider_a PROPERTIES COMPILE_DEFINITIONS ODBC_EXPORTS)

TARGET_LINK_LIBRARIES(hanaprovider
  qgis_core
  ${ODBC_LIBRARIES}
  ${ODBCCPP_LIBRARY}
)

TARGET_LINK_LIBRARIES(hanaprovider_a
  qgis_core
  ${ODBC_LIBRARIES}
  ${ODBCCPP_LIBRARY}
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (hanaprovider_a
    qgis_gui
  )

  TARGET_LINK_LIBRARIES (hanaprovider
    qgis_gui
  )
ENDIF ()

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    hanaprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

########################################################
# Install

INSTALL (TARGETS hanaprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
