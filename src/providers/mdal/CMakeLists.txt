SET(MDAL_SRCS
  qgsmdalprovider.cpp
  qgsmdaldataitems.cpp
)
SET(MDAL_MOC_HDRS
  qgsmdalprovider.h
  qgsmdaldataitems.h
)
SET(MDAL_HDRS
)


########################################################
# Compile internal MDAL
IF (WITH_INTERNAL_MDAL)
  ADD_DEFINITIONS(-DMDAL_STATIC)

  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/external/mdal
    ${CMAKE_SOURCE_DIR}/external/mdal/api
  )

  SET(MDAL_LIB_SRCS
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_utils.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_loader.cpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_2dm.cpp
  )

  SET(MDAL_LIB_HDRS
    ${CMAKE_SOURCE_DIR}/external/mdal/api/mdal.h
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_utils.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_loader.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/mdal_defines.hpp
    ${CMAKE_SOURCE_DIR}/external/mdal/frmts/mdal_2dm.hpp
  )

  UNSET(MDAL_LIBRARY)
  UNSET(MDAL_INCLUDE_DIR)

ELSE (WITH_INTERNAL_MDAL)

  INCLUDE_DIRECTORIES (SYSTEM
    ${MDAL_INCLUDE_DIR}
  )

ENDIF (WITH_INTERNAL_MDAL)

########################################################

INCLUDE_DIRECTORIES (
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/mesh
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata

  ${CMAKE_BINARY_DIR}/src/core
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
)

QT5_WRAP_CPP(MDAL_MOC_SRCS ${MDAL_MOC_HDRS})
ADD_LIBRARY (mdalprovider MODULE ${MDAL_SRCS} ${MDAL_MOC_HDRS} ${MDAL_MOC_SRCS} ${MDAL_LIB_SRCS} ${MDAL_LIB_HDRS})

TARGET_LINK_LIBRARIES (mdalprovider
  qgis_core
  ${MDAL_LIBRARY}
)

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    mdalprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

INSTALL(TARGETS mdalprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

