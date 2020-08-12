
########################################################
# Files

SET (DTEXT_SRCS
  qgsdelimitedtextfeatureiterator.cpp
  qgsdelimitedtextprovider.cpp
  qgsdelimitedtextfile.cpp
)

IF (WITH_GUI)
  SET(DTEXT_SRCS ${DTEXT_SRCS}
    qgsdelimitedtextprovidergui.cpp
    qgsdelimitedtextsourceselect.cpp
  )
ENDIF ()

########################################################
# Build

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
)
INCLUDE_DIRECTORIES (SYSTEM
)

ADD_LIBRARY(delimitedtextprovider MODULE ${DTEXT_SRCS})

TARGET_LINK_LIBRARIES(delimitedtextprovider
  qgis_core
)

IF (WITH_GUI)
  TARGET_LINK_LIBRARIES (delimitedtextprovider
    qgis_gui
  )
  ADD_DEPENDENCIES(delimitedtextprovider ui)
ENDIF ()

TARGET_COMPILE_DEFINITIONS(delimitedtextprovider PRIVATE "-DQT_NO_FOREACH")

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    delimitedtextprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

########################################################
# Install

INSTALL (TARGETS delimitedtextprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
