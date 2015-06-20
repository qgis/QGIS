ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

IF (QT5_BUILD)
  FIND_PACKAGE(Qt5Designer REQUIRED)
ENDIF (QT5_BUILD)

SET(QT_USE_QTDESIGNER ON)


IF (NOT MSVC)
  ADD_DEFINITIONS("-UCUSTOMWIDGETS_EXPORT")
  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  ENDIF()
ENDIF (NOT MSVC)
ADD_DEFINITIONS("\"-DCUSTOMWIDGETS_EXPORT=${DLLEXPORT}\"")

########################################################
# Files

SET (QGIS_CUSTOMWIDGETS_SRCS
  qgiscustomwidgets.cpp
  qgscollapsiblegroupboxplugin.cpp
  qgscolorbuttonplugin.cpp
  qgscolorbuttonv2plugin.cpp
  qgsdatetimeeditplugin.cpp
  qgsdatadefinedbuttonplugin.cpp
  qgsdoublespinboxplugin.cpp
  qgsfieldcomboboxplugin.cpp
  qgsfieldexpressionwidgetplugin.cpp
  qgsfilterlineeditplugin.cpp
  qgsmaplayercomboboxplugin.cpp
  qgsprojectionselectionwidgetplugin.cpp
  qgsrelationeditorwidgetplugin.cpp
  qgsrelationreferencewidgetplugin.cpp
  qgsscalerangewidgetplugin.cpp
  qgsscalewidgetplugin.cpp
  qgsspinboxplugin.cpp
)

SET (QGIS_CUSTOMWIDGETS_MOC_HDRS
  qgiscustomwidgets.h
  qgscollapsiblegroupboxplugin.h
  qgscolorbuttonplugin.h
  qgscolorbuttonv2plugin.h
  qgsdatetimeeditplugin.h
  qgsdatadefinedbuttonplugin.h
  qgsdoublespinboxplugin.h
  qgsfieldcomboboxplugin.h
  qgsfieldexpressionwidgetplugin.h
  qgsfilterlineeditplugin.h
  qgsmaplayercomboboxplugin.h
  qgsprojectionselectionwidgetplugin.h
  qgsrelationeditorwidgetplugin.h
  qgsrelationreferencewidgetplugin.h
  qgsscalerangewidgetplugin.h
  qgsscalewidgetplugin.h
  qgsspinboxplugin.h
)

IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_CUSTOMWIDGETS_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4996" )
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_CUSTOMWIDGETS_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
ENDIF(MSVC)

SET(QGIS_CUSTOMWIDGETS_HDRS
  qgiscustomwidgets.h
  qgscollapsiblegroupboxplugin.h
  qgscolorbuttonplugin.h
  qgscolorbuttonv2plugin.h
  qgsdatetimeeditplugin.h
  qgsdatadefinedbuttonplugin.h
  qgsdoublespinboxplugin.h
  qgsfieldcomboboxplugin.h
  qgsfieldexpressionwidgetplugin.h
  qgsfilterlineeditplugin.h
  qgsmaplayercomboboxplugin.h
  qgsprojectionselectionwidgetplugin.h
  qgsrelationeditorwidgetplugin.h
  qgsrelationreferencewidgetplugin.h
  qgsscalerangewidgetplugin.h
  qgsscalewidgetplugin.h
  qgsspinboxplugin.h
)

# left commented as there is no UI file yet
# SET(QGIS_CUSTOMWIDGETS_UI_HDRS
#   ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsXXXwidget.h
# )

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/geometry
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/symbology-ng/
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/attributetable/
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/editorwidgets/
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/
  ${GEOS_INCLUDE_DIR}
  ${Qt5Designer_INCLUDE_DIRS} # For Qt5
  ${QT_QTDESIGNER_INCLUDE_DIR} # For Qt4
)

QT4_WRAP_CPP(QGIS_CUSTOMWIDGETS_MOC_SRCS ${QGIS_CUSTOMWIDGETS_MOC_HDRS})

#############################################################
# qgis_customwidgets library

SET(IMAGE_RCCS ../../images/images.qrc)
QT4_ADD_RESOURCES(IMAGE_RCC_SRCS ${IMAGE_RCCS})

ADD_LIBRARY(qgis_customwidgets SHARED ${QGIS_CUSTOMWIDGETS_SRCS} ${QGIS_CUSTOMWIDGETS_MOC_SRCS} ${QGIS_CUSTOMWIDGETS_HDRS} ${IMAGE_RCC_SRCS})

# TODO: apple
INSTALL(FILES ${QGIS_CUSTOMWIDGETS_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})

SET_TARGET_PROPERTIES(qgis_customwidgets PROPERTIES
  VERSION ${COMPLETE_VERSION}
  SOVERSION ${COMPLETE_VERSION}
  )

# make sure that UI files will be processed first
ADD_DEPENDENCIES(qgis_customwidgets ui)

TARGET_LINK_LIBRARIES(qgis_customwidgets qgis_gui)

# install
INSTALL(TARGETS qgis_customwidgets
  RUNTIME DESTINATION ${QT_PLUGINS_DIR}/designer
  LIBRARY DESTINATION ${QT_PLUGINS_DIR}/designer
)
