ADD_SUBDIRECTORY(modules)
ADD_SUBDIRECTORY(scripts)

#ADD_DEFINITIONS(-DGRASS_BASE="${GRASS_PREFIX}")
if (HAVE_OPENPTY)
  ADD_DEFINITIONS(-DHAVE_OPENPTY)
ENDIF (HAVE_OPENPTY)

IF (WIN32)
  ADD_DEFINITIONS(-DHAVE_GETPT)
ELSE (WIN32)
  ADD_DEFINITIONS(-DHAVE_POSIX_OPENPT -DHAVE_SYS_TIME_H)
ENDIF (WIN32)

# GRASS Direct disabled in 2.0
#ADD_DEFINITIONS(-DGRASS_DIRECT)

########################################################
# Files

SET (GRASS_PLUGIN_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassaddfeature.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrasseditrenderer.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassplugin.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassselect.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrasstools.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmapcalc.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmodule.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmoduleinput.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmoduleoptions.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmoduleparam.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassnewmapset.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassregion.cpp
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassutils.cpp
)


SET (GRASS_PLUGIN_UIS
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrasstoolsbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassselectbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmapcalcbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmodulebase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassnewmapsetbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassregionbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../../ui/qgsprojectionselectorbase.ui
)

SET (GRASS_PLUGIN_MOC_HDRS
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassaddfeature.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrasseditrenderer.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassplugin.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassselect.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrasstools.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmapcalc.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmodule.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmoduleinput.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmoduleoptions.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmoduleparam.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassnewmapset.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassregion.h
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassutils.h
)

IF(NOT WIN32)
  SET (GRASS_PLUGIN_SRCS ${GRASS_PLUGIN_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassshell.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/BlockArray.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/ColorScheme.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Emulation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/History.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/HistorySearch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/KeyboardTranslator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Pty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/SearchBar.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Screen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/ScreenWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/ShellCommand.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/TerminalCharacterDecoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/TerminalDisplay.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Vt102Emulation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/kprocess.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/k3processcontroller.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/konsole_wcwidth.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/kpty.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/kptydevice.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/kptyprocess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/qtermwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/tools.cpp
  )

  SET (GRASS_PLUGIN_UIS ${GRASS_PLUGIN_UIS}
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/SearchBar.ui
  )

  SET (GRASS_PLUGIN_MOC_HDRS ${GRASS_PLUGIN_MOC_HDRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassshell.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Emulation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Filter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/HistorySearch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Pty.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/ScreenWindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/SearchBar.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Session.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/TerminalDisplay.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Vt102Emulation.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/kprocess.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/k3processcontroller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/kptydevice.h
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/kptyprocess.h
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

  IF (APPLE)
    ADD_DEFINITIONS(-DHAVE_UTMPX -D_UTMPX_COMPAT)
  ENDIF(APPLE)
  CHECK_FUNCTION_EXISTS(updwtmpx HAVE_UPDWTMPX)
  IF(HAVE_UPDWTMPX)
    ADD_DEFINITIONS(-DHAVE_UPDWTMPX)
  ENDIF(HAVE_UPDWTMPX)


  SET(KB_LAYOUT_DIR "${QGIS_DATA_DIR}/grass/qtermwidget/kb-layouts")
  ADD_DEFINITIONS(-DKB_LAYOUT_DIR="${CMAKE_INSTALL_PREFIX}/${KB_LAYOUT_DIR}")

  SET(COLORSCHEMES_DIR "${QGIS_DATA_DIR}/grass/qtermwidget/color-schemes")
  ADD_DEFINITIONS(-DCOLORSCHEMES_DIR="${CMAKE_INSTALL_PREFIX}/${COLORSCHEMES_DIR}")

ENDIF(NOT WIN32)

SET (GRASS_PLUGIN_RCCS ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrass_plugin.qrc)
SET (GRASS_BROWSER_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qgis.g.browser.cpp)

########################################################
# Build


INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/app
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
  ${CMAKE_SOURCE_DIR}/src/gui/symbology
  ${CMAKE_SOURCE_DIR}/src/gui/layertree
  ${CMAKE_SOURCE_DIR}/src/providers/grass
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/app
  ${CMAKE_CURRENT_BINARY_DIR}
)
INCLUDE_DIRECTORIES(SYSTEM
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

    QT5_WRAP_UI (GRASS_PLUGIN_UIS_H ${GRASS_PLUGIN_UIS})
    QT5_WRAP_CPP (GRASS_PLUGIN_MOC_SRCS ${GRASS_PLUGIN_MOC_HDRS})
    QT5_ADD_RESOURCES(GRASS_PLUGIN_RCC_SRCS ${GRASS_PLUGIN_RCCS})

    INCLUDE_DIRECTORIES(
      ${CMAKE_BINARY_DIR}/src/providers/grass/${GRASS_BUILD_VERSION}
    )

    ADD_LIBRARY (grassplugin${GRASS_BUILD_VERSION} MODULE
      ${GRASS_PLUGIN_SRCS}
      ${GRASS_PLUGIN_MOC_SRCS}
      ${GRASS_PLUGIN_UIS_H}
      ${GRASS_PLUGIN_RCC_SRCS}
    )

    SET_TARGET_PROPERTIES(grassplugin${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" ${GRASS_EXTRA_INCLUDE_DIRS${GRASS_BUILD_VERSION}}"
    )

    IF(MSVC)
      SET_SOURCE_FILES_PROPERTIES(${GRASS_PLUGIN_SRCS} PROPERTIES COMPILE_FLAGS "-wd4611")
    ENDIF(MSVC)

    TARGET_LINK_LIBRARIES(grassplugin${GRASS_BUILD_VERSION}
      qgisgrass${GRASS_BUILD_VERSION}
      qgis_app
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
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" ${GRASS_EXTRA_INCLUDE_DIRS${GRASS_BUILD_VERSION}}"
    )

    TARGET_LINK_LIBRARIES (qgis.g.browser${GRASS_BUILD_VERSION}
      ${Qt5Gui_LIBRARIES}
      ${Qt5Widgets_LIBRARIES}
      ${Qt5Core_LIBRARIES}
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

IF(NOT WIN32)
  INSTALL(DIRECTORY qtermwidget/kb-layouts/ DESTINATION "${KB_LAYOUT_DIR}" FILES_MATCHING PATTERN "*.keytab" )
  INSTALL(DIRECTORY qtermwidget/kb-layouts/ DESTINATION "${KB_LAYOUT_DIR}" )
  INSTALL(DIRECTORY qtermwidget/color-schemes/ DESTINATION "${COLORSCHEMES_DIR}" FILES_MATCHING PATTERN "*.*schem*")
ENDIF(NOT WIN32)

FOREACH(GRASS_BUILD_VERSION 6 7) # GRASS 6 and GRASS 7
  IF(GRASS_FOUND${GRASS_BUILD_VERSION})
    ADD_SUBDIRECTORY(${GRASS_BUILD_VERSION})
  ENDIF(GRASS_FOUND${GRASS_BUILD_VERSION})
ENDFOREACH(GRASS_BUILD_VERSION)
