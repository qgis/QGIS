ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)


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


#############################################################
# qgis_customwidgets library

ADD_LIBRARY(qgis_customwidgets SHARED ${QGIS_CUSTOMWIDGETS_SRCS} ${QGIS_CUSTOMWIDGETS_MOC_SRCS} ${QGIS_CUSTOMWIDGETS_HDRS})

# TODO: apple
INSTALL(FILES ${QGIS_CUSTOMWIDGETS_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})


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


