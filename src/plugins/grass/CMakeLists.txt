include(CopyResources)

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

set (GRASS_PLUGIN_HDRS
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

set (GRASS_PLUGIN_RCCS ${CMAKE_CURRENT_SOURCE_DIR}/qgsgrass_plugin.qrc)
set (GRASS_BROWSER_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/qgis.g.browser.cpp)

########################################################
# Build


include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/plugins/grass
  ${CMAKE_SOURCE_DIR}/src/app
  ${CMAKE_SOURCE_DIR}/src/providers/grass

  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/app
  ${CMAKE_CURRENT_BINARY_DIR}
)

macro(ADD_GRASSPLUGIN GRASS_BUILD_VERSION)
    set(GRASS_PREFIX ${GRASS_PREFIX${GRASS_BUILD_VERSION}})
    set(GRASS_MAJOR_VERSION ${GRASS_MAJOR_VERSION${GRASS_BUILD_VERSION}})
    set(GRASS_MINOR_VERSION ${GRASS_MINOR_VERSION${GRASS_BUILD_VERSION}})

    qt_wrap_ui(GRASS_PLUGIN_UIS_H ${GRASS_PLUGIN_UIS})

    include_directories(
      ${CMAKE_BINARY_DIR}/src/providers/grass/${GRASS_BUILD_VERSION}
    )

    add_library (plugin_grass${GRASS_BUILD_VERSION} MODULE
      ${GRASS_PLUGIN_SRCS}
      ${GRASS_PLUGIN_HDRS}
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

file (GLOB GMLS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.gml)
ADD_QGIS_RESOURCES("${CMAKE_CURRENT_SOURCE_DIR}" grass DEST_GRASS_GML_RESOURCES "${GMLS}")

set (GRASS_ICONS
    grass_arrow.svg
    grass_plus.svg
    grass_arrow.png
    grass_plus.png )
ADD_QGIS_RESOURCES("${CMAKE_CURRENT_SOURCE_DIR}/themes/default" grass/modules DEST_GRASS_MODULE_ICONS "${GRASS_ICONS}")

set (DEST_GRASS_RESOURCES
    "${DEST_GRASS_GML_RESOURCES}"
    "${DEST_GRASS_MODULE_ICONS}")

add_custom_target(grass_resources ALL DEPENDS ${DEST_GRASS_RESOURCES})

foreach(GRASS_BUILD_VERSION 6 7 8)
  if(GRASS_FOUND${GRASS_BUILD_VERSION})
    add_subdirectory(${GRASS_BUILD_VERSION})
  endif()
endforeach(GRASS_BUILD_VERSION)
