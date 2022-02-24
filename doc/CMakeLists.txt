# include doxygen documentation
set (WITH_APIDOC FALSE CACHE BOOL "Determines whether the QGIS API doxygen documentation should be built")

# include Qt documentation
set (QT_DOC_URL "https://doc.qt.io/qt-5/" CACHE STRING "URL for Qt docs")

set(QGIS_DOC_FILES
  ../INSTALL.md
  ../NEWS.md
  NEWS.html
  index.html
  developersmap.html
  nohelp.html
  favicon.ico
  style.css
  AUTHORS
  CONTRIBUTORS
  SPONSORS
  DONORS
  TRANSLATORS
  LICENSE
)

install(FILES ${QGIS_DOC_FILES} DESTINATION ${QGIS_DATA_DIR}/doc)
install(FILES ../images/icons/qgis-icon-60x60.png DESTINATION ${QGIS_DATA_DIR}/doc/images)

# Sponsor images
file (GLOB SPONSOR_IMAGES images/projects/thumbnails/* )
install (FILES ${SPONSOR_IMAGES} DESTINATION ${QGIS_DATA_DIR}/doc/images/projects/thumbnails/)

if(WITH_APIDOC)
  find_package(Doxygen 1.8.2 REQUIRED)
  if(DOXYGEN_FOUND)
    set(QHP_FILES ${CMAKE_CURRENT_BINARY_DIR}/api/html/index.qhp)

    # Whether to generate QHP file
    set (GENERATE_QHP FALSE CACHE BOOL "Determines whether to generate Qt QHP help from the QGIS API Docs")
    if (GENERATE_QHP)
      set(WITH_QHP YES)
      set(QHP_FILES ${QHP_FILES} ${CMAKE_CURRENT_BINARY_DIR}/api/qch/qgis.qch)
    else()
      set(WITH_QHP NO)
    endif()

    set(DOXYGEN_INCLUDE_PATH
      ${CMAKE_SOURCE_DIR}/src/core
    )
    string(REPLACE ";" " " DOXYGEN_INCLUDE_PATH "${DOXYGEN_INCLUDE_PATH}")

    set(DOXYGEN_INPUT
      ${CMAKE_SOURCE_DIR}/doc
      ${CMAKE_SOURCE_DIR}/src/core
      ${CMAKE_SOURCE_DIR}/src/core/3d
      ${CMAKE_SOURCE_DIR}/src/core/annotations
      ${CMAKE_SOURCE_DIR}/src/core/auth
      ${CMAKE_SOURCE_DIR}/src/core/browser
      ${CMAKE_SOURCE_DIR}/src/core/callouts
      ${CMAKE_SOURCE_DIR}/src/core/classification
      ${CMAKE_SOURCE_DIR}/src/core/diagram
      ${CMAKE_SOURCE_DIR}/src/core/dxf
      ${CMAKE_SOURCE_DIR}/src/core/editform
      ${CMAKE_SOURCE_DIR}/src/core/effects
      ${CMAKE_SOURCE_DIR}/src/core/expression
      ${CMAKE_SOURCE_DIR}/src/core/externalstorage
      ${CMAKE_SOURCE_DIR}/src/core/fieldformatter
      ${CMAKE_SOURCE_DIR}/src/core/geocms
      ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
      ${CMAKE_SOURCE_DIR}/src/core/geocoding
      ${CMAKE_SOURCE_DIR}/src/core/geometry
      ${CMAKE_SOURCE_DIR}/src/core/gps
      ${CMAKE_SOURCE_DIR}/src/core/labeling
      ${CMAKE_SOURCE_DIR}/src/core/layertree
      ${CMAKE_SOURCE_DIR}/src/core/layout
      ${CMAKE_SOURCE_DIR}/src/core/locator
      ${CMAKE_SOURCE_DIR}/src/core/maprenderer
      ${CMAKE_SOURCE_DIR}/src/core/mesh
      ${CMAKE_SOURCE_DIR}/src/core/metadata
      ${CMAKE_SOURCE_DIR}/src/core/network
      ${CMAKE_SOURCE_DIR}/src/core/numericformats
      ${CMAKE_SOURCE_DIR}/src/core/pal
      ${CMAKE_SOURCE_DIR}/src/core/pointcloud
      ${CMAKE_SOURCE_DIR}/src/core/processing
      ${CMAKE_SOURCE_DIR}/src/core/processing/models
      ${CMAKE_SOURCE_DIR}/src/core/proj
      ${CMAKE_SOURCE_DIR}/src/core/project
      ${CMAKE_SOURCE_DIR}/src/core/providers
      ${CMAKE_SOURCE_DIR}/src/core/providers/arcgis
      ${CMAKE_SOURCE_DIR}/src/core/providers/memory
      ${CMAKE_SOURCE_DIR}/src/core/raster
      ${CMAKE_SOURCE_DIR}/src/core/scalebar
      ${CMAKE_SOURCE_DIR}/src/core/settings
      ${CMAKE_SOURCE_DIR}/src/core/symbology
      ${CMAKE_SOURCE_DIR}/src/core/textrenderer
      ${CMAKE_SOURCE_DIR}/src/core/validity
      ${CMAKE_SOURCE_DIR}/src/core/vector
      ${CMAKE_SOURCE_DIR}/src/core/vectortile
      ${CMAKE_SOURCE_DIR}/src/gui
      ${CMAKE_SOURCE_DIR}/src/gui/annotations
      ${CMAKE_SOURCE_DIR}/src/gui/attributetable
      ${CMAKE_SOURCE_DIR}/src/gui/auth
      ${CMAKE_SOURCE_DIR}/src/gui/callouts
      ${CMAKE_SOURCE_DIR}/src/gui/codeeditors
      ${CMAKE_SOURCE_DIR}/src/gui/devtools
      ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
      ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets/core
      ${CMAKE_SOURCE_DIR}/src/gui/effects
      ${CMAKE_SOURCE_DIR}/src/gui/history
      ${CMAKE_SOURCE_DIR}/src/gui/labeling
      ${CMAKE_SOURCE_DIR}/src/gui/layertree
      ${CMAKE_SOURCE_DIR}/src/gui/layout
      ${CMAKE_SOURCE_DIR}/src/gui/locator
      ${CMAKE_SOURCE_DIR}/src/gui/maptools
      ${CMAKE_SOURCE_DIR}/src/gui/mesh
      ${CMAKE_SOURCE_DIR}/src/gui/numericformats
      ${CMAKE_SOURCE_DIR}/src/gui/ogr
      ${CMAKE_SOURCE_DIR}/src/gui/pointcloud
      ${CMAKE_SOURCE_DIR}/src/gui/processing
      ${CMAKE_SOURCE_DIR}/src/gui/processing/models
      ${CMAKE_SOURCE_DIR}/src/gui/raster
      ${CMAKE_SOURCE_DIR}/src/gui/settings
      ${CMAKE_SOURCE_DIR}/src/gui/symbology
      ${CMAKE_SOURCE_DIR}/src/gui/tableeditor
      ${CMAKE_SOURCE_DIR}/src/gui/vector
      ${CMAKE_SOURCE_DIR}/src/gui/vectortile
      ${CMAKE_SOURCE_DIR}/src/analysis
      ${CMAKE_SOURCE_DIR}/src/analysis/georeferencing
      ${CMAKE_SOURCE_DIR}/src/analysis/interpolation
      ${CMAKE_SOURCE_DIR}/src/analysis/mesh
      ${CMAKE_SOURCE_DIR}/src/analysis/network
      ${CMAKE_SOURCE_DIR}/src/analysis/processing
      ${CMAKE_SOURCE_DIR}/src/analysis/raster
      ${CMAKE_SOURCE_DIR}/src/analysis/vector
      ${CMAKE_SOURCE_DIR}/src/analysis/vector/geometry_checker
      ${CMAKE_SOURCE_DIR}/src/3d
      ${CMAKE_SOURCE_DIR}/src/3d/chunks
      ${CMAKE_SOURCE_DIR}/src/3d/materials
      ${CMAKE_SOURCE_DIR}/src/3d/processing
      ${CMAKE_SOURCE_DIR}/src/3d/symbols
      ${CMAKE_SOURCE_DIR}/src/3d/terrain
      ${CMAKE_SOURCE_DIR}/src/plugins
      ${CMAKE_SOURCE_DIR}/src/quickgui
      ${CMAKE_SOURCE_DIR}/src/quickgui/plugin
    )

    if(WITH_SERVER)
      set(DOXYGEN_INPUT
        ${DOXYGEN_INPUT}
        ${CMAKE_SOURCE_DIR}/src/server
        ${CMAKE_SOURCE_DIR}/src/server/services
        ${CMAKE_SOURCE_DIR}/src/server/services/wcs
        ${CMAKE_SOURCE_DIR}/src/server/services/wfs
        ${CMAKE_SOURCE_DIR}/src/server/services/wms
      )
    endif()

    set(DOXYGEN_FILE_PATTERNS *.h *.cpp *.dox)

    set(DOXYGEN_FILES)
    foreach(_dir ${DOXYGEN_INPUT})
      foreach(_pattern ${DOXYGEN_FILE_PATTERNS})
        file(GLOB_RECURSE _files ${_dir}/${_pattern})
        list(APPEND DOXYGEN_FILES ${_files})
      endforeach(_pattern)
    endforeach(_dir)

    string(REPLACE ";" " " DOXYGEN_INPUT "${DOXYGEN_INPUT}")

    # Doxyfile requires comma separated list of patterns for FILE_PATTERNS option
    string(REPLACE ";" "," DOXYGEN_FILE_PATTERNS "${DOXYGEN_FILE_PATTERNS}")

    if(WERROR)
      set (DOXYGEN_WARN_AS_ERROR "YES")
    else()
      set (DOXYGEN_WARN_AS_ERROR "NO")
    endif()
    configure_file(${CMAKE_SOURCE_DIR}/cmake_templates/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    set (DOXYGEN_ON_DEMAND FALSE CACHE BOOL "Determines whether the QGIS API doxygen documentation should be built on demand through the target apidoc only. If set to false it is added to the target ALL.")

    if(DOXYGEN_ON_DEMAND)
      add_custom_target(apidoc DEPENDS ${QHP_FILES} version)
      install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/api/html/ DESTINATION ${QGIS_DATA_DIR}/doc/api OPTIONAL)
    else()
      install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/api/html/ DESTINATION ${QGIS_DATA_DIR}/doc/api)
      add_custom_target(apidoc ALL DEPENDS ${QHP_FILES} version)
    endif()
    add_custom_command(
      OUTPUT ${QHP_FILES}
      DEPENDS ${DOXYGEN_FILES}
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation" VERBATIM)
    install(FILES ../images/icons/qgis-icon-60x60.png DESTINATION ${QGIS_DATA_DIR}/doc/api)
    if(GENERATE_QHP)
      install(FILES ${CMAKE_CURRENT_BINARY_DIR}/api/qch/qgis.qch DESTINATION ${QGIS_DATA_DIR}/doc/api)
    endif()
    set(WITH_DOT YES CACHE STRING "Determines if the dot application should be used to generate class diagrams for the documentation")
    mark_as_advanced(WITH_DOT)
  else()
    message(STATUS "QGIS api documentation not built - Doxygen not found")
  endif()
endif()

#SUBDIRS (images plugins install_guide)
