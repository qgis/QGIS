SET(QGIS_GUI_SRCS
  raster/qgscolorrampshaderwidget.cpp
  raster/qgsmultibandcolorrendererwidget.cpp
  raster/qgspalettedrendererwidget.cpp
  raster/qgsrasterbandcombobox.cpp
  raster/qgsrastercontourrendererwidget.cpp
  raster/qgsrasterhistogramwidget.cpp
  raster/qgsrasterlayerproperties.cpp
  raster/qgsrasterlayertemporalpropertieswidget.cpp
  raster/qgsrasterminmaxwidget.cpp
  raster/qgsrasterrendererwidget.cpp
  raster/qgssinglebandgrayrendererwidget.cpp
  raster/qgssinglebandpseudocolorrendererwidget.cpp
  raster/qgsrendererrasterpropertieswidget.cpp
  raster/qgsrastertransparencywidget.cpp
  raster/qgshillshaderendererwidget.cpp
  raster/qgsresamplingutils.cpp

  vector/qgsattributeactiondialog.cpp
  vector/qgsattributeactionpropertiesdialog.cpp
  vector/qgsattributesforminitcode.cpp
  vector/qgsattributesformproperties.cpp
  vector/qgsdiagramproperties.cpp
  vector/qgsfieldcalculator.cpp
  vector/qgsjoindialog.cpp
  vector/qgssourcefieldsproperties.cpp
  vector/qgsvectorlayerlegendwidget.cpp
  vector/qgsvectorlayerloadstyledialog.cpp
  vector/qgsvectorlayerproperties.cpp
  vector/qgsvectorlayersavestyledialog.cpp
  vector/qgswmsdimensiondialog.cpp

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
  symbology/qgsmasksymbollayerwidget.cpp
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

  callouts/qgscalloutwidget.cpp

  attributeformconfig/qgsattributeformcontaineredit.cpp
  attributeformconfig/qgsattributerelationedit.cpp
  attributeformconfig/qgsattributetypedialog.cpp
  attributeformconfig/qgsattributewidgetedit.cpp

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
  attributetable/qgsfeaturefilterwidget.cpp

  auth/qgsauthauthoritieseditor.cpp
  auth/qgsauthcertificateinfo.cpp
  auth/qgsauthcertificatemanager.cpp
  auth/qgsauthcerttrustpolicycombobox.cpp
  auth/qgsauthconfigedit.cpp
  auth/qgsauthconfigeditor.cpp
  auth/qgsauthconfigidedit.cpp
  auth/qgsauthconfigselect.cpp
  auth/qgsauthsettingswidget.cpp
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

  devtools/qgsdevtoolwidget.cpp
  devtools/qgsdevtoolwidgetfactory.cpp

  editorwidgets/core/qgseditorconfigwidget.cpp
  editorwidgets/core/qgseditorwidgetautoconf.cpp
  editorwidgets/core/qgseditorwidgetfactory.cpp
  editorwidgets/core/qgseditorwidgetregistry.cpp
  editorwidgets/core/qgseditorwidgetwrapper.cpp
  editorwidgets/core/qgssearchwidgetwrapper.cpp
  editorwidgets/core/qgswidgetwrapper.cpp

  editorwidgets/qgsbinarywidgetfactory.cpp
  editorwidgets/qgsbinarywidgetwrapper.cpp
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
  editorwidgets/qgshtmlwidgetwrapper.cpp
  editorwidgets/qgskeyvaluewidgetfactory.cpp
  editorwidgets/qgskeyvaluewidgetwrapper.cpp
  editorwidgets/qgslistwidgetfactory.cpp
  editorwidgets/qgslistwidgetwrapper.cpp
  editorwidgets/qgsmultiedittoolbutton.cpp
  editorwidgets/qgsrangeconfigdlg.cpp
  editorwidgets/qgsrangewidgetwrapper.cpp
  editorwidgets/qgsrangewidgetfactory.cpp
  editorwidgets/qgsrelationaggregatesearchwidgetwrapper.cpp
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

  labeling/qgslabelengineconfigdialog.cpp
  labeling/qgslabelinggui.cpp
  labeling/qgslabelingwidget.cpp
  labeling/qgslabelobstaclesettingswidget.cpp
  labeling/qgslabelsettingswidgetbase.cpp
  labeling/qgsrulebasedlabelingwidget.cpp

  layertree/qgscustomlayerorderwidget.cpp
  layertree/qgslayertreeembeddedconfigwidget.cpp
  layertree/qgslayertreeembeddedwidgetregistry.cpp
  layertree/qgslayertreeembeddedwidgetsimpl.cpp
  layertree/qgslayertreemapcanvasbridge.cpp
  layertree/qgslayertreeview.cpp
  layertree/qgslayertreeviewdefaultactions.cpp
  layertree/qgslayertreeviewindicator.cpp
  layertree/qgslayertreeviewitemdelegate.cpp

  layout/qgsgeopdflayertreemodel.cpp
  layout/qgslayoutaddpagesdialog.cpp
  layout/qgslayoutguiutils.cpp
  layout/qgslayoutatlaswidget.cpp
  layout/qgslayoutattributeselectiondialog.cpp
  layout/qgslayoutattributetablewidget.cpp
  layout/qgslayoutcombobox.cpp
  layout/qgslayoutcustomdrophandler.cpp
  layout/qgslayoutguidewidget.cpp
  layout/qgslayouthtmlwidget.cpp
  layout/qgslayoutimageexportoptionsdialog.cpp
  layout/qgslayoutitemcombobox.cpp
  layout/qgslayoutitemguiregistry.cpp
  layout/qgslayoutitemslistview.cpp
  layout/qgslayoutitemwidget.cpp
  layout/qgslayoutlabelwidget.cpp
  layout/qgslayoutlegendlayersdialog.cpp
  layout/qgslayoutlegendwidget.cpp
  layout/qgslayoutmanualtablewidget.cpp
  layout/qgslayoutmapgridwidget.cpp
  layout/qgslayoutmapwidget.cpp
  layout/qgslayoutmarkerwidget.cpp
  layout/qgslayoutmousehandles.cpp
  layout/qgslayoutnewitempropertiesdialog.cpp
  layout/qgslayoutpagepropertieswidget.cpp
  layout/qgslayoutpdfexportoptionsdialog.cpp
  layout/qgslayoutpicturewidget.cpp
  layout/qgslayoutpolygonwidget.cpp
  layout/qgslayoutpolylinewidget.cpp
  layout/qgslayoutpropertieswidget.cpp
  layout/qgslayoutqptdrophandler.cpp
  layout/qgslayoutreportsectionlabel.cpp
  layout/qgslayoutruler.cpp
  layout/qgslayoutscalebarwidget.cpp
  layout/qgslayoutshapewidget.cpp
  layout/qgslayouttablebackgroundcolorsdialog.cpp
  layout/qgslayoutunitscombobox.cpp
  layout/qgslayoutvaliditychecks.cpp
  layout/qgslayoutview.cpp
  layout/qgslayoutviewmouseevent.cpp
  layout/qgslayoutviewrubberband.cpp
  layout/qgslayoutviewtool.cpp
  layout/qgslayoutviewtooladditem.cpp
  layout/qgslayoutviewtooladdnodeitem.cpp
  layout/qgslayoutviewtooleditnodes.cpp
  layout/qgslayoutviewtoolmoveitemcontent.cpp
  layout/qgslayoutviewtoolpan.cpp
  layout/qgslayoutviewtoolselect.cpp
  layout/qgslayoutviewtooltemporarykeypan.cpp
  layout/qgslayoutviewtooltemporarykeyzoom.cpp
  layout/qgslayoutviewtooltemporarymousepan.cpp
  layout/qgslayoutviewtoolzoom.cpp

  locator/qgslocatorwidget.cpp

  numericformats/qgsnumericformatselectorwidget.cpp
  numericformats/qgsnumericformatwidget.cpp
  numericformats/qgsnumericformatguiregistry.cpp

  ogr/qgsogrhelperfunctions.cpp
  ogr/qgsnewogrconnection.cpp
  ogr/qgsvectorlayersaveasdialog.cpp

  processing/qgsprocessingaggregatewidgets.cpp
  processing/qgsprocessingaggregatewidgetwrapper.cpp
  processing/qgsprocessingalgorithmconfigurationwidget.cpp
  processing/qgsprocessingalgorithmdialogbase.cpp
  processing/qgsprocessingconfigurationwidgets.cpp
  processing/qgsprocessingenummodelerwidget.cpp
  processing/qgsprocessingfeaturesourceoptionswidget.cpp
  processing/qgsprocessingfieldmapwidgetwrapper.cpp
  processing/qgsprocessingguiregistry.cpp
  processing/qgsprocessingmaplayercombobox.cpp
  processing/qgsprocessingmatrixmodelerwidget.cpp
  processing/qgsprocessingmatrixparameterdialog.cpp
  processing/qgsprocessingmodelerparameterwidget.cpp
  processing/qgsprocessingmultipleselectiondialog.cpp
  processing/qgsprocessingoutputdestinationwidget.cpp
  processing/qgsprocessingparameterdefinitionwidget.cpp
  processing/qgsprocessingparameterswidget.cpp
  processing/qgsprocessingrecentalgorithmlog.cpp
  processing/qgsprocessingtoolboxmodel.cpp
  processing/qgsprocessingtoolboxtreeview.cpp
  processing/qgsprocessingvectortilewriterlayerswidgetwrapper.cpp
  processing/qgsprocessingwidgetwrapper.cpp
  processing/qgsprocessingwidgetwrapperimpl.cpp

  processing/models/qgsmodelarrowitem.cpp
  processing/models/qgsmodelcomponentgraphicitem.cpp
  processing/models/qgsmodeldesignerdialog.cpp
  processing/models/qgsmodeldesignerinputstreewidget.cpp
  processing/models/qgsmodelgraphicitem.cpp
  processing/models/qgsmodelgraphicsscene.cpp
  processing/models/qgsmodelgraphicsview.cpp
  processing/models/qgsmodelgroupboxdefinitionwidget.cpp
  processing/models/qgsmodelinputreorderwidget.cpp
  processing/models/qgsmodelsnapper.cpp
  processing/models/qgsmodelundocommand.cpp
  processing/models/qgsmodelviewmouseevent.cpp
  processing/models/qgsmodelviewmousehandles.cpp
  processing/models/qgsmodelviewrubberband.cpp
  processing/models/qgsmodelviewtool.cpp
  processing/models/qgsmodelviewtoolselect.cpp
  processing/models/qgsmodelviewtooltemporarykeypan.cpp
  processing/models/qgsmodelviewtooltemporarykeyzoom.cpp
  processing/models/qgsmodelviewtooltemporarymousepan.cpp
  processing/models/qgsmodelviewtoolzoom.cpp

  providers/gdal/qgsgdalsourceselect.cpp
  providers/gdal/qgsgdalguiprovider.cpp

  providers/ogr/qgsogrguiprovider.cpp
  providers/ogr/qgsogrdbsourceselect.cpp
  providers/ogr/qgsogrdbtablemodel.cpp
  providers/ogr/qgsogrsourceselect.cpp
  providers/ogr/qgsgeopackageprojectstoragedialog.cpp
  providers/ogr/qgsgeopackageitemguiprovider.cpp
  providers/ogr/qgsogritemguiprovider.cpp
  providers/ogr/qgsgeopackageprojectstorageguiprovider.cpp

  tableeditor/qgstableeditordialog.cpp
  tableeditor/qgstableeditorformattingwidget.cpp
  tableeditor/qgstableeditorwidget.cpp

  vectortile/qgsvectortilebasiclabelingwidget.cpp
  vectortile/qgsvectortilebasicrendererwidget.cpp
  vectortile/qgsvectortileconnectiondialog.cpp
  vectortile/qgsvectortiledataitemguiprovider.cpp
  vectortile/qgsvectortileproviderguimetadata.cpp
  vectortile/qgsvectortilesourceselect.cpp

  qgisinterface.cpp
  qgsactionmenu.cpp
  qgsaddattrdialog.cpp
  qgsaddtaborgroup.cpp
  qgsadvanceddigitizingcanvasitem.cpp
  qgsadvanceddigitizingdockwidget.cpp
  qgsadvanceddigitizingfloater.cpp
  qgsaggregatetoolbutton.cpp
  qgsalignmentcombobox.cpp
  qgsattributedialog.cpp
  qgsattributeeditorcontext.cpp
  qgsattributeform.cpp
  qgsattributeformeditorwidget.cpp
  qgsattributeforminterface.cpp
  qgsattributeformlegacyinterface.cpp
  qgsattributetypeloaddialog.cpp
  qgsattributeformwidget.cpp
  qgsattributeformrelationeditorwidget.cpp
  qgsblendmodecombobox.cpp
  qgsbrowsertreeview.cpp
  qgsbrowserguimodel.cpp
  qgsbrowserdockwidget.cpp
  qgsbusyindicatordialog.cpp
  qgscharacterselectordialog.cpp
  qgscheckablecombobox.cpp
  qgscodeeditor.cpp
  qgscodeeditorcss.cpp
  qgscodeeditorhtml.cpp
  qgscodeeditorjs.cpp
  qgscodeeditorpython.cpp
  qgscodeeditorsql.cpp
  qgscodeeditorexpression.cpp
  qgscollapsiblegroupbox.cpp
  qgscolorbrewercolorrampdialog.cpp
  qgscolorbutton.cpp
  qgscolorrampbutton.cpp
  qgscolordialog.cpp
  qgscolorschemelist.cpp
  qgscolorswatchgrid.cpp
  qgscolorwidgets.cpp
  qgscompoundcolorwidget.cpp
  qgsconfigureshortcutsdialog.cpp
  qgscoordinateboundspreviewmapwidget.cpp
  qgscoordinateoperationwidget.cpp
  qgscredentialdialog.cpp
  qgscustomdrophandler.cpp
  qgscustomprojectopenhandler.cpp
  qgscurveeditorwidget.cpp
  qgsdatabaseschemacombobox.cpp
  qgsdatabasetablecombobox.cpp
  qgsdataitemguiprovider.cpp
  qgsdataitemguiproviderregistry.cpp
  qgsdatumtransformdialog.cpp
  qgsdatasourceselectdialog.cpp
  qgsdetaileditemdata.cpp
  qgsdetaileditemdelegate.cpp
  qgsdetaileditemwidget.cpp
  qgsdial.cpp
  qgsdialog.cpp
  qgsdoublevalidator.cpp
  qgsdockwidget.cpp
  qgsencodingfiledialog.cpp
  qgserrordialog.cpp
  qgsexpressionbuilderdialog.cpp
  qgsexpressionbuilderwidget.cpp
  qgsexpressionhighlighter.cpp
  qgsexpressionlineedit.cpp
  qgsexpressionpreviewwidget.cpp
  qgsexpressionselectiondialog.cpp
  qgsexpressionstoredialog.cpp
  qgsexpressiontreeview.cpp
  qgsextentgroupbox.cpp
  qgsextentwidget.cpp
  qgsexternalresourcewidget.cpp
  qgsfeatureselectiondlg.cpp
  qgsfieldcombobox.cpp
  qgsfieldexpressionwidget.cpp
  qgsfeaturepickerwidget.cpp
  qgsfieldmappingwidget.cpp
  qgsfieldmappingmodel.cpp
  qgsfeaturelistcombobox.cpp
  qgsfieldvalidator.cpp
  qgsfieldvalueslineedit.cpp
  qgsfilecontentsourcelineedit.cpp
  qgsfilewidget.cpp
  qgsfilterlineedit.cpp
  qgsfindfilesbypatternwidget.cpp
  qgsfloatingwidget.cpp
  qgsfocuskeeper.cpp
  qgsfocuswatcher.cpp
  qgsfontbutton.cpp
  qgsformannotation.cpp
  qgsgeometryrubberband.cpp
  qgsgradientcolorrampdialog.cpp
  qgsgradientstopeditor.cpp
  qgsgraphicsviewmousehandles.cpp
  qgsgroupwmsdatadialog.cpp
  qgsgui.cpp
  qgsguiutils.cpp
  qgshighlight.cpp
  qgshighlightablecombobox.cpp
  qgshighlightablelineedit.cpp
  qgshistogramwidget.cpp
  qgshelp.cpp
  qgsidentifymenu.cpp
  qgsinstallgridshiftdialog.cpp
  qgskeyvaluewidget.cpp
  qgslistwidget.cpp
  qgslegendfilterbutton.cpp
  qgslegendpatchshapebutton.cpp
  qgslegendpatchshapewidget.cpp
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
  qgsmapcanvasutils.cpp
  qgsmaplayeractionregistry.cpp
  qgsmaplayercombobox.cpp
  qgsmaplayerconfigwidgetfactory.cpp
  qgsmaplayerstylecategoriesmodel.cpp
  qgsmaplayerstyleguiutils.cpp
  qgsmaplayerstylemanagerwidget.cpp
  qgsmapmouseevent.cpp
  qgsmapoverviewcanvas.cpp
  qgsmaskingwidget.cpp
  qgsmaptip.cpp
  qgsmaptool.cpp
  qgsmaptooladvanceddigitizing.cpp
  qgsmaptoolcapture.cpp
  qgsmaptooledit.cpp
  qgsmaptooldigitizefeature.cpp
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
  qgsmetadatawidget.cpp
  qgsmasksourceselectionwidget.cpp
  qgsnewauxiliarylayerdialog.cpp
  qgsnewauxiliaryfielddialog.cpp
  qgsnewdatabasetablenamewidget.cpp
  qgsnewhttpconnection.cpp
  qgsnewmemorylayerdialog.cpp
  qgsnewnamedialog.cpp
  qgsnewvectorlayerdialog.cpp
  qgsnewgeopackagelayerdialog.cpp
  qgsnewvectortabledialog.cpp
  qgsopacitywidget.cpp
  qgsoptionsdialogbase.cpp
  qgsoptionsdialoghighlightwidget.cpp
  qgsoptionsdialoghighlightwidgetsimpl.cpp
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
  qgsproviderguiregistry.cpp
  qgsproviderguimetadata.cpp
  qgsprojectstorageguiregistry.cpp
  qgsproviderconnectioncombobox.cpp
  qgsproxystyle.cpp
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
  qgssnapindicator.cpp
  qgssnaptogridcanvasitem.cpp
  qgsstyleitemslistwidget.cpp
  qgssublayersdialog.cpp
  qgssubstitutionlistwidget.cpp
  qgssqlcomposerdialog.cpp
  qgsstatusbar.cpp
  qgssymbolbutton.cpp
  qgssymbollayerselectionwidget.cpp
  qgstablewidgetbase.cpp
  qgstabwidget.cpp
  qgstablewidgetitem.cpp
  qgstaskmanagerwidget.cpp
  qgstemporalmapsettingswidget.cpp
  qgstemporalcontrollerwidget.cpp
  qgstextformatwidget.cpp
  qgstextpreview.cpp
  qgstreewidgetitem.cpp
  qgsunitselectionwidget.cpp
  qgsuserinputwidget.cpp
  qgsvaliditycheckresultswidget.cpp
  qgsvariableeditorwidget.cpp
  qgsvectorlayertemporalpropertieswidget.cpp
  qgsvertexmarker.cpp
  qgsfiledownloaderdialog.cpp
  qgsvscrollarea.cpp
  qgsdatasourcemanagerdialog.cpp
  qgsabstractdatasourcewidget.cpp
  qgswidgetstatehelper_p.cpp
  qgssourceselectprovider.cpp
  qgssourceselectproviderregistry.cpp
)

SET(QGIS_GUI_HDRS
  qgisinterface.h

  qgsabstractdatasourcewidget.h
  qgsactionmenu.h
  qgsaddattrdialog.h
  qgsaddtaborgroup.h
  qgsadvanceddigitizingdockwidget.h
  qgsadvanceddigitizingfloater.h
  qgsaggregatetoolbutton.h
  qgsalignmentcombobox.h
  qgsattributedialog.h
  qgsattributeeditorcontext.h
  qgsattributeform.h
  qgsattributeformeditorwidget.h
  qgsattributeforminterface.h
  qgsattributeformlegacyinterface.h
  qgsattributeformrelationeditorwidget.h
  qgsattributeformwidget.h
  qgsattributetypeloaddialog.h
  qgsblendmodecombobox.h
  qgsbrowserdockwidget.h
  qgsbrowserguimodel.h
  qgsbrowsertreeview.h
  qgsbusyindicatordialog.h
  qgscharacterselectordialog.h
  qgscheckablecombobox.h
  qgscodeeditor.h
  qgscodeeditorcss.h
  qgscodeeditorexpression.h
  qgscodeeditorhtml.h
  qgscodeeditorjs.h
  qgscodeeditorpython.h
  qgscodeeditorsql.h
  qgscollapsiblegroupbox.h
  qgscolorbrewercolorrampdialog.h
  qgscolorbutton.h
  qgscolordialog.h
  qgscolorrampbutton.h
  qgscolorschemelist.h
  qgscolorswatchgrid.h
  qgscolorwidgets.h
  qgscompoundcolorwidget.h
  qgsconfigureshortcutsdialog.h
  qgscoordinateboundspreviewmapwidget.h
  qgscoordinateoperationwidget.h
  qgscredentialdialog.h
  qgscurveeditorwidget.h
  qgscustomdrophandler.h
  qgscustomprojectopenhandler.h
  qgsdatabaseschemacombobox.h
  qgsdatabasetablecombobox.h
  qgsdataitemguiprovider.h
  qgsdataitemguiproviderregistry.h
  qgsdatasourcemanagerdialog.h
  qgsdatasourceselectdialog.h
  qgsnewdatabasetablenamewidget.h
  qgsdatumtransformdialog.h
  qgsdetaileditemdata.h
  qgsdetaileditemdelegate.h
  qgsdetaileditemwidget.h
  qgsdial.h
  qgsdialog.h
  qgsdockwidget.h
  qgsdoublevalidator.h
  qgsencodingfiledialog.h
  qgserrordialog.h
  qgsexpressionbuilderdialog.h
  qgsexpressionstoredialog.h
  qgsexpressionbuilderwidget.h
  qgsexpressionhighlighter.h
  qgsexpressionlineedit.h
  qgsexpressionpreviewwidget.h
  qgsexpressionselectiondialog.h
  qgsexpressiontreeview.h
  qgsextentgroupbox.h
  qgsextentwidget.h
  qgsexternalresourcewidget.h
  qgsfeaturepickerwidget.h
  qgsfeaturelistcombobox.h
  qgsfeatureselectiondlg.h
  qgsfieldcombobox.h
  qgsfieldexpressionwidget.h
  qgsfieldmappingwidget.h
  qgsfieldmappingmodel.h
  qgsfieldvalidator.h
  qgsfieldvalueslineedit.h
  qgsfilecontentsourcelineedit.h
  qgsfiledownloaderdialog.h
  qgsfilewidget.h
  qgsfilterlineedit.h
  qgsfindfilesbypatternwidget.h
  qgsfloatingwidget.h
  qgsfocuswatcher.h
  qgsfontbutton.h
  qgsformannotation.h
  qgsgeometryrubberband.h
  qgsgradientcolorrampdialog.h
  qgsgradientstopeditor.h
  qgsgraphicsviewmousehandles.h
  qgsgroupwmsdatadialog.h
  qgsgui.h
  qgsguiutils.h
  qgshelp.h
  qgshighlight.h
  qgshighlightablecombobox.h
  qgshighlightablelineedit.h
  qgshistogramwidget.h
  qgsidentifymenu.h
  qgsinstallgridshiftdialog.h
  qgskeyvaluewidget.h
  qgslegendfilterbutton.h
  qgslegendpatchshapebutton.h
  qgslegendpatchshapewidget.h
  qgslimitedrandomcolorrampdialog.h
  qgslistwidget.h
  qgslonglongvalidator.h
  qgsludialog.h
  qgsmanageconnectionsdialog.h
  qgsmapcanvas.h
  qgsmapcanvasannotationitem.h
  qgsmapcanvasinteractionblocker.h
  qgsmapcanvasitem.h
  qgsmapcanvasmap.h
  qgsmapcanvassnappingutils.h
  qgsmapcanvastracer.h
  qgsmapcanvasutils.h
  qgsmaplayeractionregistry.h
  qgsmaplayercombobox.h
  qgsmaplayerconfigwidget.h
  qgsmaplayerconfigwidgetfactory.h
  qgsmaplayerstylecategoriesmodel.h
  qgsmaplayerstyleguiutils.h
  qgsmaplayerstylemanagerwidget.h
  qgsmapmouseevent.h
  qgsmapoverviewcanvas.h
  qgsmaptip.h
  qgsmaptool.h
  qgsmaptooladvanceddigitizing.h
  qgsmaptoolcapture.h
  qgsmaptooldigitizefeature.h
  qgsmaptooledit.h
  qgsmaptoolemitpoint.h
  qgsmaptoolextent.h
  qgsmaptoolidentify.h
  qgsmaptoolidentifyfeature.h
  qgsmaptoolpan.h
  qgsmaptoolzoom.h
  qgsmaskingwidget.h
  qgsmenuheader.h
  qgsmessagebar.h
  qgsmessagebaritem.h
  qgsmessagelogviewer.h
  qgsmessageviewer.h
  qgsmetadatawidget.h
  qgsmasksourceselectionwidget.h
  qgsnewauxiliaryfielddialog.h
  qgsnewauxiliarylayerdialog.h
  qgsnewgeopackagelayerdialog.h
  qgsnewvectortabledialog.h
  qgsnewhttpconnection.h
  qgsnewmemorylayerdialog.h
  qgsnewnamedialog.h
  qgsnewvectorlayerdialog.h
  qgsnewvectortabledialog.h
  qgsopacitywidget.h
  qgsoptionsdialogbase.h
  qgsoptionsdialoghighlightwidget.h
  qgsoptionsdialoghighlightwidgetsimpl.h
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
  qgsprojectionselectiontreewidget.h
  qgsprojectionselectionwidget.h
  qgsprojectstorageguiprovider.h
  qgsprojectstorageguiregistry.h
  qgspropertyassistantwidget.h
  qgspropertyoverridebutton.h
  qgsproviderconnectioncombobox.h
  qgsproviderguimetadata.h
  qgsproviderguiregistry.h
  qgsproxystyle.h
  qgsquerybuilder.h
  qgsrasterformatsaveoptionswidget.h
  qgsrasterlayersaveasdialog.h
  qgsrasterpyramidsoptionswidget.h
  qgsratiolockbutton.h
  qgsrelationeditorwidget.h
  qgsrubberband.h
  qgsscalecombobox.h
  qgsscalerangewidget.h
  qgsscalevisibilitydialog.h
  qgsscalewidget.h
  qgsscrollarea.h
  qgssearchquerybuilder.h
  qgsshortcutsmanager.h
  qgsslider.h
  qgssnapindicator.h
  qgssnaptogridcanvasitem.h
  qgssourceselectprovider.h
  qgssourceselectproviderregistry.h
  qgssqlcomposerdialog.h
  qgsstatusbar.h
  qgsstyleitemslistwidget.h
  qgssublayersdialog.h
  qgssubstitutionlistwidget.h
  qgssymbolbutton.h
  qgssymbollayerselectionwidget.h
  qgstablewidgetbase.h
  qgstablewidgetitem.h
  qgstabwidget.h
  qgstaskmanagerwidget.h
  qgstemporalmapsettingswidget.h
  qgstemporalcontrollerwidget.h
  qgstextformatwidget.h
  qgstextpreview.h
  qgstreewidgetitem.h
  qgsunitselectionwidget.h
  qgsuserinputwidget.h
  qgsvaliditycheckresultswidget.h
  qgsvariableeditorwidget.h
  qgsvectorlayertemporalpropertieswidget.h
  qgsvertexmarker.h
  qgsvscrollarea.h
  qgswindowmanagerinterface.h

  attributeformconfig/qgsattributeformcontaineredit.h
  attributeformconfig/qgsattributerelationedit.h
  attributeformconfig/qgsattributetypedialog.h
  attributeformconfig/qgsattributewidgetedit.h

  attributetable/qgsattributetabledelegate.h
  attributetable/qgsattributetablefiltermodel.h
  attributetable/qgsattributetablemodel.h
  attributetable/qgsattributetableview.h
  attributetable/qgsdualview.h
  attributetable/qgsfeaturelistmodel.h
  attributetable/qgsfeaturelistview.h
  attributetable/qgsfeaturelistviewdelegate.h
  attributetable/qgsfeaturemodel.h
  attributetable/qgsfeatureselectionmodel.h
  attributetable/qgsfieldconditionalformatwidget.h
  attributetable/qgsgenericfeatureselectionmanager.h
  attributetable/qgsifeatureselectionmanager.h
  attributetable/qgsorganizetablecolumnsdialog.h
  attributetable/qgsvectorlayerselectionmanager.h

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
  auth/qgsauthsettingswidget.h
  auth/qgsauthsslconfigwidget.h
  auth/qgsauthsslerrorsdialog.h
  auth/qgsauthsslimportdialog.h
  auth/qgsauthtrustedcasdialog.h

  callouts/qgscalloutwidget.h

  devtools/qgsdevtoolwidget.h
  devtools/qgsdevtoolwidgetfactory.h

  editorwidgets/core/qgseditorconfigwidget.h
  editorwidgets/core/qgseditorwidgetautoconf.h
  editorwidgets/core/qgseditorwidgetfactory.h
  editorwidgets/core/qgseditorwidgetregistry.h
  editorwidgets/core/qgseditorwidgetwrapper.h
  editorwidgets/core/qgssearchwidgetwrapper.h
  editorwidgets/core/qgswidgetwrapper.h
  editorwidgets/qgsbinarywidgetfactory.h
  editorwidgets/qgsbinarywidgetwrapper.h
  editorwidgets/qgscheckboxconfigdlg.h
  editorwidgets/qgscheckboxsearchwidgetwrapper.h
  editorwidgets/qgscheckboxwidgetfactory.h
  editorwidgets/qgscheckboxwidgetwrapper.h
  editorwidgets/qgsclassificationwidgetwrapper.h
  editorwidgets/qgsclassificationwidgetwrapperfactory.h
  editorwidgets/qgscolorwidgetfactory.h
  editorwidgets/qgscolorwidgetwrapper.h
  editorwidgets/qgsdatetimeedit.h
  editorwidgets/qgsdatetimeeditconfig.h
  editorwidgets/qgsdatetimeeditfactory.h
  editorwidgets/qgsdatetimeeditwrapper.h
  editorwidgets/qgsdatetimesearchwidgetwrapper.h
  editorwidgets/qgsdefaultsearchwidgetwrapper.h
  editorwidgets/qgsdoublespinbox.h
  editorwidgets/qgsdummyconfigdlg.h
  editorwidgets/qgsenumerationwidgetfactory.h
  editorwidgets/qgsenumerationwidgetwrapper.h
  editorwidgets/qgsexternalresourceconfigdlg.h
  editorwidgets/qgsexternalresourcewidgetfactory.h
  editorwidgets/qgsexternalresourcewidgetwrapper.h
  editorwidgets/qgshiddenwidgetfactory.h
  editorwidgets/qgshiddenwidgetwrapper.h
  editorwidgets/qgshtmlwidgetwrapper.h
  editorwidgets/qgskeyvaluewidgetfactory.h
  editorwidgets/qgskeyvaluewidgetwrapper.h
  editorwidgets/qgslistwidgetfactory.h
  editorwidgets/qgslistwidgetwrapper.h
  editorwidgets/qgsmultiedittoolbutton.h
  editorwidgets/qgsrangeconfigdlg.h
  editorwidgets/qgsrangewidgetfactory.h
  editorwidgets/qgsrangewidgetwrapper.h
  editorwidgets/qgsrelationaggregatesearchwidgetwrapper.h
  editorwidgets/qgsrelationreferenceconfigdlg.h
  editorwidgets/qgsrelationreferencefactory.h
  editorwidgets/qgsrelationreferencesearchwidgetwrapper.h
  editorwidgets/qgsrelationreferencewidget.h
  editorwidgets/qgsrelationreferencewidgetwrapper.h
  editorwidgets/qgsrelationwidgetwrapper.h
  editorwidgets/qgssearchwidgettoolbutton.h
  editorwidgets/qgsspinbox.h
  editorwidgets/qgstexteditconfigdlg.h
  editorwidgets/qgstexteditsearchwidgetwrapper.h
  editorwidgets/qgstexteditwidgetfactory.h
  editorwidgets/qgstexteditwrapper.h
  editorwidgets/qgsuniquevaluesconfigdlg.h
  editorwidgets/qgsuniquevaluewidgetfactory.h
  editorwidgets/qgsuniquevaluewidgetwrapper.h
  editorwidgets/qgsuuidwidgetfactory.h
  editorwidgets/qgsuuidwidgetwrapper.h
  editorwidgets/qgsvaluemapconfigdlg.h
  editorwidgets/qgsvaluemapsearchwidgetwrapper.h
  editorwidgets/qgsvaluemapwidgetfactory.h
  editorwidgets/qgsvaluemapwidgetwrapper.h
  editorwidgets/qgsvaluerelationconfigdlg.h
  editorwidgets/qgsvaluerelationsearchwidgetwrapper.h
  editorwidgets/qgsvaluerelationwidgetfactory.h
  editorwidgets/qgsvaluerelationwidgetwrapper.h

  effects/qgseffectdrawmodecombobox.h
  effects/qgseffectstackpropertieswidget.h
  effects/qgspainteffectpropertieswidget.h
  effects/qgspainteffectwidget.h

  labeling/qgslabelengineconfigdialog.h
  labeling/qgslabelinggui.h
  labeling/qgslabelingwidget.h
  labeling/qgslabelobstaclesettingswidget.h
  labeling/qgslabelsettingswidgetbase.h
  labeling/qgsrulebasedlabelingwidget.h

  layertree/qgscustomlayerorderwidget.h
  layertree/qgslayertreeembeddedconfigwidget.h
  layertree/qgslayertreeembeddedwidgetregistry.h
  layertree/qgslayertreeembeddedwidgetsimpl.h
  layertree/qgslayertreemapcanvasbridge.h
  layertree/qgslayertreeview.h
  layertree/qgslayertreeviewdefaultactions.h
  layertree/qgslayertreeviewindicator.h
  layertree/qgslayertreeviewitemdelegate.h

  layout/qgsgeopdflayertreemodel.h
  layout/qgslayoutaddpagesdialog.h
  layout/qgslayoutguiutils.h
  layout/qgslayoutatlaswidget.h
  layout/qgslayoutattributeselectiondialog.h
  layout/qgslayoutattributetablewidget.h
  layout/qgslayoutcombobox.h
  layout/qgslayoutcustomdrophandler.h
  layout/qgslayoutdesignerinterface.h
  layout/qgslayoutguidewidget.h
  layout/qgslayouthtmlwidget.h
  layout/qgslayoutimageexportoptionsdialog.h
  layout/qgslayoutitemcombobox.h
  layout/qgslayoutitemguiregistry.h
  layout/qgslayoutitemslistview.h
  layout/qgslayoutitemwidget.h
  layout/qgslayoutlabelwidget.h
  layout/qgslayoutlegendlayersdialog.h
  layout/qgslayoutlegendwidget.h
  layout/qgslayoutmanualtablewidget.h
  layout/qgslayoutmapgridwidget.h
  layout/qgslayoutmapwidget.h
  layout/qgslayoutmarkerwidget.h
  layout/qgslayoutmousehandles.h
  layout/qgslayoutnewitempropertiesdialog.h
  layout/qgslayoutpagepropertieswidget.h
  layout/qgslayoutpdfexportoptionsdialog.h
  layout/qgslayoutpicturewidget.h
  layout/qgslayoutpolygonwidget.h
  layout/qgslayoutpolylinewidget.h
  layout/qgslayoutpropertieswidget.h
  layout/qgslayoutqptdrophandler.h
  layout/qgslayoutreportsectionlabel.h
  layout/qgslayoutruler.h
  layout/qgslayoutscalebarwidget.h
  layout/qgslayoutshapewidget.h
  layout/qgslayouttablebackgroundcolorsdialog.h
  layout/qgslayoutunitscombobox.h
  layout/qgslayoutview.h
  layout/qgslayoutviewmouseevent.h
  layout/qgslayoutviewrubberband.h
  layout/qgslayoutviewtool.h
  layout/qgslayoutviewtooladditem.h
  layout/qgslayoutviewtooladdnodeitem.h
  layout/qgslayoutviewtooleditnodes.h
  layout/qgslayoutviewtoolmoveitemcontent.h
  layout/qgslayoutviewtoolpan.h
  layout/qgslayoutviewtoolselect.h
  layout/qgslayoutviewtooltemporarykeypan.h
  layout/qgslayoutviewtooltemporarykeyzoom.h
  layout/qgslayoutviewtooltemporarymousepan.h
  layout/qgslayoutviewtoolzoom.h

  locator/qgslocatorwidget.h

  numericformats/qgsnumericformatselectorwidget.h
  numericformats/qgsnumericformatwidget.h
  numericformats/qgsnumericformatguiregistry.h

  ogr/qgsnewogrconnection.h
  ogr/qgsogrhelperfunctions.h
  ogr/qgsvectorlayersaveasdialog.h

  processing/qgsprocessingaggregatewidgets.h
  processing/qgsprocessingaggregatewidgetwrapper.h
  processing/qgsprocessingalgorithmconfigurationwidget.h
  processing/qgsprocessingalgorithmdialogbase.h
  processing/qgsprocessingconfigurationwidgets.h
  processing/qgsprocessingenummodelerwidget.h
  processing/qgsprocessingfeaturesourceoptionswidget.h
  processing/qgsprocessingfieldmapwidgetwrapper.h
  processing/qgsprocessinggui.h
  processing/qgsprocessingguiregistry.h
  processing/qgsprocessingmaplayercombobox.h
  processing/qgsprocessingmatrixmodelerwidget.h
  processing/qgsprocessingmatrixparameterdialog.h
  processing/qgsprocessingmodelerparameterwidget.h
  processing/qgsprocessingmultipleselectiondialog.h
  processing/qgsprocessingoutputdestinationwidget.h
  processing/qgsprocessingparameterdefinitionwidget.h
  processing/qgsprocessingparameterswidget.h
  processing/qgsprocessingrecentalgorithmlog.h
  processing/qgsprocessingtoolboxmodel.h
  processing/qgsprocessingtoolboxtreeview.h
  processing/qgsprocessingvectortilewriterlayerswidgetwrapper.h
  processing/qgsprocessingwidgetwrapper.h
  processing/qgsprocessingwidgetwrapperimpl.h

  processing/models/qgsmodelarrowitem.h
  processing/models/qgsmodelcomponentgraphicitem.h
  processing/models/qgsmodeldesignerdialog.h
  processing/models/qgsmodeldesignerinputstreewidget.h
  processing/models/qgsmodelgraphicitem.h
  processing/models/qgsmodelgraphicsscene.h
  processing/models/qgsmodelgraphicsview.h
  processing/models/qgsmodelgroupboxdefinitionwidget.h
  processing/models/qgsmodelinputreorderwidget.h
  processing/models/qgsmodelsnapper.h
  processing/models/qgsmodelundocommand.h
  processing/models/qgsmodelviewmouseevent.h
  processing/models/qgsmodelviewmousehandles.h
  processing/models/qgsmodelviewrubberband.h
  processing/models/qgsmodelviewtool.h
  processing/models/qgsmodelviewtoolselect.h
  processing/models/qgsmodelviewtooltemporarykeypan.h
  processing/models/qgsmodelviewtooltemporarykeyzoom.h
  processing/models/qgsmodelviewtooltemporarymousepan.h
  processing/models/qgsmodelviewtoolzoom.h

  providers/gdal/qgsgdalguiprovider.h
  providers/gdal/qgsgdalsourceselect.h
  providers/ogr/qgsgeopackageitemguiprovider.h
  providers/ogr/qgsgeopackageprojectstoragedialog.h
  providers/ogr/qgsgeopackageprojectstorageguiprovider.h
  providers/ogr/qgsogrdbsourceselect.h
  providers/ogr/qgsogrdbtablemodel.h
  providers/ogr/qgsogrguiprovider.h
  providers/ogr/qgsogritemguiprovider.h
  providers/ogr/qgsogrsourceselect.h

  raster/qgscolorrampshaderwidget.h
  raster/qgshillshaderendererwidget.h
  raster/qgsmultibandcolorrendererwidget.h
  raster/qgspalettedrendererwidget.h
  raster/qgsrasterbandcombobox.h
  raster/qgsrastercontourrendererwidget.h
  raster/qgsrasterhistogramwidget.h
  raster/qgsrasterminmaxwidget.h
  raster/qgsrasterrendererwidget.h
  raster/qgsrastertransparencywidget.h
  raster/qgsrendererrasterpropertieswidget.h
  raster/qgssinglebandgrayrendererwidget.h
  raster/qgssinglebandpseudocolorrendererwidget.h
  raster/qgsrasterlayerproperties.h
  raster/qgsrasterlayertemporalpropertieswidget.h
  raster/qgsresamplingutils.h

  vector/qgsattributeactiondialog.h
  vector/qgsattributeactionpropertiesdialog.h
  vector/qgsattributesforminitcode.h
  vector/qgsattributesformproperties.h
  vector/qgsdiagramproperties.h
  vector/qgsfieldcalculator.h
  vector/qgsjoindialog.h
  vector/qgssourcefieldsproperties.h
  vector/qgsvectorlayerlegendwidget.h
  vector/qgsvectorlayerloadstyledialog.h
  vector/qgsvectorlayerproperties.h
  vector/qgsvectorlayersavestyledialog.h
  vector/qgswmsdimensiondialog.h

  symbology/characterwidget.h
  symbology/qgs25drendererwidget.h
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
  symbology/qgsmasksymbollayerwidget.h
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
  symbology/qgssymbolselectordialog.h
  symbology/qgssymbolslistwidget.h
  symbology/qgssymbolwidgetcontext.h
  symbology/qgsvectorfieldsymbollayerwidget.h

  tableeditor/qgstableeditordialog.h
  tableeditor/qgstableeditorformattingwidget.h
  tableeditor/qgstableeditorwidget.h

  vectortile/qgsvectortilebasiclabelingwidget.h
  vectortile/qgsvectortilebasicrendererwidget.h
  vectortile/qgsvectortileconnectiondialog.h
  vectortile/qgsvectortiledataitemguiprovider.h
  vectortile/qgsvectortileproviderguimetadata.h
  vectortile/qgsvectortilesourceselect.h

  qgsbrowserdockwidget_p.h
)

SET(QGIS_GUI_PRIVATE_HDRS
  qgsbrowserdockwidget_p.cpp
  qgswidgetstatehelper_p.h
  attributetable/qgsfeaturefilterwidget_p.h
)

FIND_PACKAGE(Qt5 COMPONENTS Qml QuickWidgets REQUIRED)

IF(Qt5Qml_FOUND)
ADD_DEFINITIONS(-DWITH_QML)
SET(QGIS_GUI_HDRS
  ${QGIS_GUI_HDRS}
  editorwidgets/qgsqmlwidgetwrapper.h
)
ENDIF(Qt5Qml_FOUND)

# for installing headers

IF(Qt5Qml_FOUND)

SET(QGIS_GUI_SRCS
  ${QGIS_GUI_SRCS}
  editorwidgets/qgsqmlwidgetwrapper.cpp
)
ENDIF(Qt5Qml_FOUND)

SET_PROPERTY(GLOBAL PROPERTY QGIS_GUI_HDRS ${QGIS_GUI_HDRS})


SET(QGIS_GUI_UI_HDRS
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsadvanceddigitizingdockwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsadvanceddigitizingfloaterbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthauthoritieseditor.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthcertificateinfo.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthcertificatemanager.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthconfigedit.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthconfigeditor.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthconfigidedit.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthconfigselect.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthsettingswidget.h
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
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionstoredialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionbuilder.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionselectiondialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsfeaturefilterwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgenericprojectionselectorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessagelogviewer.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessageviewer.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsnewvectortabledialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsowssourceselectbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsarcgisservicesourceselectbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprojectionselectorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrasterlayerpropertiesbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsquerybuilderbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssqlcomposerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssublayersdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgstablewidgetuibase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrendererpropsdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgseffectstackpropertieswidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslayoutnewpagedialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslayoutguidewidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslayoutimageexportoptions.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslayoutitemwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslayoutpagepropertieswidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgspdfexportoptions.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslayoutwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrenderercontainerbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsvectorlayerpropertiesbase.h
)

IF(ENABLE_MODELTEST)
  INCLUDE_DIRECTORIES(../../tests/qt_modeltest)
ENDIF(ENABLE_MODELTEST)

INCLUDE_DIRECTORIES(SYSTEM
  ${QT_QTUITOOLS_INCLUDE_DIR}
)
INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/attributeformconfig
  ${CMAKE_SOURCE_DIR}/src/gui/symbology
  ${CMAKE_SOURCE_DIR}/src/gui/attributetable
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/gui/devtools
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets/core
  ${CMAKE_SOURCE_DIR}/src/gui/effects
  ${CMAKE_SOURCE_DIR}/src/gui/labeling
  ${CMAKE_SOURCE_DIR}/src/gui/layertree
  ${CMAKE_SOURCE_DIR}/src/gui/layout
  ${CMAKE_SOURCE_DIR}/src/gui/numericformats
  ${CMAKE_SOURCE_DIR}/src/gui/ogr
  ${CMAKE_SOURCE_DIR}/src/gui/processing
  ${CMAKE_SOURCE_DIR}/src/gui/processing/models
  ${CMAKE_SOURCE_DIR}/src/gui/providers/gdal
  ${CMAKE_SOURCE_DIR}/src/gui/providers/ogr
  ${CMAKE_SOURCE_DIR}/src/gui/raster
  ${CMAKE_SOURCE_DIR}/src/gui/vector
  ${CMAKE_SOURCE_DIR}/src/gui/vectortile
  ${CMAKE_SOURCE_DIR}/src/gui/tableeditor
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/annotations
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/callouts
  ${CMAKE_SOURCE_DIR}/src/core/fieldformatter
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/labeling
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/locator
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/numericformats
  ${CMAKE_SOURCE_DIR}/src/core/processing
  ${CMAKE_SOURCE_DIR}/src/core/processing/models
  ${CMAKE_SOURCE_DIR}/src/core/mesh
  ${CMAKE_SOURCE_DIR}/src/core/pal
  ${CMAKE_SOURCE_DIR}/src/core/providers/memory
  ${CMAKE_SOURCE_DIR}/src/core/providers/gdal
  ${CMAKE_SOURCE_DIR}/src/core/providers/ogr
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/scalebar
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/classification
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/textrenderer
  ${CMAKE_SOURCE_DIR}/src/core/validity
  ${CMAKE_SOURCE_DIR}/src/core/vectortile
  ${CMAKE_SOURCE_DIR}/src/native
  ${CMAKE_SOURCE_DIR}/external
  ${CMAKE_SOURCE_DIR}/external/nlohmann

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/native
)
INCLUDE_DIRECTORIES(SYSTEM
  ${PROJ_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
  ${SPATIALINDEX_INCLUDE_DIR}
  ${Protobuf_INCLUDE_DIRS}
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
  ${QGIS_GUI_SRCS} ${QGIS_GUI_HDRS} ${QGIS_GUI_PRIVATE_HDRS}
)

GENERATE_EXPORT_HEADER(
   qgis_gui
   BASE_NAME GUI
   EXPORT_FILE_NAME qgis_gui.h
)

SET(QGIS_GUI_HDRS ${QGIS_GUI_HDRS} ${CMAKE_CURRENT_BINARY_DIR}/qgis_gui.h)

IF(NOT APPLE)
  IF (WIN32 )
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/native/win)
  ENDIF (WIN32)
  IF (UNIX)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/native/linux)
  ENDIF(UNIX)
  INSTALL(FILES ${QGIS_GUI_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
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
    PUBLIC_HEADER "${QGIS_GUI_HDRS}"
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
  ${Qt5UiTools_LIBRARIES}
  ${QWT_LIBRARY}
  ${QSCINTILLA_LIBRARY}
  ${Qt5QuickWidgets_LIBRARIES}
)

IF (FORCE_STATIC_PROVIDERS)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src/providers/wms
    ${CMAKE_SOURCE_DIR}/src/providers/postgres
  )
  TARGET_LINK_LIBRARIES(qgis_gui
    wmsprovider_gui_a
    postgresprovider_gui_a
  )
ENDIF (FORCE_STATIC_PROVIDERS)

IF(ENABLE_MODELTEST)
  TARGET_LINK_LIBRARIES(qgis_gui ${Qt5Test_LIBRARIES})
ENDIF(ENABLE_MODELTEST)

TARGET_COMPILE_DEFINITIONS(qgis_gui PRIVATE "-DQT_NO_FOREACH")

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
