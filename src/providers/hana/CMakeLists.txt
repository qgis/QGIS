########################################################
# Files

SET (HANA_SRCS
  qgshanacolumntypethread.cpp
  qgshanaconnection.cpp
  qgshanaconnectionpool.cpp
  qgshanaconnectionstringbuilder.cpp
  qgshanadriver.cpp
  qgshanaexpressioncompiler.cpp
  qgshanafeatureiterator.cpp
  qgshanaprimarykeys.cpp
  qgshanaprovider.cpp
  qgshanaproviderconnection.cpp
  qgshanaresultset.cpp
  qgshanasettings.cpp
  qgshanatablemodel.cpp
  qgshanacrsutils.cpp
  qgshanautils.cpp
)

IF (WITH_GUI)
  SET(HANA_SRCS ${HANA_SRCS}
    qgshanadataitems.cpp
    qgshanadataitemguiprovider.cpp
    qgshananewconnection.cpp
    qgshanaprovidergui.cpp
    qgshanasourceselect.cpp
  )
ENDIF ()

SET(HANA_HDRS
  qgshanacolumntypethread.h
  qgshanaconnection.h
  qgshanaconnectionpool.h
  qgshanaconnectionstringbuilder.h
  qgshanadriver.h
  qgshanaexception.h
  qgshanaexpressioncompiler.h
  qgshanafeatureiterator.h
  qgshanaprimarykeys.h
  qgshanaprovider.h
  qgshanaproviderconnection.h
  qgshanaresultset.h
  qgshanasettings.h
  qgshanatablemodel.h
  qgshanacrsutils.h
  qgshanautils.h
)

IF (WITH_GUI)
  SET(HANA_HDRS ${HANA_HDRS}
    qgshanadataitems.h
    qgshanadataitemguiprovider.h
    qgshananewconnection.h
    qgshanasourceselect.h
  )
ENDIF ()

########################################################
# Build

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/external/odbccpp/src

  ${CMAKE_BINARY_DIR}/src/ui
)

INCLUDE_DIRECTORIES (SYSTEM
  ${PROJ_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${ODBC_INCLUDE_DIRS}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

ADD_LIBRARY(hanaprovider MODULE ${HANA_SRCS} ${HANA_HDRS})
ADD_LIBRARY(hanaprovider_a STATIC ${HANA_SRCS} ${HANA_HDRS})

TARGET_COMPILE_DEFINITIONS(hanaprovider PUBLIC ODBC_STATIC)
TARGET_COMPILE_DEFINITIONS(hanaprovider_a PUBLIC ODBC_STATIC)

TARGET_LINK_LIBRARIES(hanaprovider
  qgis_core
  odbccpp_static
  ${ODBC_LIBRARIES}
)

TARGET_LINK_LIBRARIES(hanaprovider_a
  qgis_core
  odbccpp_static
  ${ODBC_LIBRARIES}
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
