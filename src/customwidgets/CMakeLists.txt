
########################################################
# Files

SET (QGIS_CUSTOMWIDGETS_SRCS
   qgiscustomwidgets.cpp
   qgscollapsiblegroupboxplugin.cpp
   qgsfieldcomboboxplugin.cpp
   qgsfieldexpressionwidgetplugin.cpp
   qgsmaplayercomboboxplugin.cpp
   qgsscalevisibilitywidgetplugin.cpp
)

SET (QGIS_CUSTOMWIDGETS_MOC_HDRS
   qgiscustomwidgets.h
   qgscollapsiblegroupboxplugin.h
   qgsfieldcomboboxplugin.h
   qgsfieldexpressionwidgetplugin.h
   qgsmaplayercomboboxplugin.h
   qgsscalevisibilitywidgetplugin.h
)

QT4_WRAP_CPP(QGIS_CUSTOMWIDGETS_MOC_SRCS ${QGIS_CUSTOMWIDGETS_MOC_HDRS})

IF(UNIX)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_CUSTOMWIDGETS_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
ENDIF(UNIX)

SET(QGIS_CUSTOMWIDGETS_HDRS
  qgiscustomwidgets.h
  qgscollapsiblegroupboxplugin.h
  qgsfieldcomboboxplugin.h
  qgsfieldexpressionwidgetplugin.h
  qgsmaplayercomboboxplugin.h
  qgsscalevisibilitywidgetplugin.h
)

SET(QGIS_CUSTOMWIDGETS_UI_HDRS
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsscalevisibilitywidget.h
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
)

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DCUSTOMWIDGETS_EXPORT=${DLLEXPORT}")
  ELSE (MSVC)
    ADD_DEFINITIONS("-UCUSTOMWIDGETS_EXPORT \"-DCUSTOMWIDGETS_EXPORT=${DLLEXPORT}\"")
  ENDIF (MSVC)
ENDIF (WIN32)


#############################################################
# qgis_customwidgets library

ADD_LIBRARY(qgis_customwidgets SHARED ${QGIS_CUSTOMWIDGETS_SRCS} ${QGIS_CUSTOMWIDGETS_MOC_SRCS} ${QGIS_CUSTOMWIDGETS_HDRS})

IF(NOT APPLE)
  INSTALL(FILES ${QGIS_CUSTOMWIDGETS_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ELSE(NOT APPLE)
  SET_TARGET_PROPERTIES(qgis_customwidgets PROPERTIES
    # no moc headers, messes up PROPERTIES syntax
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis2_customwidgets
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_CUSTOMWIDGETS_HDRS}"
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
  )
ENDIF(NOT APPLE)

#generate unversioned libs for android
IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_customwidgets PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

# make sure that UI files will be processed first
ADD_DEPENDENCIES(qgis_customwidgets ui)

TARGET_LINK_LIBRARIES(qgis_customwidgets qgis_gui)


# install

INSTALL(TARGETS qgis_customwidgets
  LIBRARY DESTINATION ${QGIS_CUSTOMWIDGETS_DIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

# ui headers to install, don't exist at target definition time, install manually
IF (APPLE)
  INSTALL(FILES ${QGIS_CUSTOMWIDGETS_UI_HDRS} DESTINATION ${QGIS_FW_SUBDIR}/qgis_gui.framework/Headers)
ELSE (APPLE)
  INSTALL(FILES ${QGIS_CUSTOMWIDGETS_UI_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ENDIF (APPLE)

# Mac dev frameworks
#IF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
#  INSTALL(TARGETS qgis_customwidgets FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
#  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_customwidgets.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_customwidgets\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_customwidgets.framework/qgis_customwidgets\")")
#  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -change \"${CMAKE_INSTALL_NAME_DIR}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_customwidgets.framework/qgis_customwidgets\")")
#ENDIF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
