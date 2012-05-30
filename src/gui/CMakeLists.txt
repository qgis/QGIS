
SET(QGIS_GUI_SRCS

raster/qgsrasterrendererwidget.cpp
raster/qgsmultibandcolorrendererwidget.cpp
raster/qgspalettedrendererwidget.cpp
raster/qgssinglebandgrayrendererwidget.cpp
raster/qgssinglebandpseudocolorrendererwidget.cpp

symbology-ng/qgsbrushstylecombobox.cpp
symbology-ng/qgscolorrampcombobox.cpp
symbology-ng/qgsdashspacedialog.cpp
symbology-ng/qgspenstylecombobox.cpp
symbology-ng/qgssymbollayerv2widget.cpp
symbology-ng/qgssymbolv2propertiesdialog.cpp
symbology-ng/qgsrendererv2widget.cpp
symbology-ng/qgssinglesymbolrendererv2widget.cpp
symbology-ng/qgscategorizedsymbolrendererv2widget.cpp
symbology-ng/qgsgraduatedsymbolrendererv2widget.cpp
symbology-ng/qgsrulebasedrendererv2widget.cpp
symbology-ng/qgsrendererv2propertiesdialog.cpp
symbology-ng/qgsstylev2managerdialog.cpp
symbology-ng/qgssymbollevelsv2dialog.cpp
symbology-ng/qgssymbolv2selectordialog.cpp
symbology-ng/qgsvectorgradientcolorrampv2dialog.cpp
symbology-ng/qgsvectorrandomcolorrampv2dialog.cpp
symbology-ng/qgsvectorcolorbrewercolorrampv2dialog.cpp
symbology-ng/characterwidget.cpp
symbology-ng/qgsstylev2exportimportdialog.cpp
symbology-ng/qgsellipsesymbollayerv2widget.cpp
symbology-ng/qgspointdisplacementrendererwidget.cpp
symbology-ng/qgsvectorfieldsymbollayerwidget.cpp

attributetable/qgsattributetablemodel.cpp
attributetable/qgsattributetablememorymodel.cpp
attributetable/qgsattributetableview.cpp
attributetable/qgsattributetablefiltermodel.cpp
attributetable/qgsattributetableidcolumnpair.cpp
attributetable/qgsattributetabledelegate.cpp

qgisgui.cpp
qgisinterface.cpp
qgsannotationitem.cpp
qgsattributeeditor.cpp
qgslegendinterface.cpp
qgscolorbutton.cpp
qgscomposerview.cpp
qgscursors.cpp
qgsdetaileditemdelegate.cpp
qgsdetaileditemwidget.cpp
qgsdetaileditemdata.cpp
qgsencodingfiledialog.cpp
qgsfiledropedit.cpp
qgsfieldvalidator.cpp
qgsformannotationitem.cpp
qgsgenericprojectionselector.cpp
qgsmanageconnectionsdialog.cpp
qgsmapcanvas.cpp
qgsmapcanvasitem.cpp
qgsmapcanvasmap.cpp
qgsmapcanvassnapper.cpp
qgsmapoverviewcanvas.cpp
qgsmaptip.cpp
qgsmaptool.cpp
qgsmaptoolemitpoint.cpp
qgsmaptoolpan.cpp
qgsmaptoolzoom.cpp
qgsmessageviewer.cpp
qgsmessagelogviewer.cpp
qgsnewhttpconnection.cpp
qgsnewvectorlayerdialog.cpp
qgsnumericsortlistviewitem.cpp
qgscredentialdialog.cpp
qgsowssourceselect.cpp
qgsprojectbadlayerguihandler.cpp
qgsprojectionselector.cpp
qgsquickprint.cpp
qgsrasterlayersaveasdialog.cpp
qgsrubberband.cpp
qgsscalecombobox.cpp
qgstextannotationitem.cpp
qgsvertexmarker.cpp
qgsludialog.cpp
qgssearchquerybuilder.cpp
qgsexpressionbuilderwidget.cpp
qgsexpressionbuilderdialog.cpp
qgsexpressionhighlighter.cpp
qgsquerybuilder.cpp
)

IF (WITH_TOUCH)
  SET(QGIS_GUI_SRCS
      ${QGIS_GUI_SRCS}
      qgsmaptooltouch.cpp
  )
ENDIF (WITH_TOUCH)

SET(QGIS_GUI_MOC_HDRS

raster/qgspalettedrendererwidget.h
raster/qgsmultibandcolorrendererwidget.h
raster/qgssinglebandgrayrendererwidget.h
raster/qgssinglebandpseudocolorrendererwidget.h

symbology-ng/qgsdashspacedialog.h
symbology-ng/qgssymbollayerv2widget.h
symbology-ng/qgssymbolv2propertiesdialog.h
symbology-ng/qgssinglesymbolrendererv2widget.h
symbology-ng/qgscategorizedsymbolrendererv2widget.h
symbology-ng/qgsgraduatedsymbolrendererv2widget.h
symbology-ng/qgsrulebasedrendererv2widget.h
symbology-ng/qgsrendererv2widget.h
symbology-ng/qgsrendererv2propertiesdialog.h
symbology-ng/qgsstylev2managerdialog.h
symbology-ng/qgssymbollevelsv2dialog.h
symbology-ng/qgssymbolv2selectordialog.h
symbology-ng/qgsvectorgradientcolorrampv2dialog.h
symbology-ng/qgsvectorrandomcolorrampv2dialog.h
symbology-ng/qgsvectorcolorbrewercolorrampv2dialog.h
symbology-ng/qgscolorrampcombobox.h
symbology-ng/characterwidget.h
symbology-ng/qgspenstylecombobox.h
symbology-ng/qgsbrushstylecombobox.h
symbology-ng/qgsstylev2exportimportdialog.h
symbology-ng/qgsellipsesymbollayerv2widget.h
symbology-ng/qgspointdisplacementrendererwidget.h
symbology-ng/qgsvectorfieldsymbollayerwidget.h

attributetable/qgsattributetableview.h
attributetable/qgsattributetablemodel.h
attributetable/qgsattributetablememorymodel.h
attributetable/qgsattributetabledelegate.h

qgsattributeeditor.h
qgscomposerview.h
qgsdetaileditemdelegate.h
qgsdetaileditemwidget.h
qgslegendinterface.h
qgisinterface.h
qgsencodingfiledialog.h
qgsfieldvalidator.h
qgsformannotationitem.h
qgsgenericprojectionselector.h
qgsmanageconnectionsdialog.h
qgsmapcanvas.h
qgsmapoverviewcanvas.h
qgsmaptoolemitpoint.h
qgsmessageviewer.h
qgsmessagelogviewer.h
qgsnewhttpconnection.h
qgsnewvectorlayerdialog.h
qgscredentialdialog.h
qgsowssourceselect.h
qgsprojectionselector.h
qgsquickprint.h
qgsludialog.h
qgsprojectbadlayerguihandler.h
qgslonglongvalidator.h
qgssearchquerybuilder.h
qgsscalecombobox.h
qgsexpressionbuilderwidget.h
qgsexpressionhighlighter.h
qgsquerybuilder.h
)

QT4_WRAP_CPP(QGIS_GUI_MOC_SRCS ${QGIS_GUI_MOC_HDRS})

# for installing headers

SET(QGIS_GUI_HDRS
qgisgui.h
qgisinterface.h
qgscolorbutton.h
qgscursors.h
qgsencodingfiledialog.h
qgsfiledropedit.h
qgsgenericprojectionselector.h
qgsmapcanvas.h
qgsmapcanvasitem.h
qgsmapcanvasmap.h
qgsmapcanvassnapper.h
qgsmapoverviewcanvas.h
qgsmaptool.h
qgsmaptoolemitpoint.h
qgsmaptoolpan.h
qgsmaptoolzoom.h
qgsmessageviewer.h
qgscredentialdialog.h
qgsowssourceselect.h
qgsprojectionselector.h
qgsrubberband.h
qgsvertexmarker.h
qgsmaptip.h
qgsscalecombobox.h
qgssearchquerybuilder.h
qgsattributeeditor.h
qgsfieldvalidator.h
qgsexpressionbuilderwidget.h
qgsexpressionbuilderdialog.h
qgsexpressionhighlighter.h

attributetable/qgsattributetablemodel.h
attributetable/qgsattributetablememorymodel.h
attributetable/qgsattributetableview.h
attributetable/qgsattributetablefiltermodel.h
attributetable/qgsattributetableidcolumnpair.h
attributetable/qgsattributetabledelegate.h
)

IF (WITH_TOUCH)
  SET(QGIS_GUI_HDRS
      ${QGIS_GUI_HDRS}
      qgsmaptooltouch.h
  )
ENDIF (WITH_TOUCH)

SET(QGIS_GUI_UI_HDRS
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdetaileditemwidgetbase.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgenericprojectionselectorbase.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessageviewer.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessagelogviewer.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsowssourceselectbase.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscredentialdialog.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprojectionselectorbase.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsquerybuilderbase.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionbuilder.h
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionbuilderdialogbase.h
)

# ModelTest
#SET(MODELTEST_SRCS
#  ../../tests/qt_modeltest/modeltest.cpp
#  ../../tests/qt_modeltest/dynamictreemodel.cpp
#)
#SET(MODELTEST_MOC_HDRS
#  ../../tests/qt_modeltest/modeltest.h
#  ../../tests/qt_modeltest/dynamictreemodel.h
#)
#QT4_WRAP_CPP(MODELTEST_MOC_SRCS ${MODELTEST_MOC_HDRS})
#INCLUDE_DIRECTORIES(../../tests/qt_modeltest)


INCLUDE_DIRECTORIES(
  ${QT_QTUITOOLS_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/symbology-ng
  ../core
  ../core/composer
  ../core/raster
  ../core/renderer
  ../core/symbology
  ../core/symbology-ng
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
)
IF (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(../core/spatialite/headers)
  INCLUDE_DIRECTORIES(../core/spatialite/headers/spatialite)
ELSE (WITH_INTERNAL_SPATIALITE)
  INCLUDE_DIRECTORIES(${SQLITE3_INCLUDE_DIR})
ENDIF (WITH_INTERNAL_SPATIALITE)

INCLUDE_DIRECTORIES(${GEOS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR})

IF (WIN32)
  IF (MSVC)
    ADD_DEFINITIONS("-DGUI_EXPORT=__declspec(dllexport)")
  ELSE (MSVC)
    ADD_DEFINITIONS("-UGUI_EXPORT \"-DGUI_EXPORT=__declspec(dllexport)\"")
  ENDIF (MSVC)
ENDIF (WIN32)

# disable deprecation warnings for classes re-exporting deprecated methods
IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_BINARY_DIR}/src/gui/moc_qgisinterface.cxx
    ${CMAKE_BINARY_DIR}/src/gui/moc_qgsgenericprojectionselector.cxx
    ${CMAKE_BINARY_DIR}/src/gui/moc_qgsprojectionselector.cxx
    PROPERTIES COMPILE_FLAGS "-wd4996")
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_BINARY_DIR}/src/gui/moc_qgisinterface.cxx
    ${CMAKE_BINARY_DIR}/src/gui/moc_qgsgenericprojectionselector.cxx
    ${CMAKE_BINARY_DIR}/src/gui/moc_qgsprojectionselector.cxx
    PROPERTIES COMPILE_FLAGS "-w")
ENDIF(MSVC)

#############################################################
# qgis_gui library

ADD_LIBRARY(qgis_gui SHARED
     ${QGIS_GUI_SRCS} ${QGIS_GUI_MOC_SRCS} ${QGIS_GUI_HDRS} ${QGIS_GUI_MOC_HDRS}
     #${MODELTEST_SRCS} ${MODELTEST_MOC_SRCS}  # for ModelTest
)

SET_TARGET_PROPERTIES(qgis_gui PROPERTIES
  PUBLIC_HEADER "${QGIS_GUI_HDRS};${QGIS_GUI_MOC_HDRS}"
  CLEAN_DIRECT_OUTPUT 1
  FRAMEWORK 1
  FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
  MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
  MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
  MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis_gui)

#generate unversioned libs for android
IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_gui PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

# make sure that UI files will be processed first
ADD_DEPENDENCIES(qgis_gui ui)

TARGET_LINK_LIBRARIES(qgis_gui
  qgis_core
  ${QT_QTUITOOLS_LIBRARY}
  #${QT_QTTEST_LIBRARY}  # for ModelTest
)

IF (NOT WITH_INTERNAL_SPATIALITE)
  TARGET_LINK_LIBRARIES(qgis_gui ${SQLITE3_LIBRARIES})
ENDIF (NOT WITH_INTERNAL_SPATIALITE)

IF (APPLE)
  SET_TARGET_PROPERTIES(qgis_gui PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
ENDIF (APPLE)

# install

INSTALL(TARGETS qgis_gui
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

# ui headers to install, don't exist at target definition time, install manually
IF (APPLE)
  INSTALL(FILES ${QGIS_GUI_UI_HDRS} DESTINATION ${QGIS_FW_SUBDIR}/qgis_gui.framework/Headers)
ELSE (APPLE)
  INSTALL(FILES ${QGIS_GUI_UI_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ENDIF (APPLE)

# Mac dev frameworks

IF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
  INSTALL(TARGETS qgis_gui FRAMEWORK DESTINATION ${QGIS_MACAPP_DEV_PREFIX})
  INSTALL(FILES ${QGIS_GUI_UI_HDRS} DESTINATION ${QGIS_MACAPP_DEV_PREFIX}/qgis_gui.framework/Headers)
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -id \"${QGIS_MACAPP_DEV_PREFIX}/qgis_gui.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_gui\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_gui.framework/qgis_gui\")")
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND install_name_tool -change \"${CMAKE_INSTALL_NAME_DIR}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"${QGIS_MACAPP_DEV_PREFIX}/qgis_core.framework/Versions/${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}/qgis_core\" \"$ENV{DESTDIR}${QGIS_MACAPP_DEV_PREFIX}/qgis_gui.framework/qgis_gui\")")
ENDIF (APPLE AND QGIS_MACAPP_INSTALL_DEV)
