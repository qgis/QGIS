
SET(QGIS_GUI_SRCS

raster/qgsrasterminmaxwidget.cpp
raster/qgsrasterrendererwidget.cpp
raster/qgsmultibandcolorrendererwidget.cpp
raster/qgspalettedrendererwidget.cpp
raster/qgssinglebandgrayrendererwidget.cpp
raster/qgssinglebandpseudocolorrendererwidget.cpp
raster/qgsrasterhistogramwidget.cpp

symbology-ng/qgsbrushstylecombobox.cpp
symbology-ng/qgscolorrampcombobox.cpp
symbology-ng/qgsdashspacedialog.cpp
symbology-ng/qgsdatadefinedsymboldialog.cpp
symbology-ng/qgspenstylecombobox.cpp
symbology-ng/qgssymbollayerv2widget.cpp
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
symbology-ng/qgscptcitycolorrampv2dialog.cpp
symbology-ng/characterwidget.cpp
symbology-ng/qgsstylev2exportimportdialog.cpp
symbology-ng/qgsellipsesymbollayerv2widget.cpp
symbology-ng/qgspointdisplacementrendererwidget.cpp
symbology-ng/qgsvectorfieldsymbollayerwidget.cpp
symbology-ng/qgssymbolslistwidget.cpp
symbology-ng/qgslayerpropertieswidget.cpp
symbology-ng/qgssmartgroupeditordialog.cpp

attributetable/qgsattributetablemodel.cpp
attributetable/qgsattributetableview.cpp
attributetable/qgsattributetablefiltermodel.cpp
attributetable/qgsattributetabledelegate.cpp
attributetable/qgsfeaturelistview.cpp
attributetable/qgsfeaturelistmodel.cpp
attributetable/qgsfeaturelistviewdelegate.cpp
attributetable/qgsdualview.cpp

qgisgui.cpp
qgisinterface.cpp
qgsannotationitem.cpp
qgsattributeeditor.cpp
qgsattributedialog.cpp
qgsbusyindicatordialog.cpp
qgslegendinterface.cpp
qgsblendmodecombobox.cpp
qgscharacterselectdialog.cpp
qgscolorbutton.cpp
qgscolordialog.cpp
qgscomposerview.cpp
qgscomposerruler.cpp
qgscursors.cpp
qgsdetaileditemdelegate.cpp
qgsdetaileditemwidget.cpp
qgsdetaileditemdata.cpp
qgsdialog.cpp
qgsencodingfiledialog.cpp
qgserrordialog.cpp
qgsfiledropedit.cpp
qgsfieldvalidator.cpp
qgsformannotationitem.cpp
qgshtmlannotationitem.cpp
qgshighlight.cpp
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
qgsmaptoolidentify.cpp
qgsmaptoolpan.cpp
qgsmaptoolzoom.cpp
qgsmessagebar.cpp
qgsmessageviewer.cpp
qgsmessagelogviewer.cpp
qgsnewhttpconnection.cpp
qgsnewvectorlayerdialog.cpp
qgsnumericsortlistviewitem.cpp
qgsoptionsdialogbase.cpp
qgscredentialdialog.cpp
qgsowssourceselect.cpp
qgsprojectbadlayerguihandler.cpp
qgsprojectionselector.cpp
qgsquickprint.cpp
qgsrasterlayersaveasdialog.cpp
qgsrasterformatsaveoptionswidget.cpp
qgsrasterpyramidsoptionswidget.cpp
qgsrubberband.cpp
qgsscalecombobox.cpp
qgssvgannotationitem.cpp
qgstextannotationitem.cpp
qgsvertexmarker.cpp
qgsludialog.cpp
qgssearchquerybuilder.cpp
qgsexpressionbuilderwidget.cpp
qgsexpressionbuilderdialog.cpp
qgsexpressionselectiondialog.cpp
qgsexpressionhighlighter.cpp
qgsquerybuilder.cpp
qgscollapsiblegroupbox.cpp
qgsfilterlineedit.cpp
)

IF (WITH_TOUCH)
  SET(QGIS_GUI_SRCS
      ${QGIS_GUI_SRCS}
      qgsmaptooltouch.cpp
  )
ENDIF (WITH_TOUCH)

SET(QGIS_GUI_MOC_HDRS

qgscolorbutton.h

raster/qgsrasterminmaxwidget.h
raster/qgspalettedrendererwidget.h
raster/qgsmultibandcolorrendererwidget.h
raster/qgssinglebandgrayrendererwidget.h
raster/qgssinglebandpseudocolorrendererwidget.h
raster/qgsrasterhistogramwidget.h

symbology-ng/qgsdashspacedialog.h
symbology-ng/qgssymbollayerv2widget.h
symbology-ng/qgssinglesymbolrendererv2widget.h
symbology-ng/qgscategorizedsymbolrendererv2widget.h
symbology-ng/qgsdatadefinedsymboldialog.h
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
symbology-ng/qgscptcitycolorrampv2dialog.h
symbology-ng/qgscolorrampcombobox.h
symbology-ng/characterwidget.h
symbology-ng/qgspenstylecombobox.h
symbology-ng/qgsbrushstylecombobox.h
symbology-ng/qgsstylev2exportimportdialog.h
symbology-ng/qgsellipsesymbollayerv2widget.h
symbology-ng/qgspointdisplacementrendererwidget.h
symbology-ng/qgsvectorfieldsymbollayerwidget.h
symbology-ng/qgssymbolslistwidget.h
symbology-ng/qgslayerpropertieswidget.h
symbology-ng/qgssmartgroupeditordialog.h

attributetable/qgsattributetableview.h
attributetable/qgsattributetablemodel.h
attributetable/qgsattributetablefiltermodel.h
attributetable/qgsattributetabledelegate.h
attributetable/qgsfeaturelistview.h
attributetable/qgsfeaturelistmodel.h
attributetable/qgsfeaturelistviewdelegate.h
attributetable/qgsdualview.h

qgsattributedialog.h
qgsattributeeditor.h
qgsblendmodecombobox.h
qgsbusyindicatordialog.h
qgscharacterselectdialog.h
qgscolordialog.h
qgscomposerview.h
qgsdetaileditemdelegate.h
qgsdetaileditemwidget.h
qgsdialog.h
qgslegendinterface.h
qgisinterface.h
qgsencodingfiledialog.h
qgserrordialog.h
qgsfieldvalidator.h
qgsformannotationitem.h
qgshtmlannotationitem.h
qgsgenericprojectionselector.h
qgsmanageconnectionsdialog.h
qgsmapcanvas.h
qgsmapoverviewcanvas.h
qgsmaptoolemitpoint.h
qgsmaptoolidentify.h
qgsmessagebar.h
qgsmessageviewer.h
qgsmessagelogviewer.h
qgsnewhttpconnection.h
qgsnewvectorlayerdialog.h
qgsoptionsdialogbase.h
qgscredentialdialog.h
qgsowssourceselect.h
qgsprojectionselector.h
qgsquickprint.h
qgsrasterlayersaveasdialog.h
qgsrasterformatsaveoptionswidget.h
qgsrasterpyramidsoptionswidget.h
qgsludialog.h
qgsprojectbadlayerguihandler.h
qgslonglongvalidator.h
qgssearchquerybuilder.h
qgsscalecombobox.h
qgsexpressionbuilderwidget.h
qgsexpressionhighlighter.h
qgsexpressionselectiondialog.h
qgsquerybuilder.h
qgscollapsiblegroupbox.h
qgsfilterlineedit.h
)

QT4_WRAP_CPP(QGIS_GUI_MOC_SRCS ${QGIS_GUI_MOC_HDRS})

# for installing headers

SET(QGIS_GUI_HDRS
qgisgui.h
qgisinterface.h
qgsbusyindicatordialog.h
qgscharacterselectdialog.h
qgscolordialog.h
qgscursors.h
qgsencodingfiledialog.h
qgsfiledropedit.h
qgsgenericprojectionselector.h
qgshighlight.h
qgsmapcanvas.h
qgsmapcanvasitem.h
qgsmapcanvasmap.h
qgsmapcanvassnapper.h
qgsmapoverviewcanvas.h
qgsmaptool.h
qgsmaptoolemitpoint.h
qgsmaptoolidentify.h
qgsmaptoolpan.h
qgsmaptoolzoom.h
qgsmessagebar.h
qgsmessageviewer.h
qgsoptionsdialogbase.h
qgscredentialdialog.h
qgsowssourceselect.h
qgsprojectionselector.h
qgsrubberband.h
qgsvertexmarker.h
qgsmaptip.h
qgsscalecombobox.h
qgsblendmodecombobox.h
qgssearchquerybuilder.h
qgsattributeeditor.h
qgsattributedialog.h
qgsfieldvalidator.h
qgsexpressionbuilderwidget.h
qgsexpressionbuilderdialog.h
qgsexpressionselectiondialog.h
qgsexpressionhighlighter.h
qgscollapsiblegroupbox.h
qgsfilterlineedit.h

attributetable/qgsattributetablemodel.h
attributetable/qgsattributetableview.h
attributetable/qgsattributetablefiltermodel.h
attributetable/qgsattributetabledelegate.h
attributetable/qgsfeaturelistview.h
attributetable/qgsfeaturelistmodel.h
attributetable/qgsfeaturelistviewdelegate.h
attributetable/qgsdualview.h
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
${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionselectiondialogbase.h
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
  ${CMAKE_CURRENT_SOURCE_DIR}/attributetable
  ../core
  ../core/composer
  ../core/raster
  ../core/renderer
  ../core/symbology
  ../core/symbology-ng
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
  ${QWT_INCLUDE_DIR}
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

IF(NOT APPLE)
  INSTALL(FILES ${QGIS_GUI_HDRS} ${QGIS_GUI_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ELSE(NOT APPLE)
  SET_TARGET_PROPERTIES(qgis_gui PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis2_gui
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_GUI_HDRS};${QGIS_GUI_MOC_HDRS}"
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
  )
ENDIF(NOT APPLE)

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
  ${QWT_LIBRARY}
  #${QT_QTTEST_LIBRARY}  # for ModelTest
)

IF(WIN32)
  ADD_DEFINITIONS(-DQWT_DLL)
ENDIF(WIN32)

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
