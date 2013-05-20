# override default path where built files are put to allow running qgis without installing
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIBEXEC_SUBDIR})

########################################################
# Files

SET (HELP_SRCS
     main.cpp
     qgshelpviewer.cpp
)

SET (HELP_UIS qgshelpviewerbase.ui)

SET (HELP_MOC_HDRS qgshelpviewer.h)

SET (HELP_RCCS ../../images/images.qrc)

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

QT4_ADD_RESOURCES(HELP_RCC_SRCS ${HELP_RCCS})

ADD_EXECUTABLE (qgis_help MACOSX_BUNDLE WIN32 ${HELP_SRCS} ${HELP_MOC_SRCS} ${HELP_UIS_H} ${HELP_RCC_SRCS})

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

TARGET_LINK_LIBRARIES(qgis_help
  ${QT_QTCORE_LIBRARY}
  ${QT_QTGUI_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${QT_QTSVG_LIBRARY}
  ${QT_QTXML_LIBRARY}
  ${QT_QTWEBKIT_LIBRARY}
  ${QT_QTMAIN_LIBRARY}
)

IF(APPLE)
  SET_TARGET_PROPERTIES(qgis_help PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH true
  )
ENDIF(APPLE)


########################################################
# Install

INSTALL (TARGETS qgis_help
  BUNDLE DESTINATION ${QGIS_BIN_DIR}
  RUNTIME DESTINATION ${QGIS_LIBEXEC_DIR}
)
IF (APPLE)
  INSTALL (CODE "EXECUTE_PROCESS (COMMAND ln -sfh ../../../${QGIS_FW_SUBDIR} \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/${QGIS_BIN_DIR}/qgis_help.app/Contents/Frameworks\")")
ENDIF (APPLE)
