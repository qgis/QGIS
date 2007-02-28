INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
     ../core
     ../raster ../legend ../composer ../plugins
     ../widgets/projectionselector
     ${SQLITE3_INCLUDE_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}
     ${CMAKE_CURRENT_BINARY_DIR}/../ui
     ${CMAKE_CURRENT_BINARY_DIR}/../../tools/mapserver_export
     ${GEOS_INCLUDE_DIR}
)

SET(QGIS_APP_SRCS main.cpp)

IF (WIN32)

  # Icon for windows
  # Note: can't include .rc directly to source files
  # as it's ignored when used MinGW

  IF (NOT WINDRES)
    FIND_PROGRAM(WINDRES windres)
    IF (NOT WINDRES)
      MESSAGE(FATAL_ERROR "windres not found - aborting")
    ENDIF (NOT WINDRES)
  ENDIF (NOT WINDRES)


  #############################################################
  # application icon
  IF( MINGW )
    # resource compilation for MinGW
    ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o
      COMMAND ${WINDRES} -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/qgis_win32.rc 
      -o ${CMAKE_CURRENT_BINARY_DIR}/icon.o )
    SET(QGIS_APP_SRCS ${QGIS_APP_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
  ELSE( MINGW )
    SET(QGIS_APP_SRCS ${QGIS_APP_SRCS} icon.rc)
  ENDIF( MINGW )

ENDIF (WIN32)

# Remove console if on WIN32 and not Debug.
IF (WIN32)
   IF (CMAKE_BUILD_TYPE MATCHES Debug)
      ADD_EXECUTABLE(qgis ${QGIS_APP_SRCS})
   ELSE (CMAKE_BUILD_TYPE MATCHES Debug)
      ADD_EXECUTABLE(qgis WIN32 ${QGIS_APP_SRCS})
   ENDIF (CMAKE_BUILD_TYPE MATCHES Debug)
ELSE (WIN32)
   ADD_EXECUTABLE(qgis ${QGIS_APP_SRCS})
ENDIF (WIN32)

# This sets runpath to the installation directory
SET_TARGET_PROPERTIES(qgis PROPERTIES
  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
  BUILD_WITH_INSTALL_RPATH true
)

# Uncomment this to set runpath to the directory path specified at compile time
#SET_TARGET_PROPERTIES(qgis PROPERTIES
#  INSTALL_RPATH_USE_LINK_PATH true
#)

# Uncomment this to set runpath relative to executable
#SET_TARGET_PROPERTIES(qgis PROPERTIES
#  LINK_FLAGS "-Wl,-rpath,'\$\$ORIGIN/../lib'"
#)

TARGET_LINK_LIBRARIES(qgis
  ${QT_LIBRARIES}
  qgis_core
)

IF (POSTGRES_FOUND)
  TARGET_LINK_LIBRARIES (qgis ${POSTGRES_LIBRARY})
ENDIF (POSTGRES_FOUND)



IF (APPLE)
  # For Mac OS X, the executable must be at the root of the bundle's executable folder
  INSTALL(TARGETS qgis RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
ELSE (APPLE)
  INSTALL(TARGETS qgis RUNTIME DESTINATION ${QGIS_BIN_DIR})
ENDIF (APPLE)
