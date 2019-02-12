SET (OGR_SRCS
    qgsogrprovider.cpp
    qgsogrdataitems.cpp
    qgsogrfeatureiterator.cpp
    qgsogrconnpool.cpp
    qgsogrexpressioncompiler.cpp
    qgsgeopackagedataitems.cpp
    qgsgeopackagerasterwriter.cpp
    qgsgeopackagerasterwritertask.cpp
    qgsogrdbconnection.cpp
    qgsogrdbtablemodel.cpp
    qgsogrtransaction.cpp
)

SET(OGR_MOC_HDRS
    qgsogrprovider.h
    qgsogrdataitems.h
    qgsogrconnpool.h
    qgsgeopackagedataitems.h
    qgsgeopackagerasterwritertask.h
    qgsogrdbconnection.h
    qgsogrdbtablemodel.h
    qgsogrtransaction.h
)

IF (WITH_GUI)
  SET(OGR_SRCS ${OGR_SRCS}
    qgsogrdbsourceselect.cpp
    qgsogrsourceselect.cpp
  )
  SET(OGR_MOC_HDRS ${OGR_MOC_HDRS}
    qgsogrsourceselect.h
    qgsogrdbsourceselect.h
  )
ENDIF ()

########################################################
# Build

QT5_WRAP_CPP(OGR_MOC_SRCS ${OGR_MOC_HDRS})

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
)


ADD_LIBRARY(ogrprovider MODULE ${OGR_SRCS} ${OGR_MOC_SRCS})

TARGET_LINK_LIBRARIES(ogrprovider
  qgis_core
)


IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (ogrprovider
    qgis_gui
  )
ENDIF ()

IF (MSVC)
  #needed for linking to gdal which needs odbc
  SET(TARGET_LINK_LIBRARIES ${TARGET_LINK_LIBRARIE} odbc32 odbccp32)
ENDIF (MSVC)

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    ogrprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

INSTALL (TARGETS ogrprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
