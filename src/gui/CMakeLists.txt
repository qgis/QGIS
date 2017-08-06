SET(QGIS_GUI_SRCS
  raster/qgsmultibandcolorrendererwidget.cpp
  raster/qgspalettedrendererwidget.cpp
  raster/qgsrasterbandcombobox.cpp
  raster/qgsrasterhistogramwidget.cpp
  raster/qgsrasterminmaxwidget.cpp
  raster/qgsrasterrendererwidget.cpp
  raster/qgssinglebandgrayrendererwidget.cpp
  raster/qgssinglebandpseudocolorrendererwidget.cpp
  raster/qgsrendererrasterpropertieswidget.cpp
  raster/qgsrastertransparencywidget.cpp
  raster/qgshillshaderendererwidget.cpp

  symbology/qgs25drendererwidget.cpp
  symbology/characterwidget.cpp
  symbology/qgsarrowsymbollayerwidget.cpp
  symbology/qgsbrushstylecombobox.cpp
  symbology/qgscategorizedsymbolrendererwidget.cpp
  symbology/qgscptcitycolorrampdialog.cpp
  symbology/qgsdashspacedialog.cpp
  symbology/qgsdatadefinedsizelegendwidget.cpp
  symbology/qgsellipsesymbollayerwidget.cpp
  symbology/qgsgraduatedhistogramwidget.cpp
  symbology/qgsgraduatedsymbolrendererwidget.cpp
  symbology/qgsheatmaprendererwidget.cpp
  symbology/qgsinvertedpolygonrendererwidget.cpp
  symbology/qgslayerpropertieswidget.cpp
  symbology/qgsnullsymbolrendererwidget.cpp
  symbology/qgspenstylecombobox.cpp
  symbology/qgspointclusterrendererwidget.cpp
  symbology/qgspointdisplacementrendererwidget.cpp
  symbology/qgsrendererpropertiesdialog.cpp
  symbology/qgsrendererwidget.cpp
  symbology/qgsrulebasedrendererwidget.cpp
  symbology/qgssinglesymbolrendererwidget.cpp
  symbology/qgssmartgroupeditordialog.cpp
  symbology/qgsstyleexportimportdialog.cpp
  symbology/qgsstylegroupselectiondialog.cpp
  symbology/qgsstylemanagerdialog.cpp
  symbology/qgsstylesavedialog.cpp
  symbology/qgssvgselectorwidget.cpp
  symbology/qgssymbollayerwidget.cpp
  symbology/qgssymbollevelsdialog.cpp
  symbology/qgssymbolslistwidget.cpp
  symbology/qgssymbolselectordialog.cpp
  symbology/qgssymbolwidgetcontext.cpp
  symbology/qgsvectorfieldsymbollayerwidget.cpp

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
  editorwidgets/core/qgseditorwidgetautoconf.cpp
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
  editorwidgets/qgshiddenwidgetwrapper.cpp
  editorwidgets/qgshiddenwidgetfactory.cpp
  editorwidgets/qgskeyvaluewidgetfactory.cpp
  editorwidgets/qgskeyvaluewidgetwrapper.cpp
  editorwidgets/qgslistwidgetfactory.cpp
  editorwidgets/qgslistwidgetwrapper.cpp
  editorwidgets/qgsmultiedittoolbutton.cpp
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

  geonode/qgsgeonodenewconnection.cpp
  geonode/qgsgeonodesourceselect.cpp

  layertree/qgscustomlayerorderwidget.cpp
  layertree/qgslayertreeembeddedconfigwidget.cpp
  layertree/qgslayertreeembeddedwidgetregistry.cpp
  layertree/qgslayertreeembeddedwidgetsimpl.cpp
  layertree/qgslayertreemapcanvasbridge.cpp
  layertree/qgslayertreeview.cpp
  layertree/qgslayertreeviewdefaultactions.cpp

  layout/qgslayoutitemguiregistry.cpp
  layout/qgslayoutitemwidget.cpp
  layout/qgslayoutnewitempropertiesdialog.cpp
  layout/qgslayoutruler.cpp
  layout/qgslayoutunitscombobox.cpp
  layout/qgslayoutview.cpp
  layout/qgslayoutviewmouseevent.cpp
  layout/qgslayoutviewrubberband.cpp
  layout/qgslayoutviewtool.cpp
  layout/qgslayoutviewtooladditem.cpp
  layout/qgslayoutviewtoolpan.cpp
  layout/qgslayoutviewtoolselect.cpp
  layout/qgslayoutviewtooltemporarykeypan.cpp
  layout/qgslayoutviewtooltemporarykeyzoom.cpp
  layout/qgslayoutviewtooltemporarymousepan.cpp
  layout/qgslayoutviewtoolzoom.cpp

  locator/qgslocator.cpp
  locator/qgslocatorfilter.cpp
  locator/qgslocatorwidget.cpp

  ogr/qgsogrhelperfunctions.cpp
  ogr/qgsnewogrconnection.cpp
  ogr/qgsvectorlayersaveasdialog.cpp

  qgisinterface.cpp
  qgsactionmenu.cpp
  qgsadvanceddigitizingcanvasitem.cpp
  qgsadvanceddigitizingdockwidget.cpp
  qgsattributedialog.cpp
  qgsattributeform.cpp
  qgsattributeformeditorwidget.cpp
  qgsattributeforminterface.cpp
  qgsattributeformlegacyinterface.cpp
  qgsattributetypeloaddialog.cpp
  qgsblendmodecombobox.cpp
  qgsbrowsertreeview.cpp
  qgsbrowserdockwidget.cpp
  qgsbrowserdockwidget_p.cpp
  qgsbusyindicatordialog.cpp
  qgscharacterselectordialog.cpp
  qgscheckablecombobox.cpp
  qgscodeeditor.cpp
  qgscodeeditorcss.cpp
  qgscodeeditorhtml.cpp
  qgscodeeditorpython.cpp
  qgscodeeditorsql.cpp
  qgscollapsiblegroupbox.cpp
  qgscolorbrewercolorrampdialog.cpp
  qgscolorbutton.cpp
  qgscolorrampbutton.cpp
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
  qgscustomdrophandler.cpp
  qgscurveeditorwidget.cpp
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
  qgsexpressionlineedit.cpp
  qgsexpressionselectiondialog.cpp
  qgsextentgroupbox.cpp
  qgsexternalresourcewidget.cpp
  qgsfeatureselectiondlg.cpp
  qgsfieldcombobox.cpp
  qgsfieldexpressionwidget.cpp
  qgsfieldvalidator.cpp
  qgsfieldvalueslineedit.cpp
  qgsfilewidget.cpp
  qgsfilterlineedit.cpp
  qgsfloatingwidget.cpp
  qgsfocuswatcher.cpp
  qgsfontbutton.cpp
  qgsformannotation.cpp
  qgsgeometryrubberband.cpp
  qgsgradientcolorrampdialog.cpp
  qgsgradientstopeditor.cpp
  qgsgroupwmsdatadialog.cpp
  qgsgui.cpp
  qgsguiutils.cpp
  qgshighlight.cpp
  qgshistogramwidget.cpp
  qgshelp.cpp
  qgsidentifymenu.cpp
  qgskeyvaluewidget.cpp
  qgslistwidget.cpp
  qgslegendfilterbutton.cpp
  qgslimitedrandomcolorrampdialog.cpp
  qgsratiolockbutton.cpp
  qgsludialog.cpp
  qgsmanageconnectionsdialog.cpp
  qgsmapcanvas.cpp
  qgsmapcanvasannotationitem.cpp
  qgsmapcanvasitem.cpp
  qgsmapcanvasmap.cpp
  qgsmapcanvassnappingutils.cpp
  qgsmapcanvastracer.cpp
  qgsmaplayeractionregistry.cpp
  qgsmaplayercombobox.cpp
  qgsmaplayerconfigwidgetfactory.cpp
  qgsmaplayerstylemanagerwidget.cpp
  qgsmapmouseevent.cpp
  qgsmapoverviewcanvas.cpp
  qgsmaptip.cpp
  qgsmaptool.cpp
  qgsmaptooladvanceddigitizing.cpp
  qgsmaptoolcapture.cpp
  qgsmaptooledit.cpp
  qgsmaptoolemitpoint.cpp
  qgsmaptoolextent.cpp
  qgsmaptoolidentify.cpp
  qgsmaptoolidentifyfeature.cpp
  qgsmaptoolpan.cpp
  qgsmaptoolzoom.cpp
  qgsmaplayerconfigwidget.cpp
  qgsmenuheader.cpp
  qgsmessagebar.cpp
  qgsmessagebaritem.cpp
  qgsmessagelogviewer.cpp
  qgsmessageviewer.cpp
  qgsnewhttpconnection.cpp
  qgsnewmemorylayerdialog.cpp
  qgsnewnamedialog.cpp
  qgsnewvectorlayerdialog.cpp
  qgsnewgeopackagelayerdialog.cpp
  qgsopacitywidget.cpp
  qgsoptionsdialogbase.cpp
  qgsorderbydialog.cpp
  qgsowssourceselect.cpp
  qgspanelwidget.cpp
  qgspanelwidgetstack.cpp
  qgspasswordlineedit.cpp
  qgspixmaplabel.cpp
  qgspluginmanagerinterface.cpp
  qgspresetcolorrampdialog.cpp
  qgsprevieweffect.cpp
  qgsprojectionselectiondialog.cpp
  qgsprojectionselectionwidget.cpp
  qgsprojectionselectiontreewidget.cpp
  qgspropertyassistantwidget.cpp
  qgspropertyoverridebutton.cpp
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
  qgsscrollarea.cpp
  qgssearchquerybuilder.cpp
  qgsshortcutsmanager.cpp
  qgsslider.cpp
  qgssublayersdialog.cpp
  qgssubstitutionlistwidget.cpp
  qgssqlcomposerdialog.cpp
  qgsstatusbar.cpp
  qgssymbolbutton.cpp
  qgstablewidgetbase.cpp
  qgstabwidget.cpp
  qgstablewidgetitem.cpp
  qgstaskmanagerwidget.cpp
  qgstextformatwidget.cpp
  qgstextpreview.cpp
  qgstreewidgetitem.cpp
  qgsunitselectionwidget.cpp
  qgsuserinputdockwidget.cpp
  qgsvariableeditorwidget.cpp
  qgsvertexmarker.cpp
  qgsfiledownloader.cpp
  qgsdatasourcemanagerdialog.cpp
  qgsabstractdatasourcewidget.cpp
  qgssourceselectprovider.cpp
  qgssourceselectproviderregistry.cpp
)

SET(QGIS_GUI_MOC_HDRS
  qgisinterface.h
  qgsactionmenu.h
  qgsadvanceddigitizingdockwidget.h
  qgsattributedialog.h
  qgsattributeform.h
  qgsattributeformeditorwidget.h
  qgsattributetypeloaddialog.h
  qgsblendmodecombobox.h
  qgsbrowsertreeview.h
  qgsbusyindicatordialog.h
  qgscharacterselectordialog.h
  qgscheckablecombobox.h
  qgscodeeditor.h
  qgscodeeditorcss.h
  qgscodeeditorhtml.h
  qgscodeeditorpython.h
  qgscodeeditorsql.h
  qgscollapsiblegroupbox.h
  qgscolorbrewercolorrampdialog.h
  qgscolorbutton.h
  qgscolorrampbutton.h
  qgscolordialog.h
  qgscolorschemelist.h
  qgscolorswatchgrid.h
  qgscolorwidgets.h
  qgscomposerinterface.h
  qgscomposeritemcombobox.h
  qgscomposerruler.h
  qgscomposerview.h
  qgscompoundcolorwidget.h
  qgsconfigureshortcutsdialog.h
  qgscredentialdialog.h
  qgscurveeditorwidget.h
  qgsdatumtransformdialog.h
  qgsdetaileditemdelegate.h
  qgsdetaileditemwidget.h
  qgsdial.h
  qgsdialog.h
  qgsdockwidget.h
  qgsbrowserdockwidget_p.h
  qgsbrowserdockwidget.h
  qgsencodingfiledialog.h
  qgserrordialog.h
  qgsexpressionbuilderdialog.h
  qgsexpressionbuilderwidget.h
  qgsexpressionhighlighter.h
  qgsexpressionlineedit.h
  qgsexpressionselectiondialog.h
  qgsextentgroupbox.h
  qgsexternalresourcewidget.h
  qgsfeatureselectiondlg.h
  qgsfieldcombobox.h
  qgsfieldexpressionwidget.h
  qgsfieldvalidator.h
  qgsfieldvalueslineedit.h
  qgsfilewidget.h
  qgsfilterlineedit.h
  qgsfloatingwidget.h
  qgsfocuswatcher.h
  qgsfontbutton.h
  qgsformannotation.h
  qgsgradientcolorrampdialog.h
  qgsgradientstopeditor.h
  qgsgroupwmsdatadialog.h
  qgshistogramwidget.h
  qgsidentifymenu.h
  qgskeyvaluewidget.h
  qgslistwidget.h
  qgslegendfilterbutton.h
  qgslimitedrandomcolorrampdialog.h
  qgsratiolockbutton.h
  qgslonglongvalidator.h
  qgsludialog.h
  qgsmanageconnectionsdialog.h
  qgsmapcanvas.h
  qgsmapcanvasannotationitem.h
  qgsmapcanvassnappingutils.h
  qgsmapcanvastracer.h
  qgsmaplayeractionregistry.h
  qgsmaplayercombobox.h
  qgsmaplayerstylemanagerwidget.h
  qgsmapoverviewcanvas.h
  qgsmaptip.h
  qgsmaptool.h
  qgsmaptooladvanceddigitizing.h
  qgsmaptoolcapture.h
  qgsmaptooledit.h
  qgsmaptoolemitpoint.h
  qgsmaptoolextent.h
  qgsmaptoolidentify.h
  qgsmaptoolidentifyfeature.h
  qgsmaptoolpan.h
  qgsmaptoolzoom.h
  qgsmaplayerconfigwidget.h
  qgsmenuheader.h
  qgsmessagebar.h
  qgsmessagebaritem.h
  qgsmessagelogviewer.h
  qgsmessageviewer.h
  qgsnewhttpconnection.h
  qgsnewmemorylayerdialog.h
  qgsnewnamedialog.h
  qgsnewvectorlayerdialog.h
  qgsnewgeopackagelayerdialog.h
  qgsopacitywidget.h
  qgsoptionsdialogbase.h
  qgsoptionswidgetfactory.h
  qgsorderbydialog.h
  qgsowssourceselect.h
  qgspanelwidget.h
  qgspanelwidgetstack.h
  qgspasswordlineedit.h
  qgspixmaplabel.h
  qgspluginmanagerinterface.h
  qgspresetcolorrampdialog.h
  qgsprevieweffect.h
  qgsprojectionselectiondialog.h
  qgsprojectionselectionwidget.h
  qgsprojectionselectiontreewidget.h
  qgspropertyassistantwidget.h
  qgspropertyoverridebutton.h
  qgsquerybuilder.h
  qgsrasterformatsaveoptionswidget.h
  qgsrasterlayersaveasdialog.h
  qgsrasterpyramidsoptionswidget.h
  qgsrelationeditorwidget.h
  qgsscalecombobox.h
  qgsscalerangewidget.h
  qgsscalevisibilitydialog.h
  qgsscalewidget.h
  qgsscrollarea.h
  qgssearchquerybuilder.h
  qgsshortcutsmanager.h
  qgsslider.h
  qgssqlcomposerdialog.h
  qgsstatusbar.h
  qgssublayersdialog.h
  qgssubstitutionlistwidget.h
  qgssymbolbutton.h
  qgstablewidgetbase.h
  qgstabwidget.h
  qgstaskmanagerwidget.h
  qgstextformatwidget.h
  qgstextpreview.h
  qgstreewidgetitem.h
  qgsunitselectionwidget.h
  qgsuserinputdockwidget.h
  qgsvariableeditorwidget.h
  qgsfiledownloader.h
  qgsdatasourcemanagerdialog.h
  qgsabstractdatasourcewidget.h

  ogr/qgsnewogrconnection.h
  ogr/qgsvectorlayersaveasdialog.h

  raster/qgsmultibandcolorrendererwidget.h
  raster/qgspalettedrendererwidget.h
  raster/qgsrasterbandcombobox.h
  raster/qgsrasterhistogramwidget.h
  raster/qgsrasterminmaxwidget.h
  raster/qgsrasterrendererwidget.h
  raster/qgssinglebandgrayrendererwidget.h
  raster/qgssinglebandpseudocolorrendererwidget.h
  raster/qgsrendererrasterpropertieswidget.h
  raster/qgsrastertransparencywidget.h
  raster/qgshillshaderendererwidget.h

  symbology/qgs25drendererwidget.h
  symbology/characterwidget.h
  symbology/qgsarrowsymbollayerwidget.h
  symbology/qgsbrushstylecombobox.h
  symbology/qgscategorizedsymbolrendererwidget.h
  symbology/qgscptcitycolorrampdialog.h
  symbology/qgsdashspacedialog.h
  symbology/qgsdatadefinedsizelegendwidget.h
  symbology/qgsellipsesymbollayerwidget.h
  symbology/qgsgraduatedhistogramwidget.h
  symbology/qgsgraduatedsymbolrendererwidget.h
  symbology/qgsheatmaprendererwidget.h
  symbology/qgsinvertedpolygonrendererwidget.h
  symbology/qgslayerpropertieswidget.h
  symbology/qgsnullsymbolrendererwidget.h
  symbology/qgspenstylecombobox.h
  symbology/qgspointclusterrendererwidget.h
  symbology/qgspointdisplacementrendererwidget.h
  symbology/qgsrendererpropertiesdialog.h
  symbology/qgsrendererwidget.h
  symbology/qgsrulebasedrendererwidget.h
  symbology/qgssinglesymbolrendererwidget.h
  symbology/qgssmartgroupeditordialog.h
  symbology/qgsstyleexportimportdialog.h
  symbology/qgsstylegroupselectiondialog.h
  symbology/qgsstylemanagerdialog.h
  symbology/qgsstylesavedialog.h
  symbology/qgssvgselectorwidget.h
  symbology/qgssymbollayerwidget.h
  symbology/qgssymbollevelsdialog.h
  symbology/qgssymbolslistwidget.h
  symbology/qgssymbolselectordialog.h
  symbology/qgsvectorfieldsymbollayerwidget.h

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
  editorwidgets/qgshiddenwidgetwrapper.h
  editorwidgets/qgskeyvaluewidgetwrapper.h
  editorwidgets/qgslistwidgetwrapper.h
  editorwidgets/qgsmultiedittoolbutton.h
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

  geonode/qgsgeonodenewconnection.h
  geonode/qgsgeonodesourceselect.h

  layertree/qgscustomlayerorderwidget.h
  layertree/qgslayertreeembeddedconfigwidget.h
  layertree/qgslayertreeembeddedwidgetsimpl.h
  layertree/qgslayertreemapcanvasbridge.h
  layertree/qgslayertreeview.h
  layertree/qgslayertreeviewdefaultactions.h

  layout/qgslayoutdesignerinterface.h
  layout/qgslayoutitemguiregistry.h
  layout/qgslayoutitemwidget.h
  layout/qgslayoutnewitempropertiesdialog.h
  layout/qgslayoutruler.h
  layout/qgslayoutunitscombobox.h
  layout/qgslayoutview.h
  layout/qgslayoutviewtool.h
  layout/qgslayoutviewtooladditem.h
  layout/qgslayoutviewtoolpan.h
  layout/qgslayoutviewtoolselect.h
  layout/qgslayoutviewtooltemporarykeypan.h
  layout/qgslayoutviewtooltemporarykeyzoom.h
  layout/qgslayoutviewtooltemporarymousepan.h
  layout/qgslayoutviewtoolzoom.h

  locator/qgslocator.h
  locator/qgslocatorfilter.h
  locator/qgslocatorwidget.h
)
SET_PROPERTY(GLOBAL PROPERTY QGIS_GUI_MOC_HDRS ${QGIS_GUI_MOC_HDRS})

QT5_WRAP_CPP(QGIS_GUI_MOC_SRCS ${QGIS_GUI_MOC_HDRS})

IF(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_GUI_MOC_SRCS} PROPERTIES COMPILE_FLAGS "/wd4512 /wd4996" )
ELSE(MSVC)
  SET_SOURCE_FILES_PROPERTIES(${QGIS_GUI_MOC_SRCS} PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations" )
ENDIF(MSVC)

# for installing headers

SET(QGIS_GUI_HDRS
  qgsattributeeditorcontext.h
  qgsattributeforminterface.h
  qgsattributeformlegacyinterface.h
  qgscursors.h
  qgscustomdrophandler.h
  qgsdetaileditemdata.h
  qgsexpressionbuilderdialog.h
  qgsgeometryrubberband.h
  qgsgui.h
  qgsguiutils.h
  qgshelp.h
  qgshighlight.h
  qgsmapcanvasitem.h
  qgsmapcanvasmap.h
  qgsmapcanvassnappingutils.h
  qgsmapcanvastracer.h
  qgsmaplayerconfigwidgetfactory.h
  qgsmapmouseevent.h
  qgsmaptip.h
  qgsrubberband.h
  qgssqlcomposerdialog.h
  qgstablewidgetitem.h
  qgsuserinputdockwidget.h
  qgsbrowserdockwidget.h
  qgsbrowserdockwidget_p.h
  qgsvertexmarker.h
  qgsfiledownloader.h
  qgsdatasourcemanagerdialog.h
  qgsabstractdatasourcewidget.h
  qgssourceselectprovider.h
  qgssourceselectproviderregistry.h

  ogr/qgsogrhelperfunctions.h
  ogr/qgsnewogrconnection.h
  ogr/qgsvectorlayersaveasdialog.h

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
  editorwidgets/core/qgseditorwidgetautoconf.h

  editorwidgets/qgscheckboxwidgetfactory.h
  editorwidgets/qgsclassificationwidgetwrapperfactory.h
  editorwidgets/qgscolorwidgetfactory.h
  editorwidgets/qgsdatetimeeditfactory.h
  editorwidgets/qgsenumerationwidgetfactory.h
  editorwidgets/qgsexternalresourcewidgetfactory.h
  editorwidgets/qgshiddenwidgetfactory.h
  editorwidgets/qgskeyvaluewidgetfactory.h
  editorwidgets/qgslistwidgetfactory.h
  editorwidgets/qgsrangewidgetfactory.h
  editorwidgets/qgsrelationreferencefactory.h
  editorwidgets/qgstexteditwidgetfactory.h
  editorwidgets/qgsuniquevaluewidgetfactory.h
  editorwidgets/qgsuuidwidgetfactory.h
  editorwidgets/qgsvaluemapwidgetfactory.h
  editorwidgets/qgsvaluerelationwidgetfactory.h

  geonode/qgsgeonodenewconnection.h
  geonode/qgsgeonodesourceselect.h

  layertree/qgslayertreeembeddedconfigwidget.h
  layertree/qgslayertreeembeddedwidgetregistry.h

  layout/qgslayoutviewmouseevent.h
  layout/qgslayoutviewrubberband.h

  locator/qgslocatorcontext.h

  raster/qgsrasterrendererwidget.h

  symbology/qgssymbolwidgetcontext.h
)
SET_PROPERTY(GLOBAL PROPERTY QGIS_GUI_HDRS ${QGIS_GUI_HDRS})


SET(QGIS_GUI_UI_HDRS
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthauthoritieseditor.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthcertificateinfo.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthcertificatemanager.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthconfigedit.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthconfigeditor.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthconfigidedit.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthconfigselect.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthconfiguriedit.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsautheditorwidgets.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthidentitieseditor.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthimportcertdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthimportidentitydialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthmethodplugins.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthserverseditor.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthsslconfigwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthsslerrorsdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthsslimportdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthsslimporterrors.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthtrustedcasdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscredentialdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdetaileditemwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionbuilderdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionbuilder.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionselectiondialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgenericprojectionselectorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessagelogviewer.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessageviewer.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsowssourceselectbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsarcgisservicesourceselectbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprojectionselectorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsquerybuilderbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssqlcomposerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssublayersdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgstablewidgetuibase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgeonodesourceselectbase.h
)

IF(ENABLE_MODELTEST)
  INCLUDE_DIRECTORIES(../../tests/qt_modeltest)
ENDIF(ENABLE_MODELTEST)

INCLUDE_DIRECTORIES(SYSTEM
  ${QT_QTUITOOLS_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/symbology
  ${CMAKE_SOURCE_DIR}/src/gui/attributetable
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets/core
  ${CMAKE_SOURCE_DIR}/src/gui/layertree
  ${CMAKE_SOURCE_DIR}/src/gui/layout
  ${CMAKE_SOURCE_DIR}/src/gui/effects
  ${CMAKE_SOURCE_DIR}/src/gui/geonode
  ${CMAKE_SOURCE_DIR}/src/gui/ogr
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/annotations
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/composer
  ${CMAKE_SOURCE_DIR}/src/core/fieldformatter
  ${CMAKE_SOURCE_DIR}/src/core/geonode
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/locator
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/providers/memory
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/scalebar
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/native

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/native
)
INCLUDE_DIRECTORIES(SYSTEM
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
)

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

GENERATE_EXPORT_HEADER(
   qgis_gui
   BASE_NAME GUI
   EXPORT_FILE_NAME qgis_gui.h
)

SET(QGIS_GUI_HDRS ${QGIS_GUI_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_gui.h)

IF(NOT APPLE)
  INSTALL(FILES ${QGIS_GUI_HDRS} ${QGIS_GUI_MOC_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ELSE(NOT APPLE)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/native/mac)
  SET_TARGET_PROPERTIES(qgis_gui PROPERTIES
    CLEAN_DIRECT_OUTPUT 1
    FRAMEWORK 1
    FRAMEWORK_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}"
    MACOSX_FRAMEWORK_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/framework.info.plist.in"
    MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${COMPLETE_VERSION}
    MACOSX_FRAMEWORK_IDENTIFIER org.qgis.qgis3_gui
    BUILD_WITH_INSTALL_RPATH TRUE
    PUBLIC_HEADER "${QGIS_GUI_HDRS};${QGIS_GUI_MOC_HDRS}"
    LINK_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}"
  )
  # generated export header does not get copied with PUBLIC_HEADER files
  ADD_CUSTOM_COMMAND(TARGET qgis_gui
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy qgis_gui.h
            "${QGIS_OUTPUT_DIRECTORY}/${QGIS_LIB_SUBDIR}/qgis_gui.framework/Headers"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS qgis_gui.h
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
  qgis_native
  ${QT_QTUITOOLS_LIBRARY}
  ${QWT_LIBRARY}
  ${QSCINTILLA_LIBRARY}
)

IF(WIN32)
  ADD_DEFINITIONS(-DQWT_DLL)
  ADD_DEFINITIONS(-DQSCINTILLA_DLL)
ENDIF(WIN32)

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    qgis_gui PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

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
