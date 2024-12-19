add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_SHARED)

find_package(${QT_VERSION_BASE}UiPlugin REQUIRED)
find_package(${QT_VERSION_BASE}Designer REQUIRED)

set(QT_USE_QTDESIGNER ON)

########################################################
# Files

set (QGIS_CUSTOMWIDGETS_SRCS
  qgiscustomwidgets.cpp
  qgsauthconfigselectplugin.cpp
  qgscollapsiblegroupboxplugin.cpp
  qgscolorbuttonplugin.cpp
  qgscheckablecomboboxplugin.cpp
  qgsdatetimeeditplugin.cpp
  qgsdockwidgetplugin.cpp
  qgsdoublespinboxplugin.cpp
  qgsexpressionbuilderwidgetplugin.cpp
  qgsextentgroupboxplugin.cpp
  qgsexternalresourcewidgetplugin.cpp
  qgsfeaturelistcomboboxplugin.cpp
  qgsfeaturepickerwidgetplugin.cpp
  qgsfieldcomboboxplugin.cpp
  qgsfieldexpressionwidgetplugin.cpp
  qgsfilewidgetplugin.cpp
  qgsfilterlineeditplugin.cpp
  qgsfontbuttonplugin.cpp
  qgsmaplayercomboboxplugin.cpp
  qgsopacitywidgetplugin.cpp
  qgspasswordlineeditplugin.cpp
  qgsprojectionselectionwidgetplugin.cpp
  qgspropertyoverridebuttonplugin.cpp
  qgsrasterbandcomboboxplugin.cpp
  qgsrelationeditorwidgetplugin.cpp
  qgsrelationreferencewidgetplugin.cpp
  qgsscalerangewidgetplugin.cpp
  qgsscalewidgetplugin.cpp
#  qgsscrollareawidgetplugin.cpp
  qgsspinboxplugin.cpp
  qgssymbolbuttonplugin.cpp
)

set (QGIS_CUSTOMWIDGETS_MOC_HDRS
  qgiscustomwidgets.h
  qgsauthconfigselectplugin.h
  qgscollapsiblegroupboxplugin.h
  qgscolorbuttonplugin.h
  qgscheckablecomboboxplugin.h
  qgsdatetimeeditplugin.h
  qgsdockwidgetplugin.h
  qgsdoublespinboxplugin.h
  qgsexpressionbuilderwidgetplugin.h
  qgsextentgroupboxplugin.h
  qgsexternalresourcewidgetplugin.h
  qgsfeaturelistcomboboxplugin.h
  qgsfeaturepickerwidgetplugin.h
  qgsfieldcomboboxplugin.h
  qgsfieldexpressionwidgetplugin.h
  qgsfilewidgetplugin.h
  qgsfilterlineeditplugin.h
  qgsfontbuttonplugin.h
  qgsmaplayercomboboxplugin.h
  qgsopacitywidgetplugin.h
  qgspasswordlineeditplugin.h
  qgsprojectionselectionwidgetplugin.h
  qgspropertyoverridebuttonplugin.h
  qgsrasterbandcomboboxplugin.h
  qgsrelationeditorwidgetplugin.h
  qgsrelationreferencewidgetplugin.h
  qgsscalerangewidgetplugin.h
  qgsscalewidgetplugin.h
#  qgsscrollareawidgetplugin.h
  qgsspinboxplugin.h
  qgssymbolbuttonplugin.h
)

if(MSVC)
  set_source_files_properties(${QGIS_CUSTOMWIDGETS_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
else()
  set_source_files_properties(${QGIS_CUSTOMWIDGETS_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
endif()


# left commented as there is no UI file yet
# set(QGIS_CUSTOMWIDGETS_UI_HDRS
#   ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsXXXwidget.h
# )

include_directories(
  ${CMAKE_BINARY_DIR}/src/customwidgets
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
)
include_directories(SYSTEM
  ${${QT_VERSION_BASE}UiPlugin_INCLUDE_DIRS}
  ${${QT_VERSION_BASE}Designer_INCLUDE_DIRS}
)

if (BUILD_WITH_QT6)
  QT6_WRAP_CPP(QGIS_CUSTOMWIDGETS_MOC_SRCS ${QGIS_CUSTOMWIDGETS_MOC_HDRS})
else()
  QT5_WRAP_CPP(QGIS_CUSTOMWIDGETS_MOC_SRCS ${QGIS_CUSTOMWIDGETS_MOC_HDRS})
endif()

#############################################################
# qgis_customwidgets library

add_library(qgis_customwidgets SHARED ${QGIS_CUSTOMWIDGETS_SRCS} ${QGIS_CUSTOMWIDGETS_MOC_SRCS} ${QGIS_CUSTOMWIDGETS_MOC_HDRS})

# require c++17
target_compile_features(qgis_customwidgets PRIVATE cxx_std_17)

set_target_properties(qgis_customwidgets PROPERTIES
  VERSION ${COMPLETE_VERSION}
  SOVERSION ${COMPLETE_VERSION}
  )

# make sure that UI files will be processed first
add_dependencies(qgis_customwidgets ui)

target_link_libraries(qgis_customwidgets qgis_gui)

GENERATE_EXPORT_HEADER(
   qgis_customwidgets
   BASE_NAME CUSTOMWIDGETS
   EXPORT_FILE_NAME qgis_customwidgets.h
)

set(QGIS_CUSTOMWIDGETS_INSTALL_HDRS ${QGIS_CUSTOMWIDGETS_MOC_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_customwidgets.h)

# install
install(TARGETS qgis_customwidgets
  RUNTIME DESTINATION ${QT_PLUGINS_DIR}/designer
  LIBRARY DESTINATION ${QT_PLUGINS_DIR}/designer
)

# TODO: apple
install(FILES ${QGIS_CUSTOMWIDGETS_INSTALL_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
