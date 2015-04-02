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

  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    SET_SOURCE_FILES_PROPERTIES(
      ${CMAKE_CURRENT_BINARY_DIR}/qtermwidget/moc_Pty.cxx
      qtermwidget/Pty.cpp
      qtermwidget/Session.cpp
      qtermwidget/kpty.cpp
      qtermwidget/k3process.cpp
      PROPERTIES COMPILE_FLAGS "-Wno-overloaded-virtual -Wno-mismatched-tags"
    )
  ENDIF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
ENDIF(NOT WIN32)

SET (GRASS_PLUGIN_RCCS  qgsgrass_plugin.qrc)

SET (GRASS_BROWSER_SRCS qgis.g.browser.cpp)

########################################################
# Build

QT4_WRAP_UI (GRASS_PLUGIN_UIS_H  ${GRASS_PLUGIN_UIS})

QT4_WRAP_CPP (GRASS_PLUGIN_MOC_SRCS  ${GRASS_PLUGIN_MOC_HDRS})

QT4_ADD_RESOURCES(GRASS_PLUGIN_RCC_SRCS ${GRASS_PLUGIN_RCCS})

INCLUDE_DIRECTORIES(
  ..
  ${CMAKE_CURRENT_BINARY_DIR}
  ../../core
  ../../core/raster
  ../../gui
  ../../providers/grass
  ${CMAKE_CURRENT_BINARY_DIR}/../../ui
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  qtermwidget
)

FOREACH(GRASS_BUILD_VERSION 6) # GRASS 6 and GRASS 7
  IF(GRASS_FOUND${GRASS_BUILD_VERSION})
    SET(GRASS_PREFIX ${GRASS_PREFIX${GRASS_BUILD_VERSION}})
    SET(GRASS_MAJOR_VERSION ${GRASS_MAJOR_VERSION${GRASS_BUILD_VERSION}})
    SET(GRASS_MINOR_VERSION ${GRASS_MINOR_VERSION${GRASS_BUILD_VERSION}})

    GET_PROPERTY(INCLUDE_DIRECTORIES DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
    LIST(APPEND INCLUDE_DIRECTORIES ${GRASS_INCLUDE_DIR${GRASS_BUILD_VERSION}})

    ADD_LIBRARY (grassplugin${GRASS_BUILD_VERSION} MODULE
      ${GRASS_PLUGIN_SRCS}
      ${GRASS_PLUGIN_MOC_SRCS}
      ${GRASS_PLUGIN_UIS_H}
      ${GRASS_PLUGIN_RCC_SRCS}
    )

    SET_TARGET_PROPERTIES(grassplugin${GRASS_BUILD_VERSION} PROPERTIES
      INCLUDE_DIRECTORIES "${INCLUDE_DIRECTORIES}"
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
      INCLUDE_DIRECTORIES "${INCLUDE_DIRECTORIES}"
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


    FILE (GLOB GMLS *.gml)
    INSTALL (FILES ${GMLS}
             DESTINATION ${QGIS_DATA_DIR}/grass)

    INSTALL (FILES themes/default/grass_arrow.svg themes/default/grass_plus.svg themes/default/grass_arrow.png themes/default/grass_plus.png
             DESTINATION ${QGIS_DATA_DIR}/grass/modules)

    INSTALL(TARGETS qgis.g.browser${GRASS_BUILD_VERSION}
            RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}/grass/bin
            PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
  ENDIF(GRASS_FOUND${GRASS_BUILD_VERSION})
ENDFOREACH(GRASS_BUILD_VERSION 6 7)
