
########################################################
# Files

SET (BROWSER_SRCS
     main.cpp
     qgsbrowser.cpp
     qgsbrowsermodel.cpp
)

SET (BROWSER_UIS qgsbrowserbase.ui)

SET (BROWSER_MOC_HDRS
     qgsbrowser.h
     qgsbrowsermodel.h
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

ADD_EXECUTABLE (qbrowser MACOSX_BUNDLE ${BROWSER_SRCS} ${BROWSER_MOC_SRCS} ${BROWSER_UIS_H} ${IMAGE_RCC_SRCS})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/../core
     ${CMAKE_CURRENT_SOURCE_DIR}/../core/raster
     ${CMAKE_CURRENT_SOURCE_DIR}/../gui
     ${CMAKE_CURRENT_SOURCE_DIR}/../gui/attributetable
     ${CMAKE_CURRENT_BINARY_DIR}/../ui
     ${CMAKE_CURRENT_BINARY_DIR}
     ${GDAL_INCLUDE_DIR} # remove once raster layer is cleaned up
)

IF (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(../core/spatialite/headers/spatialite)
ELSE (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
ENDIF (WITH_INTERNAL_SPATIALITE)

TARGET_LINK_LIBRARIES(qbrowser qgis_core qgis_gui)

IF (NOT WITH_INTERNAL_SPATIALITE)
  TARGET_LINK_LIBRARIES(qbrowser ${SQLITE3_LIBRARY})
ENDIF (NOT WITH_INTERNAL_SPATIALITE)

IF (${QTVERSION} STRLESS "4.3.0")
  TARGET_LINK_LIBRARIES(qbrowser
    ${QT_LIBRARIES} 
    )
ELSE (${QTVERSION} STRLESS "4.3.0")
  TARGET_LINK_LIBRARIES(qbrowser
    ${QT_QTCORE_LIBRARY} 
    ${QT_QTGUI_LIBRARY} 
    ${QT_QTNETWORK_LIBRARY} 
    ${QT_QTSVG_LIBRARY} 
    ${QT_QTXML_LIBRARY} 
    ${QT_QTWEBKIT_LIBRARY} 
    )
ENDIF (${QTVERSION} STRLESS "4.3.0")

SET_TARGET_PROPERTIES(qbrowser PROPERTIES
  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true
)


########################################################
# Install

IF (APPLE)
  INSTALL (TARGETS qbrowser BUNDLE DESTINATION ${QGIS_BIN_DIR})
  # needed because global install_name prefix is for main qgis app
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND install_name_tool -change ${CMAKE_INSTALL_NAME_DIR}/libqgis_core.${COMPLETE_VERSION}.dylib @executable_path/../../../../lib/libqgis_core.${COMPLETE_VERSION}.dylib \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qbrowser.app/Contents/MacOS/qbrowser\")")
  INSTALL (CODE "EXECUTE_PROCESS (COMMAND ln -sfh ../../../${QGIS_FW_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qbrowser.app/Contents/Frameworks\")")
ELSE (APPLE)
  INSTALL (TARGETS qbrowser RUNTIME DESTINATION ${QGIS_BIN_DIR})
ENDIF (APPLE)
