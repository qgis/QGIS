########################################################
# Files

SUBDIRS( sqlanyconnection )

SET (SA_SRCS 
    qgssqlanywhereprovider.cpp
)
SET (SA_MOC_HDRS 
    qgssqlanywhereprovider.h
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

QT4_WRAP_CPP (SA_MOC_SRCS  ${SA_MOC_HDRS})

INCLUDE_DIRECTORIES(
     ${GEOS_INCLUDE_DIR}
     ../../core 
     sqlanyconnection
)

ADD_LIBRARY (sqlanywhereprovider MODULE ${SA_SRCS} ${SA_MOC_SRCS})

TARGET_LINK_LIBRARIES(sqlanywhereprovider
  qgis_core
  qgissqlanyconnection
)


########################################################
# Install

INSTALL(TARGETS sqlanywhereprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

