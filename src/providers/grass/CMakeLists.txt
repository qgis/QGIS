ADD_DEFINITIONS(-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\")

########################################################
# Build

INCLUDE_DIRECTORIES(
  ../../core
  ../../core/raster
  ${GRASS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
)

#
# GRASS library
#

QT4_WRAP_CPP(GRASS_PROVIDER_MOC_SRCS qgsgrassprovider.h)
ADD_LIBRARY(qgisgrass SHARED qgsgrass.cpp qgsgrassprovider.cpp ${GRASS_PROVIDER_MOC_SRCS})

SET_TARGET_PROPERTIES(qgisgrass PROPERTIES
  CLEAN_DIRECT_OUTPUT 1
  FRAMEWORK 1
  FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
  MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
  MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
  MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgisgrass
  COMPILE_FLAGS "\"-DGRASS_LIB_EXPORT=${DLLEXPORT}\" \"-DGRASS_EXPORT=${DLLIMPORT}\"" )

#generate unversioned libs for android
IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgisgrass PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

TARGET_LINK_LIBRARIES(qgisgrass
  qgis_core
  ${GRASS_LIBRARY_gis}
  ${GRASS_LIBRARY_vect}
  ${GRASS_LIBRARY_dbmibase}
  ${GRASS_LIBRARY_dbmiclient}
  ${GRASS_LIBRARY_gproj}
)

IF (APPLE)
  SET_TARGET_PROPERTIES(qgisgrass PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
ENDIF (APPLE)

#
# GRASS vector provider
#

ADD_LIBRARY(grassprovider MODULE qgsgrassprovidermodule.cpp)
SET_TARGET_PROPERTIES(grassprovider PROPERTIES COMPILE_FLAGS "\"-DGRASS_EXPORT=${DLLEXPORT}\" \"-DGRASS_LIB_EXPORT=${DLLIMPORT}\"" )
TARGET_LINK_LIBRARIES(grassprovider qgisgrass)

#
# grass raster provider
#

QT4_WRAP_CPP(GRASS_RASTERPROVIDER_MOC_SRCS qgsgrassrasterprovider.h)
ADD_LIBRARY(grassrasterprovider MODULE qgsgrassrasterprovider.cpp ${GRASS_RASTERPROVIDER_MOC_SRCS})
SET_TARGET_PROPERTIES(grassrasterprovider PROPERTIES COMPILE_FLAGS "\"-DGRASS_EXPORT=${DLLEXPORT}\" \"-DGRASS_LIB_EXPORT=${DLLIMPORT}\"" )
TARGET_LINK_LIBRARIES(grassrasterprovider qgisgrass qgis_core)

# override default path where built files are put to allow running qgis without installing
# the modules go under libexec subdir
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR}/grass/modules)

#
# grass raster display module
#

ADD_EXECUTABLE(qgis.d.rast qgis.d.rast.c)
TARGET_LINK_LIBRARIES(qgis.d.rast
  ${GRASS_LIBRARY_gis}
  ${GRASS_LIBRARY_datetime}
  ${GDAL_LIBRARY}
)

#
# grass info module
#

ADD_EXECUTABLE(qgis.g.info qgis.g.info.c)
TARGET_LINK_LIBRARIES(qgis.g.info
  ${GRASS_LIBRARY_gis}
  ${GRASS_LIBRARY_datetime}
  ${GRASS_LIBRARY_gproj}
  ${GDAL_LIBRARY}
)

########################################################
# Install

INSTALL(TARGETS qgisgrass
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR})
       
INSTALL(TARGETS grassprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

INSTALL(TARGETS grassrasterprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

INSTALL(TARGETS qgis.d.rast qgis.g.info
        RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}/grass/modules 
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
