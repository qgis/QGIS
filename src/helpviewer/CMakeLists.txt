
########################################################
# Files

SET (HELP_SRCS
     main.cpp
     qgshelpserver.cpp
     qgshelpviewer.cpp
     #../core/qgsapplication.cpp
     #../core/qgslogger.cpp
)

SET (HELP_UIS qgshelpviewerbase.ui)

SET (HELP_MOC_HDRS
     qgshelpserver.h
     qgshelpviewer.h
)

#SET (HELP_RCCS  qgsgps_plugin.qrc)

IF (WIN32)
  IF (MSVC)
    SET (HELP_SRCS ${HELP_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/../app/qgis_win32.rc)
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


########################################################
# Build

QT4_WRAP_UI (HELP_UIS_H  ${HELP_UIS})

QT4_WRAP_CPP (HELP_MOC_SRCS  ${HELP_MOC_HDRS})

#QT4_ADD_RESOURCES(HELP_RCC_SRCS ${HELP_RCCS})

IF (BUILD_MACAPP_BUNDLE)
  ADD_EXECUTABLE (qgis_help MACOSX_BUNDLE ${HELP_SRCS} ${HELP_MOC_SRCS} ${HELP_UIS_H})
ELSE(BUILD_MACAPP_BUNDLE)
  ADD_EXECUTABLE (qgis_help ${HELP_SRCS} ${HELP_MOC_SRCS} ${HELP_UIS_H})
ENDIF(BUILD_MACAPP_BUNDLE)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
     ${CMAKE_CURRENT_SOURCE_DIR}/../core
     ${CMAKE_CURRENT_BINARY_DIR}
)

IF (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(../core/spatialite/headers/spatialite)
ELSE (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
ENDIF (WITH_INTERNAL_SPATIALITE)

TARGET_LINK_LIBRARIES(qgis_help qgis_core)

IF (NOT WITH_INTERNAL_SPATIALITE)
  TARGET_LINK_LIBRARIES(qgis_help ${SQLITE3_LIBRARY})
ENDIF (NOT WITH_INTERNAL_SPATIALITE)

IF (${QTVERSION} STRLESS "4.3.0")
  TARGET_LINK_LIBRARIES(qgis_help
    ${QT_LIBRARIES} 
    )
ELSE (${QTVERSION} STRLESS "4.3.0")
  TARGET_LINK_LIBRARIES(qgis_help
    ${QT_QTCORE_LIBRARY} 
    ${QT_QTGUI_LIBRARY} 
    ${QT_QTNETWORK_LIBRARY} 
    ${QT_QTSVG_LIBRARY} 
    ${QT_QTXML_LIBRARY} 
    ${QT_QTWEBKIT_LIBRARY} 
    )
ENDIF (${QTVERSION} STRLESS "4.3.0")

SET_TARGET_PROPERTIES(qgis_help PROPERTIES
  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  INSTALL_RPATH_USE_LINK_PATH true
)


########################################################
# Install

IF (APPLE AND BUILD_MACAPP_BUNDLE)
  INSTALL (TARGETS qgis_help BUNDLE DESTINATION ${QGIS_BIN_DIR})
  # needed because global install_name prefix is for main qgis app
  INSTALL (CODE "EXECUTE_PROCESS(COMMAND install_name_tool -change ${CMAKE_INSTALL_NAME_DIR}/libqgis_core.${COMPLETE_VERSION}.dylib @executable_path/../../../../lib/libqgis_core.${COMPLETE_VERSION}.dylib \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_help.app/Contents/MacOS/qgis_help\")")
  INSTALL (CODE "EXECUTE_PROCESS (COMMAND ln -sfh ../../../${QGIS_FW_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_help.app/Contents/Frameworks\")")
ELSE (APPLE AND BUILD_MACAPP_BUNDLE)
  INSTALL (TARGETS qgis_help RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR})
ENDIF (APPLE AND BUILD_MACAPP_BUNDLE)
