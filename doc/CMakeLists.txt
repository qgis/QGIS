# include doxygen documentation
SET (WITH_APIDOC FALSE CACHE BOOL "Determines whether the QGIS API doxygen documentation should be built")

# include doxygen documentation
SET (WITH_TXT2TAGS_PDF FALSE CACHE BOOL "Determines whether PDF should be generate for the txt2tags documentation")

# include Qt documentation
SET (QT_DOC_URL "http://doc.qt.io/qt-5/" CACHE STRING "URL for Qt docs")

INCLUDE(Txt2Tags)
FIND_TXT2TAGS()

IF(TXT2TAGS_EXECUTABLE)
  ADD_TXT2TAGS_FILES(QGIS_DOC_FILES INSTALL.t2t news.t2t)
  IF(WITH_TXT2TAGS_PDF)
    ADD_TXT2TAGS_PDFS(QGIS_DOC_FILES INSTALL.t2t news.t2t)
  ENDIF(WITH_TXT2TAGS_PDF)
  ADD_CUSTOM_TARGET (t2tdoc ALL DEPENDS ${QGIS_DOC_FILES})
ELSE(TXT2TAGS_EXECUTABLE)
  SET(QGIS_DOC_FILES
    ${CMAKE_SOURCE_DIR}/INSTALL
  )
ENDIF(TXT2TAGS_EXECUTABLE)

SET(QGIS_DOC_FILES ${QGIS_DOC_FILES} index.html news.html developersmap.html nohelp.html contributors.json favicon.ico style.css AUTHORS CONTRIBUTORS SPONSORS DONORS TRANSLATORS LICENSE)

INSTALL(FILES ${QGIS_DOC_FILES} DESTINATION ${QGIS_DATA_DIR}/doc)
INSTALL(FILES ../images/icons/qgis-icon-60x60.png DESTINATION ${QGIS_DATA_DIR}/doc/images)

# Sponsor images
FILE (GLOB SPONSOR_IMAGES images/projects/thumbnails/* )
INSTALL (FILES ${SPONSOR_IMAGES} DESTINATION ${QGIS_DATA_DIR}/doc/images/projects/thumbnails/)

IF(WITH_APIDOC)
  FIND_PACKAGE(Doxygen 1.8.2 REQUIRED)
  IF(DOXYGEN_FOUND)
    SET(QHP_FILES ${CMAKE_CURRENT_BINARY_DIR}/api/html/index.qhp)

    # Whether to generate QHP file
    SET (GENERATE_QHP FALSE CACHE BOOL "Determines whether to generate Qt QHP help from the QGIS API Docs")
    IF (GENERATE_QHP)
      SET(WITH_QHP YES)
      SET(QHP_FILES ${QHP_FILES} ${CMAKE_CURRENT_BINARY_DIR}/api/qch/qgis.qch)
    ELSE(GENERATE_QHP)
      SET(WITH_QHP NO)
    ENDIF(GENERATE_QHP)

    SET(DOXYGEN_INCLUDE_PATH
      ${CMAKE_SOURCE_DIR}/src/core
    )
    STRING(REPLACE ";" " " DOXYGEN_INCLUDE_PATH "${DOXYGEN_INCLUDE_PATH}")

    SET(DOXYGEN_INPUT
      ${CMAKE_SOURCE_DIR}/doc
      ${CMAKE_SOURCE_DIR}/src/core
      ${CMAKE_SOURCE_DIR}/src/core/annotations
      ${CMAKE_SOURCE_DIR}/src/core/auth
      ${CMAKE_SOURCE_DIR}/src/core/composer
      ${CMAKE_SOURCE_DIR}/src/core/diagram
      ${CMAKE_SOURCE_DIR}/src/core/dxf
      ${CMAKE_SOURCE_DIR}/src/core/effects
      ${CMAKE_SOURCE_DIR}/src/core/fieldformatter
      ${CMAKE_SOURCE_DIR}/src/core/geometry
      ${CMAKE_SOURCE_DIR}/src/core/geocms
      ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
      ${CMAKE_SOURCE_DIR}/src/core/gps
      ${CMAKE_SOURCE_DIR}/src/core/layertree
      ${CMAKE_SOURCE_DIR}/src/core/layout
      ${CMAKE_SOURCE_DIR}/src/core/metadata
      ${CMAKE_SOURCE_DIR}/src/core/pal
      ${CMAKE_SOURCE_DIR}/src/core/processing
      ${CMAKE_SOURCE_DIR}/src/core/providers
      ${CMAKE_SOURCE_DIR}/src/core/providers/memory
      ${CMAKE_SOURCE_DIR}/src/core/raster
      ${CMAKE_SOURCE_DIR}/src/core/scalebar
      ${CMAKE_SOURCE_DIR}/src/core/symbology
      ${CMAKE_SOURCE_DIR}/src/gui
      ${CMAKE_SOURCE_DIR}/src/gui/auth
      ${CMAKE_SOURCE_DIR}/src/gui/attributetable
      ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
      ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets/core
      ${CMAKE_SOURCE_DIR}/src/gui/effects
      ${CMAKE_SOURCE_DIR}/src/gui/layertree
      ${CMAKE_SOURCE_DIR}/src/gui/locator
      ${CMAKE_SOURCE_DIR}/src/gui/raster
      ${CMAKE_SOURCE_DIR}/src/gui/symbology
      ${CMAKE_SOURCE_DIR}/src/analysis
      ${CMAKE_SOURCE_DIR}/src/analysis/interpolation
      ${CMAKE_SOURCE_DIR}/src/analysis/network
      ${CMAKE_SOURCE_DIR}/src/analysis/openstreetmap
      ${CMAKE_SOURCE_DIR}/src/analysis/raster
      ${CMAKE_SOURCE_DIR}/src/analysis/vector
      ${CMAKE_SOURCE_DIR}/src/plugins
    )

    IF(WITH_SERVER_PLUGINS)
      SET(DOXYGEN_INPUT
        ${DOXYGEN_INPUT}
        ${CMAKE_SOURCE_DIR}/src/server/qgsserver.h
        ${CMAKE_SOURCE_DIR}/src/server/qgscapabilitiescache.h
        ${CMAKE_SOURCE_DIR}/src/server/qgsserverexception.h
        ${CMAKE_SOURCE_DIR}/src/server/qgsrequesthandler.h
        ${CMAKE_SOURCE_DIR}/src/server/qgsserverfilter.h
        ${CMAKE_SOURCE_DIR}/src/server/qgsaccesscontrolfilter.h
        ${CMAKE_SOURCE_DIR}/src/server/qgsserverinterface.h
        ${CMAKE_SOURCE_DIR}/src/server/qgsserverrequest.h
        ${CMAKE_SOURCE_DIR}/src/server/qgsserverresponse.h
        ${CMAKE_SOURCE_DIR}/src/server/qgsserverrequest.h
        ${CMAKE_SOURCE_DIR}/src/server/qgsservice.h
        ${CMAKE_SOURCE_DIR}/src/server/qgsserviceregistry.h
      )
    ENDIF(WITH_SERVER_PLUGINS)

    SET(DOXYGEN_FILE_PATTERNS *.h *.cpp *.dox)

    SET(DOXYGEN_FILES)
    FOREACH(_dir ${DOXYGEN_INPUT})
      FOREACH(_pattern ${DOXYGEN_FILE_PATTERNS})
        FILE(GLOB_RECURSE _files ${_dir}/${_pattern})
        LIST(APPEND DOXYGEN_FILES ${_files})
      ENDFOREACH(_pattern)
    ENDFOREACH(_dir)

    STRING(REPLACE ";" " " DOXYGEN_INPUT "${DOXYGEN_INPUT}")

    CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake_templates/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    SET (DOXYGEN_ON_DEMAND FALSE CACHE BOOL "Determines whether the QGIS API doxygen documentation should be build on demand only")

    IF(DOXYGEN_ON_DEMAND)
      ADD_CUSTOM_TARGET(apidoc DEPENDS ${QHP_FILES})
    ELSE(DOXYGEN_ON_DEMAND)
      ADD_CUSTOM_TARGET(apidoc ALL DEPENDS ${QHP_FILES})
    ENDIF(DOXYGEN_ON_DEMAND)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${QHP_FILES}
      DEPENDS ${DOXYGEN_FILES}
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation" VERBATIM)
    INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/api/html/ DESTINATION ${QGIS_DATA_DIR}/doc/api)
    INSTALL(FILES ../images/icons/qgis-icon-60x60.png DESTINATION ${QGIS_DATA_DIR}/doc/api)
    IF(GENERATE_QHP)
      INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/api/qch/qgis.qch DESTINATION ${QGIS_DATA_DIR}/doc/api)
    ENDIF(GENERATE_QHP)
  ELSE(DOXYGEN_FOUND)
    MESSAGE(STATUS "QGIS api documentation not built - Doxygen not found")
  ENDIF(DOXYGEN_FOUND)
ENDIF(WITH_APIDOC)

#SUBDIRS (images plugins install_guide)
