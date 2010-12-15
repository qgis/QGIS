########################################################
# Files

SET (SACONN_SRCS 
    sqlanystatement.cpp
    sqlanyconnection.cpp
    sacapi/sacapidll.c
)
SET (SACONN_MOC_HDRS 
    sqlanystatement.h
    sqlanyconnection.h
    sacapi/sacapidll.h
)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DSACONN_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-USACONN_EXPORT \"-DSACONN_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ELSE (WIN32)
  ADD_DEFINITIONS(-DSACONN_EXPORT=)
ENDIF (WIN32)

########################################################
# Build

QT4_WRAP_CPP (SACONN_MOC_SRCS  ${SACONN_MOC_HDRS})

INCLUDE_DIRECTORIES(
     ../../../core 
)

ADD_LIBRARY (sqlanyconnection SHARED ${SACONN_SRCS} ${SACONN_MOC_SRCS})

SET_TARGET_PROPERTIES(sqlanyconnection PROPERTIES VERSION ${COMPLETE_VERSION} SOVERSION ${COMPLETE_VERSION})

TARGET_LINK_LIBRARIES(sqlanyconnection
  qgis_core
)


########################################################
# Install

IF (WIN32)
    INSTALL(TARGETS sqlanyconnection
      RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
      LIBRARY DESTINATION ${QGIS_PLUGIN_DIR}
      )
ELSE (WIN32)
    INSTALL(TARGETS sqlanyconnection
      RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
      LIBRARY DESTINATION ${QGIS_LIB_DIR}
      )
ENDIF (WIN32)

