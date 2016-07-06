SET(QGIS_GUI_SRCS
  raster/qgsmultibandcolorrendererwidget.cpp
  raster/qgspalettedrendererwidget.cpp
  raster/qgsrasterhistogramwidget.cpp
  raster/qgsrasterminmaxwidget.cpp
  raster/qgsrasterrendererwidget.cpp
  raster/qgssinglebandgrayrendererwidget.cpp
  raster/qgssinglebandpseudocolorrendererwidget.cpp
  raster/qgsrendererrasterpropertieswidget.cpp
  raster/qgsrastertransparencywidget.cpp
  raster/qgshillshaderendererwidget.cpp
  raster/qwt5_histogram_item.cpp

  symbology-ng/qgs25drendererwidget.cpp
  symbology-ng/characterwidget.cpp
  symbology-ng/qgsarrowsymbollayerwidget.cpp
  symbology-ng/qgsbrushstylecombobox.cpp
  symbology-ng/qgscategorizedsymbolrendererv2widget.cpp
  symbology-ng/qgscolorrampcombobox.cpp
  symbology-ng/qgscptcitycolorrampv2dialog.cpp
  symbology-ng/qgsdashspacedialog.cpp
  symbology-ng/qgsdatadefinedsymboldialog.cpp
  symbology-ng/qgsellipsesymbollayerv2widget.cpp
  symbology-ng/qgsgraduatedhistogramwidget.cpp
  symbology-ng/qgsgraduatedsymbolrendererv2widget.cpp
  symbology-ng/qgsheatmaprendererwidget.cpp
  symbology-ng/qgsinvertedpolygonrendererwidget.cpp
  symbology-ng/qgslayerpropertieswidget.cpp
  symbology-ng/qgsnullsymbolrendererwidget.cpp
  symbology-ng/qgspenstylecombobox.cpp
  symbology-ng/qgspointdisplacementrendererwidget.cpp
  symbology-ng/qgsrendererv2propertiesdialog.cpp
  symbology-ng/qgsrendererv2widget.cpp
  symbology-ng/qgsrulebasedrendererv2widget.cpp
  symbology-ng/qgssinglesymbolrendererv2widget.cpp
  symbology-ng/qgssizescalewidget.cpp
  symbology-ng/qgssmartgroupeditordialog.cpp
  symbology-ng/qgsstylev2exportimportdialog.cpp
  symbology-ng/qgsstylev2groupselectiondialog.cpp
  symbology-ng/qgsstylev2managerdialog.cpp
  symbology-ng/qgssvgselectorwidget.cpp
  symbology-ng/qgssymbollayerv2widget.cpp
  symbology-ng/qgssymbollevelsv2dialog.cpp
  symbology-ng/qgssymbolslistwidget.cpp
  symbology-ng/qgssymbolv2selectordialog.cpp
  symbology-ng/qgsvectorcolorbrewercolorrampv2dialog.cpp
  symbology-ng/qgsvectorfieldsymbollayerwidget.cpp
  symbology-ng/qgsvectorgradientcolorrampv2dialog.cpp
  symbology-ng/qgsvectorrandomcolorrampv2dialog.cpp

  effects/qgseffectdrawmodecombobox.cpp
  effects/qgseffectstackpropertieswidget.cpp
  effects/qgspainteffectpropertieswidget.cpp
  effects/qgspainteffectwidget.cpp

  attributetable/qgsattributetabledelegate.cpp
  attributetable/qgsattributetablefiltermodel.cpp
  attributetable/qgsattributetablemodel.cpp
  attributetable/qgsattributetableview.cpp
  attributetable/qgsdualview.cpp
  attributetable/qgsfeaturelistmodel.cpp
  attributetable/qgsfeaturelistview.cpp
  attributetable/qgsfeaturelistviewdelegate.cpp
  attributetable/qgsfeatureselectionmodel.cpp
  attributetable/qgsfieldconditionalformatwidget.cpp
  attributetable/qgsgenericfeatureselectionmanager.cpp
  attributetable/qgsvectorlayerselectionmanager.cpp
  attributetable/qgsorganizetablecolumnsdialog.cpp

  auth/qgsauthauthoritieseditor.cpp
  auth/qgsauthcertificateinfo.cpp
  auth/qgsauthcertificatemanager.cpp
  auth/qgsauthcerttrustpolicycombobox.cpp
  auth/qgsauthconfigedit.cpp
  auth/qgsauthconfigeditor.cpp
  auth/qgsauthconfigidedit.cpp
  auth/qgsauthconfigselect.cpp
  auth/qgsautheditorwidgets.cpp
  auth/qgsauthguiutils.cpp
  auth/qgsauthidentitieseditor.cpp
  auth/qgsauthimportcertdialog.cpp
  auth/qgsauthimportidentitydialog.cpp
  auth/qgsauthmasterpassresetdialog.cpp
  auth/qgsauthserverseditor.cpp
  auth/qgsauthsslconfigwidget.cpp
  auth/qgsauthsslerrorsdialog.cpp
  auth/qgsauthsslimportdialog.cpp
  auth/qgsauthtrustedcasdialog.cpp

  editorwidgets/core/qgseditorconfigwidget.cpp
  editorwidgets/core/qgseditorwidgetfactory.cpp
  editorwidgets/core/qgseditorwidgetregistry.cpp
  editorwidgets/core/qgseditorwidgetwrapper.cpp
  editorwidgets/core/qgssearchwidgetwrapper.cpp
  editorwidgets/core/qgswidgetwrapper.cpp

  editorwidgets/qgscheckboxconfigdlg.cpp
  editorwidgets/qgscheckboxsearchwidgetwrapper.cpp
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
  editorwidgets/qgsdatetimesearchwidgetwrapper.cpp
  editorwidgets/qgsdefaultsearchwidgetwrapper.cpp
  editorwidgets/qgsdoublespinbox.cpp
  editorwidgets/qgsdummyconfigdlg.cpp
  editorwidgets/qgsenumerationwidgetwrapper.cpp
  editorwidgets/qgsenumerationwidgetfactory.cpp
  editorwidgets/qgsexternalresourceconfigdlg.cpp
  editorwidgets/qgsexternalresourcewidgetwrapper.cpp
  editorwidgets/qgsexternalresourcewidgetfactory.cpp
  editorwidgets/qgsfilenamewidgetwrapper.cpp
  editorwidgets/qgsfilenamewidgetfactory.cpp
  editorwidgets/qgshiddenwidgetwrapper.cpp
  editorwidgets/qgshiddenwidgetfactory.cpp
  editorwidgets/qgsmultiedittoolbutton.cpp
  editorwidgets/qgsphotoconfigdlg.cpp
  editorwidgets/qgsphotowidgetwrapper.cpp
  editorwidgets/qgsphotowidgetfactory.cpp
  editorwidgets/qgsrangeconfigdlg.cpp
  editorwidgets/qgsrangewidgetwrapper.cpp
  editorwidgets/qgsrangewidgetfactory.cpp
  editorwidgets/qgssearchwidgettoolbutton.cpp
  editorwidgets/qgsspinbox.cpp
  editorwidgets/qgsrelationwidgetwrapper.cpp
  editorwidgets/qgsrelationreferenceconfigdlg.cpp
  editorwidgets/qgsrelationreferencefactory.cpp
  editorwidgets/qgsrelationreferencesearchwidgetwrapper.cpp
  editorwidgets/qgsrelationreferencewidget.cpp
  editorwidgets/qgsrelationreferencewidgetwrapper.cpp
  editorwidgets/qgstexteditconfigdlg.cpp
  editorwidgets/qgstexteditsearchwidgetwrapper.cpp
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
  layertree/qgslayertreeembeddedconfigwidget.cpp
  layertree/qgslayertreeembeddedwidgetregistry.cpp
  layertree/qgslayertreeembeddedwidgetsimpl.cpp
  layertree/qgslayertreemapcanvasbridge.cpp
  layertree/qgslayertreeview.cpp
  layertree/qgslayertreeviewdefaultactions.cpp

  qgisgui.cpp
  qgisinterface.cpp
  qgsactionmenu.cpp
  qgsadvanceddigitizingcanvasitem.cpp
  qgsadvanceddigitizingdockwidget.cpp
  qgsannotationitem.cpp
  qgsattributedialog.cpp
  qgsattributeeditor.cpp
  qgsattributeform.cpp
  qgsattributeformeditorwidget.cpp
  qgsattributeforminterface.cpp
  qgsattributeformlegacyinterface.cpp
  qgsattributetypeloaddialog.cpp
  qgsblendmodecombobox.cpp
  qgsbrowsertreeview.cpp
  qgsbusyindicatordialog.cpp
  qgscharacterselectdialog.cpp
  qgscodeeditor.cpp
  qgscodeeditorcss.cpp
  qgscodeeditorhtml.cpp
  qgscodeeditorpython.cpp
  qgscodeeditorsql.cpp
  qgscollapsiblegroupbox.cpp
  qgscolorbutton.cpp
  qgscolorbuttonv2.cpp
  qgscolordialog.cpp
  qgscolorschemelist.cpp
  qgscolorswatchgrid.cpp
  qgscolorwidgets.cpp
  qgscomposeritemcombobox.cpp
  qgscomposerruler.cpp
  qgscomposerview.cpp
  qgscompoundcolorwidget.cpp
  qgsconfigureshortcutsdialog.cpp
  qgscredentialdialog.cpp
  qgscursors.cpp
  qgsdatadefinedbutton.cpp
  qgsdatumtransformdialog.cpp
  qgsdetaileditemdata.cpp
  qgsdetaileditemdelegate.cpp
  qgsdetaileditemwidget.cpp
  qgsdial.cpp
  qgsdialog.cpp
  qgsdockwidget.cpp
  qgsencodingfiledialog.cpp
  qgserrordialog.cpp
  qgsexpressionbuilderdialog.cpp
  qgsexpressionbuilderwidget.cpp
  qgsexpressionhighlighter.cpp
  qgsexpressionselectiondialog.cpp
  qgsextentgroupbox.cpp
  qgsexternalresourcewidget.cpp
  qgsfeatureselectiondlg.cpp
  qgsfieldcombobox.cpp
  qgsfieldexpressionwidget.cpp
  qgsfieldmodel.cpp
  qgsfieldproxymodel.cpp
  qgsfieldvalidator.cpp
  qgsfiledropedit.cpp
  qgsfilewidget.cpp
  qgsfilterlineedit.cpp
  qgsfocuswatcher.cpp
  qgsformannotationitem.cpp
  qgsgenericprojectionselector.cpp
  qgsgeometryrubberband.cpp
  qgsgradientstopeditor.cpp
  qgsgroupwmsdatadialog.cpp
  qgshighlight.cpp
  qgshistogramwidget.cpp
  qgshtmlannotationitem.cpp
  qgsidentifymenu.cpp
  qgslegendfilterbutton.cpp
  qgslegendinterface.cpp
  qgsludialog.cpp
  qgsmanageconnectionsdialog.cpp
  qgsmapcanvas.cpp
  qgsmapcanvasitem.cpp
  qgsmapcanvasmap.cpp
  qgsmapcanvassnapper.cpp
  qgsmapcanvassnappingutils.cpp
  qgsmapcanvastracer.cpp
  qgsmaplayeractionregistry.cpp
  qgsmaplayercombobox.cpp
  qgsmaplayermodel.cpp
  qgsmaplayerconfigwidgetfactory.cpp
  qgsmaplayerproxymodel.cpp
  qgsmaplayerstylemanagerwidget.cpp
  qgsmapmouseevent.cpp
  qgsmapoverviewcanvas.cpp
  qgsmaptip.cpp
  qgsmaptool.cpp
  qgsmaptooladvanceddigitizing.cpp
  qgsmaptoolcapture.cpp
  qgsmaptooledit.cpp
  qgsmaptoolemitpoint.cpp
  qgsmaptoolidentify.cpp
  qgsmaptoolidentifyfeature.cpp
  qgsmaptoolpan.cpp
  qgsmaptoolzoom.cpp
  qgsmaplayerconfigwidget.cpp
  qgsmessagebar.cpp
  qgsmessagebaritem.cpp
  qgsmessagelogviewer.cpp
  qgsmessageviewer.cpp
  qgsnewhttpconnection.cpp
  qgsnewmemorylayerdialog.cpp
  qgsnewnamedialog.cpp
  qgsnewvectorlayerdialog.cpp
  qgsnewgeopackagelayerdialog.cpp
  qgsnumericsortlistviewitem.cpp
  qgsoptionsdialogbase.cpp
  qgsorderbydialog.cpp
  qgsowssourceselect.cpp
  qgssourceselectdialog.cpp
  qgspanelwidget.cpp
  qgspanelwidgetstack.cpp
  qgspixmaplabel.cpp
  qgspluginmanagerinterface.cpp
  qgsprevieweffect.cpp
  qgsprojectbadlayerguihandler.cpp
  qgsprojectionselectionwidget.cpp
  qgsprojectionselector.cpp
  qgsquerybuilder.cpp
  qgsrasterformatsaveoptionswidget.cpp
  qgsrasterlayersaveasdialog.cpp
  qgsrasterpyramidsoptionswidget.cpp
  qgsrelationeditorwidget.cpp
  qgsrubberband.cpp
  qgsscalecombobox.cpp
  qgsscalerangewidget.cpp
  qgsscalevisibilitydialog.cpp
  qgsscalewidget.cpp
  qgssearchquerybuilder.cpp
  qgsshortcutsmanager.cpp
  qgsslider.cpp
  qgssublayersdialog.cpp
  qgssqlcomposerdialog.cpp
  qgssvgannotationitem.cpp
  qgstablewidgetitem.cpp
  qgstextannotationitem.cpp
  qgstrackedvectorlayertools.cpp
  qgsunitselectionwidget.cpp
  qgsuserinputdockwidget.cpp
  qgsvariableeditorwidget.cpp
  qgsvertexmarker.cpp
)

IF (WITH_QTWEBKIT)
  SET(QGIS_GUI_SRCS
      ${QGIS_GUI_SRCS}
      editorwidgets/qgswebviewconfigdlg.cpp
      editorwidgets/qgswebviewwidgetfactory.cpp
      editorwidgets/qgswebviewwidgetwrapper.cpp
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
  qgsadvanceddigitizingdockwidget.h
  qgsattributedialog.h
  qgsattributeeditor.h
  qgsattributeform.h
  qgsattributeformeditorwidget.h
  qgsattributetypeloaddialog.h
  qgsblendmodecombobox.h
  qgsbrowsertreeview.h
  qgsbusyindicatordialog.h
  qgscharacterselectdialog.h
  qgscodeeditor.h
  qgscodeeditorcss.h
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
  qgscomposeritemcombobox.h
  qgscomposerruler.h
  qgscomposerview.h
  qgscompoundcolorwidget.h
  qgsconfigureshortcutsdialog.h
  qgscredentialdialog.h
  qgsdatadefinedbutton.h
  qgsdatumtransformdialog.h
  qgsdetaileditemdelegate.h
  qgsdetaileditemwidget.h
  qgsdial.h
  qgsdialog.h
  qgsdockwidget.h
  qgsencodingfiledialog.h
  qgserrordialog.h
  qgsexpressionbuilderdialog.h
  qgsexpressionbuilderwidget.h
  qgsexpressionhighlighter.h
  qgsexpressionselectiondialog.h
  qgsextentgroupbox.h
  qgsexternalresourcewidget.h
  qgsfeatureselectiondlg.h
  qgsfieldcombobox.h
  qgsfieldexpressionwidget.h
  qgsfieldmodel.h
  qgsfieldproxymodel.h
  qgsfieldvalidator.h
  qgsfiledropedit.h
  qgsfilewidget.h
  qgsfilterlineedit.h
  qgsfocuswatcher.h
  qgsformannotationitem.h
  qgsgenericprojectionselector.h
  qgsgradientstopeditor.h
  qgsgroupwmsdatadialog.h
  qgshistogramwidget.h
  qgshtmlannotationitem.h
  qgsidentifymenu.h
  qgslegendfilterbutton.h
  qgslegendinterface.h
  qgslonglongvalidator.h
  qgsludialog.h
  qgsmanageconnectionsdialog.h
  qgsmapcanvas.h
  qgsmapcanvassnappingutils.h
  qgsmapcanvastracer.h
  qgsmaplayeractionregistry.h
  qgsmaplayercombobox.h
  qgsmaplayermodel.h
  qgsmaplayerproxymodel.h
  qgsmaplayerstylemanagerwidget.h
  qgsmapoverviewcanvas.h
  qgsmaptip.h
  qgsmaptool.h
  qgsmaptooladvanceddigitizing.h
  qgsmaptoolcapture.h
  qgsmaptooledit.h
  qgsmaptoolemitpoint.h
  qgsmaptoolidentify.h
  qgsmaptoolidentifyfeature.h
  qgsmaptoolpan.h
  qgsmaptoolzoom.h
  qgsmaplayerconfigwidget.h
  qgsmessagebar.h
  qgsmessagebaritem.h
  qgsmessagelogviewer.h
  qgsmessageviewer.h
  qgsnewhttpconnection.h
  qgsnewmemorylayerdialog.h
  qgsnewnamedialog.h
  qgsnewvectorlayerdialog.h
  qgsnewgeopackagelayerdialog.h
  qgsoptionsdialogbase.h
  qgsorderbydialog.h
  qgsowssourceselect.h
  qgssourceselectdialog.h
  qgspanelwidget.h
  qgspanelwidgetstack.h
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
  qgsrelationeditorwidget.h
  qgsscalecombobox.h
  qgsscalerangewidget.h
  qgsscalevisibilitydialog.h
  qgsscalewidget.h
  qgssearchquerybuilder.h
  qgsshortcutsmanager.h
  qgsslider.h
  qgssqlcomposerdialog.h
  qgssublayersdialog.h
  qgsunitselectionwidget.h
  qgsuserinputdockwidget.h
  qgsvariableeditorwidget.h

  raster/qgsmultibandcolorrendererwidget.h
  raster/qgspalettedrendererwidget.h
  raster/qgsrasterhistogramwidget.h
  raster/qgsrasterminmaxwidget.h
  raster/qgsrasterrendererwidget.h
  raster/qgssinglebandgrayrendererwidget.h
  raster/qgssinglebandpseudocolorrendererwidget.h
  raster/qgsrendererrasterpropertieswidget.h
  raster/qgsrastertransparencywidget.h
  raster/qgshillshaderendererwidget.h

  symbology-ng/qgs25drendererwidget.h
  symbology-ng/characterwidget.h
  symbology-ng/qgsarrowsymbollayerwidget.h
  symbology-ng/qgsbrushstylecombobox.h
  symbology-ng/qgscategorizedsymbolrendererv2widget.h
  symbology-ng/qgscolorrampcombobox.h
  symbology-ng/qgscptcitycolorrampv2dialog.h
  symbology-ng/qgsdashspacedialog.h
  symbology-ng/qgsdatadefinedsymboldialog.h
  symbology-ng/qgsellipsesymbollayerv2widget.h
  symbology-ng/qgsgraduatedhistogramwidget.h
  symbology-ng/qgsgraduatedsymbolrendererv2widget.h
  symbology-ng/qgsheatmaprendererwidget.h
  symbology-ng/qgsinvertedpolygonrendererwidget.h
  symbology-ng/qgslayerpropertieswidget.h
  symbology-ng/qgsnullsymbolrendererwidget.h
  symbology-ng/qgspenstylecombobox.h
  symbology-ng/qgspointdisplacementrendererwidget.h
  symbology-ng/qgsrendererv2propertiesdialog.h
  symbology-ng/qgsrendererv2widget.h
  symbology-ng/qgsrulebasedrendererv2widget.h
  symbology-ng/qgssinglesymbolrendererv2widget.h
  symbology-ng/qgssizescalewidget.h
  symbology-ng/qgssmartgroupeditordialog.h
  symbology-ng/qgsstylev2exportimportdialog.h
  symbology-ng/qgsstylev2groupselectiondialog.h
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

  attributetable/qgsattributetabledelegate.h
  attributetable/qgsattributetablefiltermodel.h
  attributetable/qgsattributetablemodel.h
  attributetable/qgsattributetableview.h
  attributetable/qgsdualview.h
  attributetable/qgsfeaturelistmodel.h
  attributetable/qgsfeaturelistview.h
  attributetable/qgsfeaturelistviewdelegate.h
  attributetable/qgsfeatureselectionmodel.h
  attributetable/qgsfieldconditionalformatwidget.h
  attributetable/qgsgenericfeatureselectionmanager.h
  attributetable/qgsifeatureselectionmanager.h
  attributetable/qgsvectorlayerselectionmanager.h
  attributetable/qgsorganizetablecolumnsdialog.h

  auth/qgsauthauthoritieseditor.h
  auth/qgsauthcertificateinfo.h
  auth/qgsauthcertificatemanager.h
  auth/qgsauthcerttrustpolicycombobox.h
  auth/qgsauthconfigedit.h
  auth/qgsauthconfigeditor.h
  auth/qgsauthconfigidedit.h
  auth/qgsauthconfigselect.h
  auth/qgsautheditorwidgets.h
  auth/qgsauthidentitieseditor.h
  auth/qgsauthimportcertdialog.h
  auth/qgsauthimportidentitydialog.h
  auth/qgsauthmasterpassresetdialog.h
  auth/qgsauthmethodedit.h
  auth/qgsauthserverseditor.h
  auth/qgsauthsslconfigwidget.h
  auth/qgsauthsslerrorsdialog.h
  auth/qgsauthsslimportdialog.h
  auth/qgsauthtrustedcasdialog.h

  effects/qgseffectdrawmodecombobox.h
  effects/qgseffectstackpropertieswidget.h
  effects/qgspainteffectpropertieswidget.h
  effects/qgspainteffectwidget.h

  editorwidgets/core/qgseditorconfigwidget.h
  editorwidgets/core/qgseditorconfigwidget.h
  editorwidgets/core/qgseditorwidgetregistry.h
  editorwidgets/core/qgseditorwidgetregistry.h
  editorwidgets/core/qgseditorwidgetwrapper.h
  editorwidgets/core/qgssearchwidgetwrapper.h
  editorwidgets/core/qgswidgetwrapper.h

  editorwidgets/qgscheckboxconfigdlg.h
  editorwidgets/qgscheckboxsearchwidgetwrapper.h
  editorwidgets/qgscheckboxwidgetwrapper.h
  editorwidgets/qgsclassificationwidgetwrapper.h
  editorwidgets/qgscolorwidgetwrapper.h
  editorwidgets/qgsdatetimeedit.h
  editorwidgets/qgsdatetimeeditconfig.h
  editorwidgets/qgsdatetimesearchwidgetwrapper.h
  editorwidgets/qgsdatetimeeditwrapper.h
  editorwidgets/qgsdefaultsearchwidgetwrapper.h
  editorwidgets/qgsdoublespinbox.h
  editorwidgets/qgsdummyconfigdlg.h
  editorwidgets/qgsenumerationwidgetwrapper.h
  editorwidgets/qgsexternalresourceconfigdlg.h
  editorwidgets/qgsexternalresourcewidgetwrapper.h
  editorwidgets/qgsfilenamewidgetwrapper.h
  editorwidgets/qgshiddenwidgetwrapper.h
  editorwidgets/qgsmultiedittoolbutton.h
  editorwidgets/qgsphotoconfigdlg.h
  editorwidgets/qgsphotowidgetwrapper.h
  editorwidgets/qgsrangeconfigdlg.h
  editorwidgets/qgsrangewidgetwrapper.h
  editorwidgets/qgsrelationreferenceconfigdlg.h
  editorwidgets/qgsrelationreferencesearchwidgetwrapper.h
  editorwidgets/qgsrelationreferencewidget.h
  editorwidgets/qgsrelationreferencewidgetwrapper.h
  editorwidgets/qgsrelationwidgetwrapper.h
  editorwidgets/qgssearchwidgettoolbutton.h
  editorwidgets/qgsspinbox.h
  editorwidgets/qgstexteditconfigdlg.h
  editorwidgets/qgstexteditwrapper.h
  editorwidgets/qgstexteditsearchwidgetwrapper.h
  editorwidgets/qgsuniquevaluesconfigdlg.h
  editorwidgets/qgsuniquevaluewidgetwrapper.h
  editorwidgets/qgsuuidwidgetwrapper.h
  editorwidgets/qgsvaluemapconfigdlg.h
  editorwidgets/qgsvaluemapsearchwidgetwrapper.h
  editorwidgets/qgsvaluemapwidgetwrapper.h
  editorwidgets/qgsvaluerelationconfigdlg.h
  editorwidgets/qgsvaluerelationsearchwidgetwrapper.h
  editorwidgets/qgsvaluerelationwidgetwrapper.h

  layertree/qgscustomlayerorderwidget.h
  layertree/qgslayertreeembeddedconfigwidget.h
  layertree/qgslayertreeembeddedwidgetsimpl.h
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
  SET_SOURCE_FILES_PROPERTIES(${QGIS_GUI_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
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
  qgsmapcanvastracer.h
  qgsmaplayerconfigwidgetfactory.h
  qgsmapmouseevent.h
  qgsmaptip.h
  qgsnumericsortlistviewitem.h
  qgsrubberband.h
  qgssqlcomposerdialog.h
  qgssvgannotationitem.h
  qgstablewidgetitem.h
  qgstextannotationitem.h
  qgstrackedvectorlayertools.h
  qgsuserinputdockwidget.h
  qgsvectorlayertools.h
  qgsvertexmarker.h

  attributetable/qgsfeaturemodel.h

  auth/qgsauthauthoritieseditor.h
  auth/qgsauthcertificateinfo.h
  auth/qgsauthcertificatemanager.h
  auth/qgsauthcerttrustpolicycombobox.h
  auth/qgsauthconfigedit.h
  auth/qgsauthconfigeditor.h
  auth/qgsauthconfigidedit.h
  auth/qgsauthconfigselect.h
  auth/qgsautheditorwidgets.h
  auth/qgsauthguiutils.h
  auth/qgsauthidentitieseditor.h
  auth/qgsauthimportcertdialog.h
  auth/qgsauthimportidentitydialog.h
  auth/qgsauthmasterpassresetdialog.h
  auth/qgsauthmethodedit.h
  auth/qgsauthserverseditor.h
  auth/qgsauthsslconfigwidget.h
  auth/qgsauthsslerrorsdialog.h
  auth/qgsauthsslimportdialog.h
  auth/qgsauthtrustedcasdialog.h

  editorwidgets/core/qgseditorwidgetfactory.h

  editorwidgets/qgscheckboxwidgetfactory.h
  editorwidgets/qgsclassificationwidgetwrapperfactory.h
  editorwidgets/qgscolorwidgetfactory.h
  editorwidgets/qgsdatetimeeditfactory.h
  editorwidgets/qgsenumerationwidgetfactory.h
  editorwidgets/qgsexternalresourcewidgetfactory.h
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

  layertree/qgslayertreeembeddedconfigwidget.h
  layertree/qgslayertreeembeddedwidgetregistry.h

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
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscredentialdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdetaileditemwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionbuilderdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionbuilder.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionselectiondialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgenericprojectionselectorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessagelogviewer.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessageviewer.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsowssourceselectbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprojectionselectorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsquerybuilderbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssqlcomposerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssublayersdialogbase.h
)

IF(ENABLE_MODELTEST)
  INCLUDE_DIRECTORIES(../../tests/qt_modeltest)
ENDIF(ENABLE_MODELTEST)

INCLUDE_DIRECTORIES(SYSTEM
  ${QT_QTUITOOLS_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/symbology-ng
  ${CMAKE_CURRENT_SOURCE_DIR}/attributetable
  ${CMAKE_CURRENT_SOURCE_DIR}/auth
  ${CMAKE_CURRENT_SOURCE_DIR}/editorwidgets
  ${CMAKE_CURRENT_SOURCE_DIR}/editorwidgets/core
  ${CMAKE_CURRENT_SOURCE_DIR}/layertree
  ${CMAKE_CURRENT_SOURCE_DIR}/effects
  ../core
  ../core/auth
  ../core/composer
  ../core/geometry
  ../core/layertree
  ../core/raster
  ../core/symbology-ng
  ../core/effects
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/../ui
)
INCLUDE_DIRECTORIES(SYSTEM
  ${QCA_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

REMOVE_DEFINITIONS("-DGUI_EXPORT=${DLLIMPORT}")
ADD_DEFINITIONS("\"-DGUI_EXPORT=${DLLEXPORT}\"")

# disable deprecation warnings for classes re-exporting deprecated methods
IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(
    ${CMAKE_BINARY_DIR}/src/gui/moc_qgisinterface.cxx
    ${CMAKE_BINARY_DIR}/src/gui/moc_qgsgenericprojectionselector.cxx
    ${CMAKE_BINARY_DIR}/src/gui/moc_qgsprojectionselector.cxx
    PROPERTIES COMPILE_FLAGS "/wd4996")
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
