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

  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/qgis_win32.obj
                     COMMAND ${WINDRES}  -I${CMAKE_CURRENT_SOURCE_DIR}/..
                                         -i${CMAKE_CURRENT_SOURCE_DIR}/../qgis_win32.rc
                                         -o ${CMAKE_CURRENT_BINARY_DIR}/qgis_win32.obj)
  SET(QGIS_APP_SRCS ${QGIS_APP_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_win32.obj)

ENDIF (WIN32)


ADD_EXECUTABLE(qgis ${QGIS_APP_SRCS})

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
