
########################################################
# Files

SET (BROWSER_SRCS
     main.cpp
     qgsbrowser.cpp
)

SET (BROWSER_UIS qgsbrowserbase.ui)

SET (BROWSER_MOC_HDRS
     qgsbrowser.h
)

#SET (BROWSER_RCCS  qgsgps_plugin.qrc)

IF (WIN32)
  IF (MSVC)
    SET (BROWSER_SRCS ${BROWSER_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/../app/qgis_win32.rc)
  ELSE(MSVC)

    # Icon for windows MINGW
    # Note: can't include .rc directly to source files
    # as it's ignored when used MinGW

    IF (NOT WINDRES)
      FIND_PROGRAM(WINDRES windres)
      IF (NOT WINDRES)
        MESSAGE(FATAL_ERROR "windres not found - aborting")
      ENDIF (NOT WINDRES)
    ENDIF (NOT WINDRES)
    #############################################################
    # application icon
    # resource compilation for MinGW
    ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o
      COMMAND ${WINDRES} -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/../app/qgis_win32.rc
      -o ${CMAKE_CURRENT_BINARY_DIR}/icon.o )
    SET(QGIS_APP_SRCS ${QGIS_APP_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
  ENDIF (MSVC)
ENDIF (WIN32)

SET(IMAGE_RCCS ../../images/images.qrc)

########################################################
# Build

QT4_WRAP_UI (BROWSER_UIS_H  ${BROWSER_UIS})

QT4_WRAP_CPP (BROWSER_MOC_SRCS  ${BROWSER_MOC_HDRS})

QT4_ADD_RESOURCES(IMAGE_RCC_SRCS ${IMAGE_RCCS})
#QT4_ADD_RESOURCES(BROWSER_RCC_SRCS ${BROWSER_RCCS})

ADD_EXECUTABLE (qbrowser MACOSX_BUNDLE WIN32 ${BROWSER_SRCS} ${BROWSER_MOC_SRCS} ${BROWSER_UIS_H} ${IMAGE_RCC_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../core
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/raster
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/attributetable
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/editorwidgets
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/editorwidgets/core
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
  ${CMAKE_CURRENT_BINARY_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR} # remove once raster layer is cleaned up
)

INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})

TARGET_LINK_LIBRARIES(qbrowser
  qgis_core
  qgis_gui
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTSVG_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}
  ${QT_QTMAIN_LIBRARY}
  ${SQLITE3_LIBRARY}
)

IF(APPLE)
  SET_TARGET_PROPERTIES(qbrowser PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH true
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/browser.info.plist.in"
    OUTPUT_NAME "QGIS Browser"
  )
ENDIF(APPLE)

########################################################
# Install

INSTALL (TARGETS qbrowser
  BUNDLE DESTINATION ${QGIS_BIN_DIR}
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
)
IF (APPLE)
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfh ../../../${QGIS_FW_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/QGIS Browser.app/Contents/Frameworks\")")
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfh ../../../../${QGIS_LIB_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/QGIS Browser.app/Contents/MacOS/lib\")")
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfh ../../../${QGIS_PLUGIN_SUBDIR}/.. \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/QGIS Browser.app/Contents/Plugins\")")
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND ln -sfh ../../../${QGIS_DATA_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/QGIS Browser.app/Contents/Resources\")")
ENDIF (APPLE)
