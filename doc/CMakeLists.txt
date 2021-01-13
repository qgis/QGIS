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
      ${CMAKE_SOURCE_DIR}/src/3d
      ${CMAKE_SOURCE_DIR}/src/3d/chunks
      ${CMAKE_SOURCE_DIR}/src/3d/materials
      ${CMAKE_SOURCE_DIR}/src/3d/symbols
      ${CMAKE_SOURCE_DIR}/src/3d/terrain
      ${CMAKE_SOURCE_DIR}/src/plugins
      ${CMAKE_SOURCE_DIR}/src/quickgui
      ${CMAKE_SOURCE_DIR}/src/quickgui/plugin
    )

    if(WITH_SERVER)
      set(DOXYGEN_INPUT
        ${DOXYGEN_INPUT}
        ${CMAKE_SOURCE_DIR}/src/server/
        ${CMAKE_SOURCE_DIR}/src/server/services/
        ${CMAKE_SOURCE_DIR}/src/server/services/wms
        ${CMAKE_SOURCE_DIR}/src/server/services/wfs
        ${CMAKE_SOURCE_DIR}/src/server/services/wcs
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
