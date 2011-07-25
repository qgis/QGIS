# override default path where built files are put to allow running qgis without installing
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR})

ADD_EXECUTABLE(crssync main.cpp)
INCLUDE_DIRECTORIES(
  ../core
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(crssync
  qgis_core
  ${PROJ_LIBRARY}
  ${GDAL_LIBRARY}
)

INSTALL(CODE "MESSAGE(\"Installing crssync ...\")")
INSTALL(TARGETS crssync RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR})
