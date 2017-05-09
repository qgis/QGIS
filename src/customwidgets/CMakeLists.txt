ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)

FIND_PACKAGE(Qt5UiPlugin REQUIRED)
FIND_PACKAGE(Qt5Designer REQUIRED)

SET(QT_USE_QTDESIGNER ON)

########################################################
# Files

SET (QGIS_CUSTOMWIDGETS_SRCS
  qgiscustomwidgets.cpp
  qgscollapsiblegroupboxplugin.cpp
  qgscolorbuttonplugin.cpp
  qgscheckablecomboboxplugin.cpp
  qgsdatetimeeditplugin.cpp
  qgsdockwidgetplugin.cpp
  qgsdoublespinboxplugin.cpp
  qgsexpressionbuilderwidgetplugin.cpp
  qgsextentgroupboxplugin.cpp
  qgsexternalresourcewidgetplugin.cpp
  qgsfieldcomboboxplugin.cpp
  qgsfieldexpressionwidgetplugin.cpp
  qgsfilewidgetplugin.cpp
  qgsfilterlineeditplugin.cpp
  qgsmaplayercomboboxplugin.cpp
  qgspasswordlineeditplugin.cpp
  qgsprojectionselectionwidgetplugin.cpp
  qgspropertyoverridebuttonplugin.cpp
  qgsrasterbandcomboboxplugin.cpp
  qgsrelationeditorwidgetplugin.cpp
  qgsrelationreferencewidgetplugin.cpp
  qgsscalerangewidgetplugin.cpp
  qgsscalewidgetplugin.cpp
  qgsscrollareawidgetplugin.cpp
  qgsspinboxplugin.cpp
)

SET (QGIS_CUSTOMWIDGETS_MOC_HDRS
  qgiscustomwidgets.h
  qgscollapsiblegroupboxplugin.h
  qgscolorbuttonplugin.h
  qgscheckablecomboboxplugin.h
  qgsdatetimeeditplugin.h
  qgsdockwidgetplugin.h
  qgsdoublespinboxplugin.h
  qgsexpressionbuilderwidgetplugin.h
  qgsextentgroupboxplugin.h
  qgsexternalresourcewidgetplugin.h
  qgsfieldcomboboxplugin.h
  qgsfieldexpressionwidgetplugin.h
  qgsfilewidgetplugin.h
  qgsfilterlineeditplugin.h
  qgsmaplayercomboboxplugin.h
  qgspasswordlineeditplugin.h
  qgsprojectionselectionwidgetplugin.h
  qgspropertyoverridebuttonplugin.h
  qgsrasterbandcomboboxplugin.h
  qgsrelationeditorwidgetplugin.h
  qgsrelationreferencewidgetplugin.h
  qgsscalerangewidgetplugin.h
  qgsscalewidgetplugin.h
  qgsscrollareawidgetplugin.h
  qgsspinboxplugin.h
)

IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_CUSTOMWIDGETS_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_CUSTOMWIDGETS_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
ENDIF(MSVC)

SET(QGIS_CUSTOMWIDGETS_HDRS
  qgiscustomwidgets.h
  qgscollapsiblegroupboxplugin.h
  qgscolorbuttonplugin.h
  qgsdatetimeeditplugin.h
  qgsdockwidgetplugin.h
  qgsdoublespinboxplugin.h
  qgsexpressionbuilderwidgetplugin.h
  qgsextentgroupboxplugin.h
  qgsexternalresourcewidgetplugin.h
  qgsfieldcomboboxplugin.h
  qgsfieldexpressionwidgetplugin.h
  qgsfilewidgetplugin.h
  qgsfilterlineeditplugin.h
  qgsmaplayercomboboxplugin.h
  qgsprojectionselectionwidgetplugin.h
  qgspropertyoverridebuttonplugin.h
  qgsrasterbandcomboboxplugin.h
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
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/metadata
  ${CMAKE_CURRENT_SOURCE_DIR}/../core/symbology-ng/
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/attributetable/
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/editorwidgets/
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/editorwidgets/core
  ${CMAKE_CURRENT_SOURCE_DIR}/../gui/raster/
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/core/raster/
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/customwidgets
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/
)
INCLUDE_DIRECTORIES(SYSTEM
  ${GEOS_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
  ${Qt5UiPlugin_INCLUDE_DIRS}
  ${Qt5Designer_INCLUDE_DIRS}
)

QT5_WRAP_CPP(QGIS_CUSTOMWIDGETS_MOC_SRCS ${QGIS_CUSTOMWIDGETS_MOC_HDRS})

#############################################################
# qgis_customwidgets library

SET(IMAGE_RCCS ../../images/images.qrc)
QT5_ADD_RESOURCES(IMAGE_RCC_SRCS ${IMAGE_RCCS})

ADD_LIBRARY(qgis_customwidgets SHARED ${QGIS_CUSTOMWIDGETS_SRCS} ${QGIS_CUSTOMWIDGETS_MOC_SRCS} ${QGIS_CUSTOMWIDGETS_HDRS} ${IMAGE_RCC_SRCS})

SET_TARGET_PROPERTIES(qgis_customwidgets PROPERTIES
  VERSION ${COMPLETE_VERSION}
  SOVERSION ${COMPLETE_VERSION}
  )

# make sure that UI files will be processed first
ADD_DEPENDENCIES(qgis_customwidgets ui)

TARGET_LINK_LIBRARIES(qgis_customwidgets qgis_gui)

GENERATE_EXPORT_HEADER(
   qgis_customwidgets
   BASE_NAME CUSTOMWIDGETS
   EXPORT_FILE_NAME qgis_customwidgets.h
)

SET(QGIS_CUSTOMWIDGETS_HDRS ${QGIS_CUSTOMWIDGETS_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_customwidgets.h)

# install
INSTALL(TARGETS qgis_customwidgets
  RUNTIME DESTINATION ${QT_PLUGINS_DIR}/designer
  LIBRARY DESTINATION ${QT_PLUGINS_DIR}/designer
)

# TODO: apple
INSTALL(FILES ${QGIS_CUSTOMWIDGETS_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
