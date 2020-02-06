ADD_EXECUTABLE(crssync main.cpp)

INCLUDE_DIRECTORIES(
  ../core
  ../core/geometry
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_BINARY_DIR}/src/core
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(crssync
  qgis_core
  ${PROJ_LIBRARY}
  ${GDAL_LIBRARY}
)

IF(MSVC AND NOT USING_NMAKE)
  ADD_CUSTOM_TARGET(synccrsdb
    COMMENT "Running crssync"
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/crssync.exe
    DEPENDS crssync
  )
ELSEIF(CMAKE_CROSSCOMPILING AND NOT MXE)
  SET(NATIVE_CRSSYNC_BIN CACHE PATH "Path to a natively compiled synccrsdb binary")
  IF(NOT NATIVE_CRSSYNC_BIN)
    MESSAGE(FATAL_ERROR "NATIVE_CRSSYNC_BIN needs to be defined when cross-compiling")
  ENDIF()
  ADD_CUSTOM_TARGET(synccrsdb
    COMMENT "Running native crssync"
    COMMAND ${NATIVE_CRSSYNC_BIN}
    DEPENDS crssync
  )
ELSE()
  ADD_CUSTOM_TARGET(synccrsdb
    COMMENT "Running crssync"
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/crssync
    DEPENDS crssync
  )
ENDIF()

INSTALL(CODE "MESSAGE(\"Installing crssync ...\")")
INSTALL(TARGETS crssync RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR})
