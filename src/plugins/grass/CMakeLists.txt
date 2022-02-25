add_subdirectory(modules)
add_subdirectory(scripts)

#add_definitions(-DGRASS_BASE="${GRASS_PREFIX}")
if (HAVE_OPENPTY)
  add_definitions(-DHAVE_OPENPTY)
endif()

if (WIN32)
  add_definitions(-DHAVE_GETPT)
else()
  add_definitions(-DHAVE_POSIX_OPENPT -DHAVE_SYS_TIME_H)
endif()

# GRASS Direct disabled in 2.0
#add_definitions(-DGRASS_DIRECT)

########################################################
# Files

set (GRASS_PLUGIN_SRCS
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


set (GRASS_PLUGIN_UIS
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrasstoolsbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassselectbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmapcalcbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassmodulebase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassnewmapsetbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrassregionbase.ui
   ${CMAKE_CURRENT_SOURCE_DIR}/../../ui/qgsprojectionselectorbase.ui
)

set (GRASS_PLUGIN_MOC_HDRS
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

if(NOT WIN32)
  set (GRASS_PLUGIN_SRCS ${GRASS_PLUGIN_SRCS}
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

  set (GRASS_PLUGIN_UIS ${GRASS_PLUGIN_UIS}
    ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/SearchBar.ui
  )

  set (GRASS_PLUGIN_MOC_HDRS ${GRASS_PLUGIN_MOC_HDRS}
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

  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set_source_files_properties(
      ${CMAKE_CURRENT_BINARY_DIR}/qtermwidget/moc_Pty.cxx
      ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Pty.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/Session.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/kpty.cpp
      ${CMAKE_CURRENT_SOURCE_DIR}/qtermwidget/k3process.cpp
      PROPERTIES COMPILE_FLAGS "-Wno-overloaded-virtual -Wno-mismatched-tags"
    )
  endif()

  if (APPLE)
    add_definitions(-DHAVE_UTMPX -D_UTMPX_COMPAT)
  endif()
  CHECK_FUNCTION_EXISTS(updwtmpx HAVE_UPDWTMPX)
  if(HAVE_UPDWTMPX)
    add_definitions(-DHAVE_UPDWTMPX)
  endif()


  set(KB_LAYOUT_DIR "${QGIS_DATA_DIR}/grass/qtermwidget/kb-layouts")
  add_definitions(-DKB_LAYOUT_DIR="${CMAKE_INSTALL_PREFIX}/${KB_LAYOUT_DIR}")

  set(COLORSCHEMES_DIR "${QGIS_DATA_DIR}/grass/qtermwidget/color-schemes")
  add_definitions(-DCOLORSCHEMES_DIR="${CMAKE_INSTALL_PREFIX}/${COLORSCHEMES_DIR}")

endif()

set (GRASS_PLUGIN_RCCS ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrass_plugin.qrc)
set (GRASS_BROWSER_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qgis.g.browser.cpp)

########################################################
# Build


include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/app
  ${CMAKE_SOURCE_DIR}/src/providers/grass

  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/app
  ${CMAKE_CURRENT_BINARY_DIR}
)
include_directories(SYSTEM
  ${GDAL_INCLUDE_DIR}
  ${PROJ_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${POSTGRES_INCLUDE_DIR}
  qtermwidget
)


macro(ADD_GRASSPLUGIN GRASS_BUILD_VERSION)
    set(GRASS_PREFIX ${GRASS_PREFIX${GRASS_BUILD_VERSION}})
    set(GRASS_MAJOR_VERSION ${GRASS_MAJOR_VERSION${GRASS_BUILD_VERSION}})
    set(GRASS_MINOR_VERSION ${GRASS_MINOR_VERSION${GRASS_BUILD_VERSION}})

    if (WITH_QT6)
      QT6_WRAP_UI (GRASS_PLUGIN_UIS_H ${GRASS_PLUGIN_UIS})
      QT6_WRAP_CPP (GRASS_PLUGIN_MOC_SRCS ${GRASS_PLUGIN_MOC_HDRS})
    else()
      QT5_WRAP_UI (GRASS_PLUGIN_UIS_H ${GRASS_PLUGIN_UIS})
      QT5_WRAP_CPP (GRASS_PLUGIN_MOC_SRCS ${GRASS_PLUGIN_MOC_HDRS})
    endif()

    include_directories(
      ${CMAKE_BINARY_DIR}/src/providers/grass/${GRASS_BUILD_VERSION}
    )

    add_library (plugin_grass${GRASS_BUILD_VERSION} MODULE
      ${GRASS_PLUGIN_SRCS}
      ${GRASS_PLUGIN_MOC_SRCS}
      ${GRASS_PLUGIN_UIS_H}
      ${GRASS_PLUGIN_RCCS}
    )

    set_target_properties(plugin_grass${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" ${GRASS_EXTRA_INCLUDE_DIRS${GRASS_BUILD_VERSION}}"
    )

    if(MSVC)
      set_source_files_properties(${GRASS_PLUGIN_SRCS} PROPERTIES COMPILE_FLAGS "-wd4611")
    endif()

    target_compile_features(plugin_grass${GRASS_BUILD_VERSION} PRIVATE cxx_std_17)

    target_link_libraries(plugin_grass${GRASS_BUILD_VERSION}
      qgisgrass${GRASS_BUILD_VERSION}
      qgis_app
      qgis_gui
      ${OPENPTY_LIBRARY}
    )

    if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
      target_link_libraries(plugin_grass${GRASS_BUILD_VERSION} ulog)
    endif()

    # override default path where built files are put to allow running qgis without installing
    # the binary goes under libexec subdir
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR}/grass/bin)


    add_executable(qgis.g.browser${GRASS_BUILD_VERSION} ${GRASS_BROWSER_SRCS})

    set_target_properties(qgis.g.browser${GRASS_BUILD_VERSION} PROPERTIES
      COMPILE_FLAGS "-DGRASS_BASE=\\\"${GRASS_PREFIX}\\\" ${GRASS_EXTRA_INCLUDE_DIRS${GRASS_BUILD_VERSION}}"
    )

    target_link_libraries (qgis.g.browser${GRASS_BUILD_VERSION}
      ${QT_VERSION_BASE}::Gui
      ${QT_VERSION_BASE}::Widgets
      ${QT_VERSION_BASE}::Core
    )

    ########################################################
    # Install

    install(TARGETS plugin_grass${GRASS_BUILD_VERSION}
      RUNTIME DESTINATION ${QGIS_PLUGIN_DIR}
      LIBRARY DESTINATION ${QGIS_PLUGIN_DIR})

    install(TARGETS qgis.g.browser${GRASS_BUILD_VERSION}
        RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}/grass/bin
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    )
endmacro(ADD_GRASSPLUGIN)

file (GLOB GMLS *.gml)
install (FILES ${GMLS}
         DESTINATION ${QGIS_DATA_DIR}/grass)

install (FILES themes/default/grass_arrow.svg themes/default/grass_plus.svg themes/default/grass_arrow.png themes/default/grass_plus.png
         DESTINATION ${QGIS_DATA_DIR}/grass/modules)

if(NOT WIN32)
  install(DIRECTORY qtermwidget/kb-layouts/ DESTINATION "${KB_LAYOUT_DIR}" FILES_MATCHING PATTERN "*.keytab" )
  install(DIRECTORY qtermwidget/kb-layouts/ DESTINATION "${KB_LAYOUT_DIR}" )
  install(DIRECTORY qtermwidget/color-schemes/ DESTINATION "${COLORSCHEMES_DIR}" FILES_MATCHING PATTERN "*.*schem*")
endif()

foreach(GRASS_BUILD_VERSION 6 7 8)
  if(GRASS_FOUND${GRASS_BUILD_VERSION})
    add_subdirectory(${GRASS_BUILD_VERSION})
  endif()
endforeach(GRASS_BUILD_VERSION)
