ADD_DEFINITIONS(-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\")

########################################################
# Files

SET(GRASS_PROVIDER_SRCS provider.cpp)

SET(GRASS_LIB_SRCS qgsgrassprovider.cpp qgsgrass.cpp)

IF (NOT MSVC)
  IF (PEDANTIC)
    MESSAGE("providers/grass : -Werror removed for qgsgrassprovider.cpp for now - please get rid of any compiler warnings!")
  ENDIF (PEDANTIC)
  # The warnings are caused by multiple definitions of NDEBUG in grass sources
  # I have submitted a bug to the grass folks in the meantime we need to
  # disable treating warnings as errors for the affected files
  SET_SOURCE_FILES_PROPERTIES(qgsgrassprovider.cpp PROPERTIES COMPILE_FLAGS -Wno-error )
  SET_SOURCE_FILES_PROPERTIES(qgsgrass.cpp PROPERTIES COMPILE_FLAGS -Wno-error )   
  SET_SOURCE_FILES_PROPERTIES(provider.cpp PROPERTIES COMPILE_FLAGS -Wno-error )
ENDIF (NOT MSVC)

########################################################
# Build

INCLUDE_DIRECTORIES (
  ../../core
  ${GRASS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
)


ADD_LIBRARY (qgisgrass SHARED ${GRASS_LIB_SRCS})

SET_TARGET_PROPERTIES(qgisgrass PROPERTIES 
			VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}
			SOVERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR})

IF (WIN32)
  SET_TARGET_PROPERTIES(qgisgrass PROPERTIES COMPILE_FLAGS "\"-DGRASS_EXPORT=__declspec(dllexport)\"" )
ELSE (WIN32)
  SET_TARGET_PROPERTIES(qgisgrass PROPERTIES COMPILE_FLAGS "-DGRASS_EXPORT=" )
ENDIF (WIN32)

TARGET_LINK_LIBRARIES (qgisgrass
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTXML_LIBRARY} 
  ${QT_QT3SUPPORT_LIBRARY} 
  ${GRASS_LIBRARIES}
  qgis_core
)
#work aroud for the fact that qt3support on mac is linked to qtsql
IF (APPLE)
  TARGET_LINK_LIBRARIES (qgisgrass
    ${QT_QTSQL_LIBRARY} 
    )
ENDIF (APPLE)

ADD_LIBRARY (grassprovider MODULE ${GRASS_PROVIDER_SRCS})

IF (WIN32)
  SET_TARGET_PROPERTIES(grassprovider PROPERTIES COMPILE_FLAGS "\"-DGRASS_EXPORT=__declspec(dllimport)\"" )
ELSE (WIN32)
  SET_TARGET_PROPERTIES(grassprovider PROPERTIES COMPILE_FLAGS "-DGRASS_EXPORT=" )
ENDIF (WIN32)

TARGET_LINK_LIBRARIES (grassprovider
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTXML_LIBRARY} 
  qgis_core
  qgisgrass
)

########################################################
# Install

INSTALL(TARGETS qgisgrass
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR})
       
INSTALL(TARGETS grassprovider
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})
