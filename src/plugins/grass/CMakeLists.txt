IF (GRASS_NUM_VERSION LESS 60300)
  SET (GRASS_MODULES_DIR "modules-6.3")
ELSE (GRASS_NUM_VERSION LESS 60300)
  SET (GRASS_MODULES_DIR "modules-${GRASS_MAJOR_VERSION}.${GRASS_MINOR_VERSION}")
ENDIF (GRASS_NUM_VERSION LESS 60300)

IF (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${GRASS_MODULES_DIR}")
  MESSAGE (SEND_ERROR "Your GRASS version is not supported (${CMAKE_CURRENT_SOURCE_DIR}/${GRASS_MODULES_DIR} is not found).")
ENDIF (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${GRASS_MODULES_DIR}")
IF (NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${GRASS_MODULES_DIR}")
  MESSAGE (SEND_ERROR "Your GRASS version is not supported (${CMAKE_CURRENT_SOURCE_DIR}/${GRASS_MODULES_DIR} is not a Directory).")
ENDIF (NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${GRASS_MODULES_DIR}")

ADD_SUBDIRECTORY(modules-common)
ADD_SUBDIRECTORY(${GRASS_MODULES_DIR})
ADD_SUBDIRECTORY(scripts)

ADD_DEFINITIONS(-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\")
ADD_DEFINITIONS(-DHAVE_OPENPTY=${HAVE_OPENPTY})
ADD_DEFINITIONS("-DGRASS_EXPORT=${DLLIMPORT} -DGRASS_LIB_EXPORT=${DLLIMPORT}")

IF (WIN32)
  ADD_DEFINITIONS(-DHAVE_GETPT)
ELSE (WIN32)
  ADD_DEFINITIONS(-DHAVE_POSIX_OPENPT)
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
     qgsgrassattributes.cpp
     qgsgrassregion.cpp
     qgsgrassutils.cpp
)


SET (GRASS_PLUGIN_UIS
     qgsgrasstoolsbase.ui
     qgsgrassselectbase.ui
     qgsgrasseditbase.ui
     qgsgrassmapcalcbase.ui
     qgsgrassmodulebase.ui
     qgsgrassnewmapsetbase.ui
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
     qgsgrassattributes.h
     qgsgrassregion.h
     qgsgrassutils.h
     qgsgrassedittools.h
)

IF(NOT WIN32)
	SET (GRASS_PLUGIN_SRCS ${GRASS_PLUGIN_SRCS}
     		qgsgrassshell.cpp
		qtermwidget/BlockArray.cpp
		qtermwidget/Emulation.cpp
		qtermwidget/Filter.cpp
		qtermwidget/History.cpp
		qtermwidget/KeyboardTranslator.cpp
		qtermwidget/Pty.cpp
		qtermwidget/Screen.cpp
		qtermwidget/ScreenWindow.cpp
		qtermwidget/Session.cpp
		qtermwidget/ShellCommand.cpp
		qtermwidget/TerminalCharacterDecoder.cpp
		qtermwidget/TerminalDisplay.cpp
		qtermwidget/Vt102Emulation.cpp
		qtermwidget/k3process.cpp
		qtermwidget/k3processcontroller.cpp
		qtermwidget/konsole_wcwidth.cpp
		qtermwidget/kpty.cpp
		qtermwidget/qtermwidget.cpp
		)

	SET (GRASS_PLUGIN_MOC_HDRS ${GRASS_PLUGIN_MOC_HDRS}
     		qgsgrassshell.h
		qtermwidget/Emulation.h
		qtermwidget/Filter.h
		qtermwidget/Pty.h
		qtermwidget/ScreenWindow.h
		qtermwidget/Session.h
		qtermwidget/TerminalDisplay.h
		qtermwidget/Vt102Emulation.h
		qtermwidget/k3process.h
		qtermwidget/k3processcontroller.h
		qtermwidget/qtermwidget.h
		)
ENDIF(NOT WIN32)

SET (GRASS_PLUGIN_RCCS  qgsgrass_plugin.qrc)

SET (GRASS_BROWSER_SRCS qgis.g.browser.cpp)

########################################################
# Build

QT4_WRAP_UI (GRASS_PLUGIN_UIS_H  ${GRASS_PLUGIN_UIS})

QT4_WRAP_CPP (GRASS_PLUGIN_MOC_SRCS  ${GRASS_PLUGIN_MOC_HDRS})

QT4_ADD_RESOURCES(GRASS_PLUGIN_RCC_SRCS ${GRASS_PLUGIN_RCCS})

########################################################
# build lib

ADD_LIBRARY (grassplugin MODULE 
  ${GRASS_PLUGIN_SRCS} 
  ${GRASS_PLUGIN_MOC_SRCS} 
  ${GRASS_PLUGIN_UIS_H} 
  ${GRASS_PLUGIN_RCC_SRCS}
  )

INCLUDE_DIRECTORIES(
     ..
     ${CMAKE_CURRENT_BINARY_DIR}
     ../../core 
     ../../core/raster 
     ../../core/renderer 
     ../../core/symbology
     ../../gui
     ../../providers/grass
     ${CMAKE_CURRENT_BINARY_DIR}/../../ui
     ${GRASS_INCLUDE_DIR}
     ${GDAL_INCLUDE_DIR}
     ${PROJ_INCLUDE_DIR}
     ${GEOS_INCLUDE_DIR}
     qtermwidget
)

TARGET_LINK_LIBRARIES(grassplugin
  qgisgrass
  qgis_gui
  grassprovider
  ${OPENPTY_LIBRARY}
)

ADD_EXECUTABLE(qgis.g.browser ${GRASS_BROWSER_SRCS})

TARGET_LINK_LIBRARIES (qgis.g.browser
  ${QT_QTGUI_LIBRARY}
  ${QT_QTCORE_LIBRARY}
)

########################################################
# Install

INSTALL(TARGETS grassplugin
  RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
  LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})


FILE (GLOB GMLS *.gml)
INSTALL (FILES ${GMLS}
         DESTINATION ${QGIS_DATA_DIR}/grass)

INSTALL (FILES themes/gis/grass_arrow.svg themes/gis/grass_plus.svg themes/gis/grass_arrow.png themes/gis/grass_plus.png
         DESTINATION ${QGIS_DATA_DIR}/grass/modules)

INSTALL(TARGETS qgis.g.browser 
        RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}/grass/bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
