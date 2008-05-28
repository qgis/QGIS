
SUBDIRS(config modules themes)

ADD_DEFINITIONS(-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\")
ADD_DEFINITIONS(-DHAVE_OPENPTY=${HAVE_OPENPTY})

IF (NOT MSVC)
  IF (PEDANTIC)
    MESSAGE("providers/grass : -Werror removed for qgsgrassplugin.cpp for now - please get rid of any compiler warnings!")
  ENDIF (PEDANTIC)
  # The warnings are caused by multiple definitions of NDEBUG in grass sources
  # I have submitted a bug to the grass folks in the meantime we need to
  # disable treating warnings as errors for the affected files TS
  FILE (GLOB files *.cpp)
  SET_SOURCE_FILES_PROPERTIES(${files} PROPERTIES COMPILE_FLAGS -Wno-error ) 
ENDIF (NOT MSVC)

IF (WIN32)
  ADD_DEFINITIONS("\"-DGRASS_EXPORT=__declspec(dllimport)\"")
ELSE (WIN32)
  ADD_DEFINITIONS("-DGRASS_EXPORT=")
ENDIF (WIN32)

########################################################
# Files

SET (GRASS_PLUGIN_SRCS
     qgsgrassplugin.cpp
     qgsgrassselect.cpp
     qgsgrassbrowser.cpp
     qgsgrassedit.cpp
     qgsgrassedittools.cpp
     qgsgrasstools.cpp
     qgsgrassmodel.cpp
     qgsgrassmapcalc.cpp
     qgsgrassmodule.cpp
     qgsgrassnewmapset.cpp
     qgsgrassshell.cpp
     qgsgrassattributes.cpp
     qgsgrassregion.cpp
     qgsgrassutils.cpp
)

SET (GRASS_PLUGIN_UIS
     qgsgrassselectbase.ui
     qgsgrasseditbase.ui
     qgsgrassmapcalcbase.ui
     qgsgrassmodulebase.ui
     qgsgrassnewmapsetbase.ui
     qgsgrassshellbase.ui
     qgsgrassattributesbase.ui
     qgsgrassregionbase.ui
     ../../ui/qgsprojectionselectorbase.ui
)

SET (GRASS_PLUGIN_MOC_HDRS
     qgsgrassplugin.h
     qgsgrassselect.h
     qgsgrassbrowser.h
     qgsgrassedit.h
     qgsgrasstools.h
     qgsgrassmodel.h
     qgsgrassmapcalc.h
     qgsgrassmodule.h
     qgsgrassnewmapset.h
     qgsgrassshell.h
     qgsgrassattributes.h
     qgsgrassregion.h
     qgsgrassutils.h
)

SET (GRASS_PLUGIN_RCCS  qgsgrass_plugin.qrc)


########################################################
# Build

QT4_WRAP_UI (GRASS_PLUGIN_UIS_H  ${GRASS_PLUGIN_UIS})

QT4_WRAP_CPP (GRASS_PLUGIN_MOC_SRCS  ${GRASS_PLUGIN_MOC_HDRS})

QT4_ADD_RESOURCES(GRASS_PLUGIN_RCC_SRCS ${GRASS_PLUGIN_RCCS})

########################################################
# deal with warnings
IF (NOT MSVC)
  IF (PEDANTIC)
    MESSAGE("providers/grass : -Werror removed for qgsgrassplugin.cpp for now - please get rid of any compiler warnings!")
  ENDIF (PEDANTIC)
  # The warnings are caused by multiple definitions of NDEBUG in grass sources
  # I have submitted a bug to teh grass folks in the meantime we need to
  # disable treating warnings as errors for the affected files
  FILE (GLOB files *.cpp)
  SET_SOURCE_FILES_PROPERTIES(${files} PROPERTIES COMPILE_FLAGS -Wno-error )
  SET_SOURCE_FILES_PROPERTIES(${GRASS_PLUGIN_MOC_SRCS} PROPERTIES COMPILE_FLAGS -Wno-error )
ENDIF (NOT MSVC)

########################################################
# build lib

ADD_LIBRARY (grassplugin MODULE 
  ${GRASS_PLUGIN_SRCS} 
  ${GRASS_PLUGIN_MOC_SRCS} 
  ${GRASS_PLUGIN_UIS_H} 
  ${GRASS_PLUGIN_RCC_SRCS}
  )

INCLUDE_DIRECTORIES(
     .
     ..
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core 
     ../../core/raster 
     ../../core/renderer 
     ../../core/symbology
     ../../gui
     ${CMAKE_CURRENT_BINARY_DIR}/../../ui
     ${GRASS_INCLUDE_DIR}
     ${GDAL_INCLUDE_DIR}
     ${PROJ_INCLUDE_DIR}
)

TARGET_LINK_LIBRARIES(grassplugin
  qgis_core
  qgis_gui
  qgisgrass
  ${GRASS_LIBRARIES}
)


########################################################
# Install

INSTALL(TARGETS grassplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})


FILE (GLOB GMLS *.gml)
INSTALL (FILES ${GMLS} images/world.png
         DESTINATION ${QGIS_DATA_DIR}/grass)
