########################################################
# Files

SET (SACONN_SRCS 
    sqlanystatement.cpp
    sqlanyconnection.cpp
    sacapi/sacapidll.c
)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DSACONN_EXPORT=__declspec(dllexport)")
    SET_SOURCE_FILES_PROPERTIES(sacapi/sacapidll.c PROPERTIES COMPILE_FLAGS "-wd4152 -wd4055")
  ELSE (MSVC)
    ADD_DEFINITIONS("-USACONN_EXPORT \"-DSACONN_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ELSE (WIN32)
  ADD_DEFINITIONS(-DSACONN_EXPORT=)
ENDIF (WIN32)

########################################################
# Build

INCLUDE_DIRECTORIES(
     ../../../core 
)

ADD_LIBRARY (qgissqlanyconnection SHARED ${SACONN_SRCS})

SET_TARGET_PROPERTIES(qgissqlanyconnection PROPERTIES VERSION ${COMPLETE_VERSION} SOVERSION ${COMPLETE_VERSION})

TARGET_LINK_LIBRARIES(qgissqlanyconnection
  qgis_core
)

IF (APPLE)
  SET_TARGET_PROPERTIES(qgissqlanyconnection PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE )
ENDIF (APPLE)

########################################################
# Install

IF (WIN32)
    INSTALL(TARGETS qgissqlanyconnection
      RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
      LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
      )
ELSE (WIN32)
    INSTALL(TARGETS qgissqlanyconnection
      RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
      LIBRARY DESTINATION ${QGIS_LIB_DIR}
      )
ENDIF (WIN32)

