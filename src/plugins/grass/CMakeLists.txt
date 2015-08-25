ADD_SUBDIRECTORY(modules)
ADD_SUBDIRECTORY(scripts)

#ADD_DEFINITIONS(-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\")
ADD_DEFINITIONS(-DHAVE_OPENPTY=${HAVE_OPENPTY})
ADD_DEFINITIONS("-DGRASS_EXPORT=${DLLIMPORT} -DGRASS_LIB_EXPORT=${DLLIMPORT}")

IF (WIN32)
  ADD_DEFINITIONS(-DHAVE_GETPT)
ELSE (WIN32)
  ADD_DEFINITIONS(-DHAVE_POSIX_OPENPT)
ENDIF (WIN32)

# GRASS Direct disabled in 2.0
#ADD_DEFINITIONS("-DGRASS_DIRECT")

########################################################
# Files

SET (GRASS_PLUGIN_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassplugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassselect.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassedit.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassedittools.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrasstools.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmapcalc.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmodule.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmoduleoptions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmoduleparam.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassnewmapset.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassattributes.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassregion.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassutils.cpp
)


SET (GRASS_PLUGIN_UIS
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrasstoolsbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassselectbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrasseditbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmapcalcbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmodulebase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassnewmapsetbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassattributesbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassregionbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../../ui/qgsprojectionselectorbase.ui
)

SET (GRASS_PLUGIN_MOC_HDRS
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassplugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassselect.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassedit.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrasstools.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmapcalc.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmodule.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmoduleoptions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmoduleparam.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassnewmapset.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassattributes.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassregion.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassutils.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassedittools.h
)

IF(NOT WIN32)
  SET (GRASS_PLUGIN_SRCS ${GRASS_PLUGIN_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassshell.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/BlockArray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Emulation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/History.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/KeyboardTranslator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Pty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Screen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/ScreenWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/ShellCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/TerminalCharacterDecoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/TerminalDisplay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Vt102Emulation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/k3process.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/k3processcontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/konsole_wcwidth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/kpty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/qtermwidget.cpp
  )

  SET (GRASS_PLUGIN_MOC_HDRS ${GRASS_PLUGIN_MOC_HDRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassshell.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Emulation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Filter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Pty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/ScreenWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Session.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/TerminalDisplay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Vt102Emulation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/k3process.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/k3processcontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/qtermwidget.h
  )

  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    SET_SOURCE_FILES_PROPERTIES(
      ${CMAKE_CURRENT_BINARY_DIR}/qtermwidget/moc_Pty.cxx
      ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Pty.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Session.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/kpty.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/k3process.cpp
      PROPERTIES COMPILE_FLAGS "-Wno-overloaded-virtual -Wno-mismatched-tags"
    )
  ENDIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
ENDIF(NOT WIN32)

SET (GRASS_PLUGIN_RCCS ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrass_plugin.qrc)
SET (GRASS_BROWSER_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qgis.g.browser.cpp)

########################################################
# Build


INCLUDE_DIRECTORIES(
  ..
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../core
  ../../core/geometry
  ../../core/raster
  ../../gui
  ../../providers/grass
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
  qtermwidget
)

MACRO(ADD_GRASSPLUGIN GRASS_BUILD_VERSION)
    SET(GRASS_PREFIX ${GRASS_PREFIX${GRASS_BUILD_VERSION}})
    SET(GRASS_MAJOR_VERSION ${GRASS_MAJOR_VERSION${GRASS_BUILD_VERSION}})
    SET(GRASS_MINOR_VERSION ${GRASS_MINOR_VERSION${GRASS_BUILD_VERSION}})

    QT4_WRAP_UI (GRASS_PLUGIN_UIS_H ${GRASS_PLUGIN_UIS})
    QT4_WRAP_CPP (GRASS_PLUGIN_MOC_SRCS ${GRASS_PLUGIN_MOC_HDRS})
    QT4_ADD_RESOURCES(GRASS_PLUGIN_RCC_SRCS ${GRASS_PLUGIN_RCCS})

    ADD_LIBRARY (grassplugin${GRASS_BUILD_VERSION} MODULE
      ${GRASS_PLUGIN_SRCS}
      ${GRASS_PLUGIN_MOC_SRCS}
      ${GRASS_PLUGIN_UIS_H}
      ${GRASS_PLUGIN_RCC_SRCS}
    )

    SET_TARGET_PROPERTIES(grassplugin${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\""
    )

    TARGET_LINK_LIBRARIES(grassplugin${GRASS_BUILD_VERSION}
      qgisgrass${GRASS_BUILD_VERSION}
      qgis_gui
      ${OPENPTY_LIBRARY}
    )

    IF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
      TARGET_LINK_LIBRARIES(grassplugin${GRASS_BUILD_VERSION} ulog)
    ENDIF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

    # override default path where built files are put to allow running qgis without installing
    # the binary goes under libexec subdir
    SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR}/grass/bin)


    ADD_EXECUTABLE(qgis.g.browser${GRASS_BUILD_VERSION} ${GRASS_BROWSER_SRCS})

    SET_TARGET_PROPERTIES(qgis.g.browser${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\""
    )

    TARGET_LINK_LIBRARIES (qgis.g.browser${GRASS_BUILD_VERSION}
      ${QT_QTGUI_LIBRARY}
      ${QT_QTCORE_LIBRARY}
    )

    ########################################################
    # Install

    INSTALL(TARGETS grassplugin${GRASS_BUILD_VERSION}
      RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
      LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

    INSTALL(TARGETS qgis.g.browser${GRASS_BUILD_VERSION}
        RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}/grass/bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
ENDMACRO(ADD_GRASSPLUGIN)

FILE (GLOB GMLS *.gml)
INSTALL (FILES ${GMLS}
         DESTINATION ${QGIS_DATA_DIR}/grass)

INSTALL (FILES themes/default/grass_arrow.svg themes/default/grass_plus.svg themes/default/grass_arrow.png themes/default/grass_plus.png
         DESTINATION ${QGIS_DATA_DIR}/grass/modules)

FOREACH(GRASS_BUILD_VERSION 6 7) # GRASS 6 and GRASS 7
  IF(GRASS_FOUND${GRASS_BUILD_VERSION})
    ADD_SUBDIRECTORY(${GRASS_BUILD_VERSION})
  ENDIF(GRASS_FOUND${GRASS_BUILD_VERSION})
ENDFOREACH(GRASS_BUILD_VERSION)
