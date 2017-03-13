
SET (MEMORY_SRCS qgsmemoryprovider.cpp qgsmemoryfeatureiterator.cpp)

INCLUDE_DIRECTORIES(
  .
  ../../core
  ../../core/geometry
  ${CMAKE_BINARY_DIR}/src/core
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
)

QT5_WRAP_CPP(MEMORY_MOC_SRCS qgsmemoryprovider.h)
ADD_LIBRARY(memoryprovider MODULE ${MEMORY_SRCS} ${MEMORY_MOC_SRCS})

TARGET_LINK_LIBRARIES(memoryprovider
  qgis_core
)

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    memoryprovider PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)


INSTALL (TARGETS memoryprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
