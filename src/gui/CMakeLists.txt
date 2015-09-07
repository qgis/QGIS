SET(QGIS_GUI_SRCS
  raster/qgsrasterminmaxwidget.cpp
  raster/qgsrasterrendererwidget.cpp
  raster/qgsmultibandcolorrendererwidget.cpp
  raster/qgspalettedrendererwidget.cpp
  raster/qgssinglebandgrayrendererwidget.cpp
  raster/qgssinglebandpseudocolorrendererwidget.cpp
  raster/qgsrasterhistogramwidget.cpp
  raster/qwt5_histogram_item.cpp

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
  symbology-ng/qgsgraduatedhistogramwidget.cpp
  symbology-ng/qgsrulebasedrendererv2widget.cpp
  symbology-ng/qgsheatmaprendererwidget.cpp
  symbology-ng/qgsinvertedpolygonrendererwidget.cpp
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
  symbology-ng/qgssvgselectorwidget.cpp
  symbology-ng/qgslayerpropertieswidget.cpp
  symbology-ng/qgssmartgroupeditordialog.cpp
  symbology-ng/qgssizescalewidget.cpp

  effects/qgseffectdrawmodecombobox.cpp
  effects/qgspainteffectpropertieswidget.cpp
  effects/qgspainteffectwidget.cpp
  effects/qgseffectstackpropertieswidget.cpp

  attributetable/qgsattributetabledelegate.cpp
  attributetable/qgsattributetablefiltermodel.cpp
  attributetable/qgsfieldconditionalformatwidget.cpp
  attributetable/qgsattributetablemodel.cpp
  attributetable/qgsattributetableview.cpp
  attributetable/qgsdualview.cpp
  attributetable/qgsfeaturelistmodel.cpp
  attributetable/qgsfeaturelistview.cpp
  attributetable/qgsfeaturelistviewdelegate.cpp
  attributetable/qgsfeatureselectionmodel.cpp
  attributetable/qgsgenericfeatureselectionmanager.cpp
  attributetable/qgsvectorlayerselectionmanager.cpp

  editorwidgets/core/qgseditorconfigwidget.cpp
  editorwidgets/core/qgseditorwidgetfactory.cpp
  editorwidgets/core/qgseditorwidgetregistry.cpp
  editorwidgets/core/qgseditorwidgetwrapper.cpp
  editorwidgets/core/qgssearchwidgetwrapper.cpp
  editorwidgets/core/qgswidgetwrapper.cpp

  editorwidgets/qgscheckboxconfigdlg.cpp
  editorwidgets/qgscheckboxwidgetwrapper.cpp
  editorwidgets/qgscheckboxwidgetfactory.cpp
  editorwidgets/qgsclassificationwidgetwrapper.cpp
  editorwidgets/qgsclassificationwidgetwrapperfactory.cpp
  editorwidgets/qgscolorwidgetwrapper.cpp
  editorwidgets/qgscolorwidgetfactory.cpp
  editorwidgets/qgsdatetimeedit.cpp
  editorwidgets/qgsdatetimeeditfactory.cpp
  editorwidgets/qgsdatetimeeditconfig.cpp
  editorwidgets/qgsdatetimeeditwrapper.cpp
  editorwidgets/qgsdefaultsearchwidgetwrapper.cpp
  editorwidgets/qgsdoublespinbox.cpp
  editorwidgets/qgsdummyconfigdlg.cpp
  editorwidgets/qgsenumerationwidgetwrapper.cpp
  editorwidgets/qgsenumerationwidgetfactory.cpp
  editorwidgets/qgsfilenamewidgetwrapper.cpp
  editorwidgets/qgsfilenamewidgetfactory.cpp
  editorwidgets/qgshiddenwidgetwrapper.cpp
  editorwidgets/qgshiddenwidgetfactory.cpp
  editorwidgets/qgsphotoconfigdlg.cpp
  editorwidgets/qgsphotowidgetwrapper.cpp
  editorwidgets/qgsphotowidgetfactory.cpp
  editorwidgets/qgsrangeconfigdlg.cpp
  editorwidgets/qgsrangewidgetwrapper.cpp
  editorwidgets/qgsrangewidgetfactory.cpp
  editorwidgets/qgsspinbox.cpp
  editorwidgets/qgsrelationwidgetwrapper.cpp
  editorwidgets/qgsrelationreferenceconfigdlg.cpp
  editorwidgets/qgsrelationreferencefactory.cpp
  editorwidgets/qgsrelationreferencewidget.cpp
  editorwidgets/qgsrelationreferencewidgetwrapper.cpp
  editorwidgets/qgstexteditconfigdlg.cpp
  editorwidgets/qgstexteditwrapper.cpp
  editorwidgets/qgstexteditwidgetfactory.cpp
  editorwidgets/qgsuniquevaluesconfigdlg.cpp
  editorwidgets/qgsuniquevaluewidgetwrapper.cpp
  editorwidgets/qgsuniquevaluewidgetfactory.cpp
  editorwidgets/qgsuuidwidgetwrapper.cpp
  editorwidgets/qgsuuidwidgetfactory.cpp
  editorwidgets/qgsvaluemapconfigdlg.cpp
  editorwidgets/qgsvaluemapwidgetwrapper.cpp
  editorwidgets/qgsvaluemapwidgetfactory.cpp
  editorwidgets/qgsvaluemapsearchwidgetwrapper.cpp
  editorwidgets/qgsvaluerelationconfigdlg.cpp
  editorwidgets/qgsvaluerelationwidgetwrapper.cpp
  editorwidgets/qgsvaluerelationsearchwidgetwrapper.cpp
  editorwidgets/qgsvaluerelationwidgetfactory.cpp


  layertree/qgscustomlayerorderwidget.cpp
  layertree/qgslayertreemapcanvasbridge.cpp
  layertree/qgslayertreeview.cpp
  layertree/qgslayertreeviewdefaultactions.cpp

  qgisgui.cpp
  qgisinterface.cpp
  qgsannotationitem.cpp
  qgsactionmenu.cpp
  qgsattributedialog.cpp
  qgsattributeeditor.cpp
  qgsattributeform.cpp
  qgsattributeforminterface.cpp
  qgsattributeformlegacyinterface.cpp
  qgsattributetypeloaddialog.cpp
  qgsblendmodecombobox.cpp
  qgsbusyindicatordialog.cpp
  qgscharacterselectdialog.cpp
  qgsbrowsertreeview.cpp
  qgscollapsiblegroupbox.cpp
  qgscolorbutton.cpp
  qgscolorbuttonv2.cpp
  qgscolordialog.cpp
  qgscolorswatchgrid.cpp
  qgscolorschemelist.cpp
  qgscolorwidgets.cpp
  qgscodeeditor.cpp
  qgscodeeditorpython.cpp
  qgscodeeditorsql.cpp
  qgscodeeditorhtml.cpp
  qgscodeeditorcss.cpp
  qgscomposerruler.cpp
  qgscomposerview.cpp
  qgsprevieweffect.cpp
  qgscredentialdialog.cpp
  qgscursors.cpp
  qgsdatadefinedbutton.cpp
  qgsdetaileditemdata.cpp
  qgsdatumtransformdialog.cpp
  qgsdetaileditemdelegate.cpp
  qgsdetaileditemwidget.cpp
  qgsdial.cpp
  qgsdialog.cpp
  qgsencodingfiledialog.cpp
  qgserrordialog.cpp
  qgsexpressionbuilderdialog.cpp
  qgsexpressionbuilderwidget.cpp
  qgsexpressionhighlighter.cpp
  qgsexpressionselectiondialog.cpp
  qgsextentgroupbox.cpp
  qgsfeatureselectiondlg.cpp
  qgsfieldcombobox.cpp
  qgsfieldexpressionwidget.cpp
  qgsfieldmodel.cpp
  qgsfieldvalidator.cpp
  qgsfieldproxymodel.cpp
  qgsfiledropedit.cpp
  qgsfilterlineedit.cpp
  qgsformannotationitem.cpp
  qgsgenericprojectionselector.cpp
  qgsgeometryrubberband.cpp
  qgshighlight.cpp
  qgshistogramwidget.cpp
  qgsidentifymenu.cpp
  qgshtmlannotationitem.cpp
  qgslegendinterface.cpp
  qgsludialog.cpp
  qgsmanageconnectionsdialog.cpp
  qgsmapcanvas.cpp
  qgsmapcanvasitem.cpp
  qgsmapcanvasmap.cpp
  qgsmapcanvassnapper.cpp
  qgsmapcanvassnappingutils.cpp
  qgsmaplayeractionregistry.cpp
  qgsmaplayercombobox.cpp
  qgsmaplayermodel.cpp
  qgsmaplayerproxymodel.cpp
  qgsmapoverviewcanvas.cpp
  qgsmaptip.cpp
  qgsmaptool.cpp
  qgsmaptoolemitpoint.cpp
  qgsmaptoolidentify.cpp
  qgsmaptoolidentifyfeature.cpp
  qgsmaptoolpan.cpp
  qgsmaptoolzoom.cpp
  qgsmessagebar.cpp
  qgsmessagebaritem.cpp
  qgsmessagelogviewer.cpp
  qgsmessageviewer.cpp
  qgsnewhttpconnection.cpp
  qgsnewmemorylayerdialog.cpp
  qgsnewnamedialog.cpp
  qgsnewvectorlayerdialog.cpp
  qgsnumericsortlistviewitem.cpp
  qgsoptionsdialogbase.cpp
  qgsowssourceselect.cpp
  qgspixmaplabel.cpp
  qgspluginmanagerinterface.cpp
  qgsprojectbadlayerguihandler.cpp
  qgsprojectionselectionwidget.cpp
  qgsprojectionselector.cpp
  qgsquerybuilder.cpp
  qgsrasterformatsaveoptionswidget.cpp
  qgsrasterlayersaveasdialog.cpp
  qgsrasterpyramidsoptionswidget.cpp
  qgsrelationadddlg.cpp
  qgsrelationeditorwidget.cpp
  qgsrelationmanagerdialog.cpp
  qgsrubberband.cpp
  qgsscalecombobox.cpp
  qgsscalerangewidget.cpp
  qgsscalevisibilitydialog.cpp
  qgsscalewidget.cpp
  qgssearchquerybuilder.cpp
  qgsslider.cpp
  qgssublayersdialog.cpp
  qgssvgannotationitem.cpp
  qgstextannotationitem.cpp
  qgsuserinputdockwidget.cpp
  qgsvariableeditorwidget.cpp
  qgsvertexmarker.cpp
  qgsunitselectionwidget.cpp
)

IF (WITH_QTWEBKIT)
  SET(QGIS_GUI_SRCS
      ${QGIS_GUI_SRCS}
      editorwidgets/qgswebviewconfigdlg.cpp
      editorwidgets/qgswebviewwidgetwrapper.cpp
      editorwidgets/qgswebviewwidgetfactory.cpp
  )
ENDIF (WITH_QTWEBKIT)


IF (WITH_TOUCH)
  SET(QGIS_GUI_SRCS
      ${QGIS_GUI_SRCS}
      qgsmaptooltouch.cpp
  )
ENDIF (WITH_TOUCH)

SET(QGIS_GUI_MOC_HDRS
  qgisinterface.h
  qgsactionmenu.h
  qgsattributedialog.h
  qgsattributeeditor.h
  qgsattributeform.h
  qgsattributetypeloaddialog.h
  qgsblendmodecombobox.h
  qgsbrowsertreeview.h
  qgsbusyindicatordialog.h
  qgscharacterselectdialog.h
  qgscodeeditorcss.h
  qgscodeeditor.h
  qgscodeeditorhtml.h
  qgscodeeditorpython.h
  qgscodeeditorsql.h
  qgscollapsiblegroupbox.h
  qgscolorbutton.h
  qgscolorbuttonv2.h
  qgscolordialog.h
  qgscolorschemelist.h
  qgscolorswatchgrid.h
  qgscolorwidgets.h
  qgscomposerruler.h
  qgscomposerview.h
  qgscredentialdialog.h
  qgsdatadefinedbutton.h
  qgsdatumtransformdialog.h
  qgsdetaileditemdelegate.h
  qgsdetaileditemwidget.h
  qgsdial.h
  qgsdialog.h
  qgsencodingfiledialog.h
  qgserrordialog.h
  qgsexpressionbuilderwidget.h
  qgsexpressionhighlighter.h
  qgsexpressionselectiondialog.h
  qgsextentgroupbox.h
  qgsfeatureselectiondlg.h
  qgsfieldcombobox.h
  qgsfieldexpressionwidget.h
  qgsfieldmodel.h
  qgsfieldproxymodel.h
  qgsfieldvalidator.h
  qgsfilterlineedit.h
  qgsformannotationitem.h
  qgsgenericprojectionselector.h
  qgshistogramwidget.h
  qgshtmlannotationitem.h
  qgsidentifymenu.h
  qgslegendinterface.h
  qgslonglongvalidator.h
  qgsludialog.h
  qgsmanageconnectionsdialog.h
  qgsmapcanvas.h
  qgsmapcanvassnappingutils.h
  qgsmaplayeractionregistry.h
  qgsmaplayercombobox.h
  qgsmaplayermodel.h
  qgsmaplayerproxymodel.h
  qgsmapoverviewcanvas.h
  qgsmaptoolemitpoint.h
  qgsmaptool.h
  qgsmaptoolidentifyfeature.h
  qgsmaptoolidentify.h
  qgsmaptoolpan.h
  qgsmaptoolzoom.h
  qgsmessagebar.h
  qgsmessagebaritem.h
  qgsmessagelogviewer.h
  qgsmessageviewer.h
  qgsnewhttpconnection.h
  qgsnewmemorylayerdialog.h
  qgsnewnamedialog.h
  qgsnewvectorlayerdialog.h
  qgsoptionsdialogbase.h
  qgsowssourceselect.h
  qgspixmaplabel.h
  qgspluginmanagerinterface.h
  qgsprevieweffect.h
  qgsprojectbadlayerguihandler.h
  qgsprojectionselectionwidget.h
  qgsprojectionselector.h
  qgsquerybuilder.h
  qgsrasterformatsaveoptionswidget.h
  qgsrasterlayersaveasdialog.h
  qgsrasterpyramidsoptionswidget.h
  qgsrelationadddlg.h
  qgsrelationeditorwidget.h
  qgsrelationmanagerdialog.h
  qgsscalecombobox.h
  qgsscalerangewidget.h
  qgsscalevisibilitydialog.h
  qgsscalewidget.h
  qgssearchquerybuilder.h
  qgsslider.h
  qgssublayersdialog.h
  qgsvariableeditorwidget.h
  qgsunitselectionwidget.h
  qgsuserinputdockwidget.h

  raster/qgsrasterminmaxwidget.h
  raster/qgspalettedrendererwidget.h
  raster/qgsmultibandcolorrendererwidget.h
  raster/qgssinglebandgrayrendererwidget.h
  raster/qgssinglebandpseudocolorrendererwidget.h
  raster/qgsrasterhistogramwidget.h
  raster/qgsrasterrendererwidget.h

  symbology-ng/characterwidget.h
  symbology-ng/qgsbrushstylecombobox.h
  symbology-ng/qgscategorizedsymbolrendererv2widget.h
  symbology-ng/qgscolorrampcombobox.h
  symbology-ng/qgscptcitycolorrampv2dialog.h
  symbology-ng/qgsdashspacedialog.h
  symbology-ng/qgsdatadefinedsymboldialog.h
  symbology-ng/qgsellipsesymbollayerv2widget.h
  symbology-ng/qgsgraduatedsymbolrendererv2widget.h
  symbology-ng/qgsgraduatedhistogramwidget.h
  symbology-ng/qgsheatmaprendererwidget.h
  symbology-ng/qgsinvertedpolygonrendererwidget.h
  symbology-ng/qgslayerpropertieswidget.h
  symbology-ng/qgspenstylecombobox.h
  symbology-ng/qgspointdisplacementrendererwidget.h
  symbology-ng/qgsrendererv2propertiesdialog.h
  symbology-ng/qgsrendererv2widget.h
  symbology-ng/qgsrulebasedrendererv2widget.h
  symbology-ng/qgssinglesymbolrendererv2widget.h
  symbology-ng/qgssmartgroupeditordialog.h
  symbology-ng/qgsstylev2exportimportdialog.h
  symbology-ng/qgsstylev2managerdialog.h
  symbology-ng/qgssvgselectorwidget.h
  symbology-ng/qgssymbollayerv2widget.h
  symbology-ng/qgssymbollevelsv2dialog.h
  symbology-ng/qgssymbolslistwidget.h
  symbology-ng/qgssymbolv2selectordialog.h
  symbology-ng/qgsvectorcolorbrewercolorrampv2dialog.h
  symbology-ng/qgsvectorfieldsymbollayerwidget.h
  symbology-ng/qgsvectorgradientcolorrampv2dialog.h
  symbology-ng/qgsvectorrandomcolorrampv2dialog.h
  symbology-ng/qgssizescalewidget.h

  attributetable/qgsattributetabledelegate.h
  attributetable/qgsattributetablefiltermodel.h
  attributetable/qgsattributetablemodel.h
  attributetable/qgsattributetableview.h
  attributetable/qgsdualview.h
  attributetable/qgsfieldconditionalformatwidget.h
  attributetable/qgsfeaturelistmodel.h
  attributetable/qgsfeaturelistview.h
  attributetable/qgsfeaturelistviewdelegate.h
  attributetable/qgsfeatureselectionmodel.h
  attributetable/qgsgenericfeatureselectionmanager.h
  attributetable/qgsifeatureselectionmanager.h
  attributetable/qgsvectorlayerselectionmanager.h

  effects/qgseffectdrawmodecombobox.h
  effects/qgspainteffectpropertieswidget.h
  effects/qgspainteffectwidget.h
  effects/qgseffectstackpropertieswidget.h

  editorwidgets/core/qgseditorconfigwidget.h
  editorwidgets/core/qgseditorwidgetregistry.h
  editorwidgets/core/qgseditorconfigwidget.h
  editorwidgets/core/qgseditorwidgetregistry.h
  editorwidgets/core/qgseditorwidgetwrapper.h
  editorwidgets/core/qgssearchwidgetwrapper.h
  editorwidgets/core/qgswidgetwrapper.h

  editorwidgets/qgscheckboxconfigdlg.h
  editorwidgets/qgscheckboxwidgetwrapper.h
  editorwidgets/qgsclassificationwidgetwrapper.h
  editorwidgets/qgscolorwidgetwrapper.h
  editorwidgets/qgsdatetimeedit.h
  editorwidgets/qgsdatetimeeditconfig.h
  editorwidgets/qgsdatetimeeditwrapper.h
  editorwidgets/qgsdefaultsearchwidgetwrapper.h
  editorwidgets/qgsdoublespinbox.h
  editorwidgets/qgsdummyconfigdlg.h
  editorwidgets/qgsenumerationwidgetwrapper.h
  editorwidgets/qgsfilenamewidgetwrapper.h
  editorwidgets/qgshiddenwidgetwrapper.h
  editorwidgets/qgsphotoconfigdlg.h
  editorwidgets/qgsphotowidgetwrapper.h
  editorwidgets/qgsrangeconfigdlg.h
  editorwidgets/qgsrangewidgetwrapper.h
  editorwidgets/qgsrelationreferenceconfigdlg.h
  editorwidgets/qgsrelationreferencewidget.h
  editorwidgets/qgsrelationreferencewidgetwrapper.h
  editorwidgets/qgsrelationwidgetwrapper.h
  editorwidgets/qgsspinbox.h
  editorwidgets/qgstexteditconfigdlg.h
  editorwidgets/qgstexteditwrapper.h
  editorwidgets/qgsuniquevaluesconfigdlg.h
  editorwidgets/qgsuniquevaluewidgetwrapper.h
  editorwidgets/qgsuuidwidgetwrapper.h
  editorwidgets/qgsvaluemapconfigdlg.h
  editorwidgets/qgsvaluemapwidgetwrapper.h
  editorwidgets/qgsvaluemapsearchwidgetwrapper.h
  editorwidgets/qgsvaluerelationconfigdlg.h
  editorwidgets/qgsvaluerelationwidgetwrapper.h
  editorwidgets/qgsvaluerelationsearchwidgetwrapper.h

  layertree/qgscustomlayerorderwidget.h
  layertree/qgslayertreemapcanvasbridge.h
  layertree/qgslayertreeview.h
  layertree/qgslayertreeviewdefaultactions.h
)

IF (WITH_QTWEBKIT)
  SET(QGIS_GUI_MOC_HDRS
      ${QGIS_GUI_MOC_HDRS}
      editorwidgets/qgswebviewconfigdlg.h
      editorwidgets/qgswebviewwidgetwrapper.h
  )
ENDIF (WITH_QTWEBKIT)

QT4_WRAP_CPP(QGIS_GUI_MOC_SRCS ${QGIS_GUI_MOC_HDRS})

IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_GUI_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4996" )
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_GUI_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
ENDIF(MSVC)

# for installing headers

SET(QGIS_GUI_HDRS
  qgisgui.h
  qgsannotationitem.h
  qgsattributeeditorcontext.h
  qgsattributeforminterface.h
  qgsattributeformlegacyinterface.h
  qgscursors.h
  qgsdetaileditemdata.h
  qgsexpressionbuilderdialog.h
  qgsfiledropedit.h
  qgshighlight.h
  qgsmapcanvasitem.h
  qgsmapcanvasmap.h
  qgsmapcanvassnapper.h
  qgsmapcanvassnappingutils.h
  qgsmaptip.h
  qgsmaptoolpan.h
  qgsmaptoolzoom.h
  qgsnumericsortlistviewitem.h
  qgsrubberband.h
  qgssvgannotationitem.h
  qgstextannotationitem.h
  qgsuserinputdockwidget.h
  qgsvectorlayertools.h
  qgsvertexmarker.h

  attributetable/qgsfeaturemodel.h

  editorwidgets/core/qgseditorwidgetfactory.h

  editorwidgets/qgscheckboxwidgetfactory.h
  editorwidgets/qgsclassificationwidgetwrapperfactory.h
  editorwidgets/qgscolorwidgetfactory.h
  editorwidgets/qgsdatetimeeditfactory.h
  editorwidgets/qgsenumerationwidgetfactory.h
  editorwidgets/qgsfilenamewidgetfactory.h
  editorwidgets/qgshiddenwidgetfactory.h
  editorwidgets/qgsphotowidgetfactory.h
  editorwidgets/qgsrangewidgetfactory.h
  editorwidgets/qgsrelationreferencefactory.h
  editorwidgets/qgstexteditwidgetfactory.h
  editorwidgets/qgsuniquevaluewidgetfactory.h
  editorwidgets/qgsuuidwidgetfactory.h
  editorwidgets/qgsvaluemapwidgetfactory.h
  editorwidgets/qgsvaluerelationwidgetfactory.h

  raster/qgsrasterrendererwidget.h
)

IF (WITH_QTWEBKIT)
  SET(QGIS_GUI_HDRS
      ${QGIS_GUI_HDRS}
      editorwidgets/qgswebviewwidgetfactory.h
  )
ENDIF (WITH_QTWEBKIT)

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
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssublayersdialogbase.h
)

IF(ENABLE_MODELTEST)
  INCLUDE_DIRECTORIES(../../tests/qt_modeltest)
ENDIF(ENABLE_MODELTEST)

INCLUDE_DIRECTORIES(
  ${QT_QTUITOOLS_INCLUDE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/symbology-ng
  ${CMAKE_CURRENT_SOURCE_DIR}/attributetable
  ${CMAKE_CURRENT_SOURCE_DIR}/editorwidgets
  ${CMAKE_CURRENT_SOURCE_DIR}/editorwidgets/core
  ${CMAKE_CURRENT_SOURCE_DIR}/layertree
  ${CMAKE_CURRENT_SOURCE_DIR}/effects
  ../core
  ../core/composer
  ../core/geometry
  ../core/layertree
  ../core/raster
  ../core/symbology-ng
  ../core/effects
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
  ${QWT_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
)

INCLUDE_DIRECTORIES(${QSCINTILLA_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${GEOS_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR})

IF (NOT MSVC)
  ADD_DEFINITIONS("-UGUI_EXPORT")
  IF ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
  ENDIF()
ENDIF (NOT MSVC)
ADD_DEFINITIONS("\"-DGUI_EXPORT=${DLLEXPORT}\"")

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
    PROPERTIES COMPILE_FLAGS "-w -Wno-deprecated-declarations")
ENDIF(MSVC)

#############################################################
# qgis_gui library

ADD_LIBRARY(qgis_gui SHARED
     ${QGIS_GUI_SRCS} ${QGIS_GUI_MOC_SRCS} ${QGIS_GUI_HDRS} ${QGIS_GUI_MOC_HDRS}
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
  ${QSCINTILLA_LIBRARY}
)

IF(WIN32)
  ADD_DEFINITIONS(-DQWT_DLL)
  ADD_DEFINITIONS(-DQSCINTILLA_DLL)
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
