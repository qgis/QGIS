ADD_SUBDIRECTORY(../../external/libdxfrw dwg/libdxfrw)
IF(NOT WIN32)
SUBDIRS(${CMAKE_SOURCE_DIR}/external/qt-unix-signals)
ENDIF(NOT WIN32)

SET(QGIS_APP_SRCS
  ${CMAKE_SOURCE_DIR}/external/nmea/gmath.c
  ${CMAKE_SOURCE_DIR}/external/nmea/time.c

  qgisapp.cpp
  qgisappinterface.cpp
  qgisappstylesheet.cpp
  qgsabout.cpp
  qgsalignrasterdialog.cpp
  qgsappauthrequesthandler.cpp
  qgsappbrowserproviders.cpp
  qgsappcoordinateoperationhandlers.cpp
  qgsapplayertreeviewmenuprovider.cpp
  qgsappwindowmanager.cpp
  qgsaddattrdialog.cpp
  qgsaddtaborgroup.cpp
  qgsappscreenshots.cpp
  qgsjoindialog.cpp
  qgswmsdimensiondialog.cpp
  qgsannotationwidget.cpp
  qgsappsslerrorhandler.cpp
  qgsattributeactiondialog.cpp
  qgsattributeactionpropertiesdialog.cpp
  qgsattributetypedialog.cpp
  qgsattributerelationedit.cpp
  qgsattributesforminitcode.cpp
  qgsattributetabledialog.cpp
  qgsbookmarks.cpp
  qgsbookmarkeditordialog.cpp
  qgsclipboard.cpp
  qgscustomization.cpp
  qgscustomprojectiondialog.cpp
  qgsdatumtransformtablewidget.cpp
  qgsdiscoverrelationsdialog.cpp
  qgsdxfexportdialog.cpp
  qgsformannotationdialog.cpp
  qgsguivectorlayertools.cpp
  qgshtmlannotationdialog.cpp
  qgsdelattrdialog.cpp
  qgsdiagramproperties.cpp
  qgsdisplayangle.cpp
  qgsfieldcalculator.cpp
  qgsfirstrundialog.cpp
  qgsgeometryvalidationservice.cpp
  qgsgeometryvalidationdock.cpp
  qgsgeometryvalidationmodel.cpp
  qgssourcefieldsproperties.cpp
  qgsattributesformproperties.cpp
  qgsidentifyresultsdialog.cpp
  qgsinstallgridshiftdialog.cpp
  qgsfeatureaction.cpp
  qgslayercapabilitiesmodel.cpp
  qgslayertreeviewindicatorprovider.cpp
  qgslayertreeviewembeddedindicator.cpp
  qgslayertreeviewfilterindicator.cpp
  qgslayertreeviewmemoryindicator.cpp
  qgslayertreeviewnocrsindicator.cpp
  qgslayertreeviewnonremovableindicator.cpp
  qgslayertreeviewbadlayerindicator.cpp
  qgsmaskingwidget.cpp
  qgsmasksourceselectionwidget.cpp
  qgsmapcanvasdockwidget.cpp
  qgsmaplayerstylecategoriesmodel.cpp
  qgsmapsavedialog.cpp
  qgsprojectlistitemdelegate.cpp
  qgspuzzlewidget.cpp
  qgsversionmigration.cpp
  qgssnappinglayertreemodel.cpp
  qgssnappingwidget.cpp
  qgsstatusbarcoordinateswidget.cpp
  qgsstatusbarmagnifierwidget.cpp
  qgsstatusbarscalewidget.cpp
  qgssymbollayerselectionwidget.cpp
  qgstemplateprojectsmodel.cpp
  qgsvectorlayerloadstyledialog.cpp
  qgsversioninfo.cpp
  qgsrecentprojectsitemsmodel.cpp
  qgswelcomepage.cpp

  qgsmaptooladdfeature.cpp
  qgsmaptooladdpart.cpp
  qgsmaptooladdring.cpp
  qgsmaptoolfillring.cpp
  qgsmaptoolannotation.cpp
  qgsmaptooldeletering.cpp
  qgsmaptooldeletepart.cpp
  qgsmaptooltrimextendfeature.cpp
  qgsmaptoolfeatureaction.cpp
  qgsmaptoolformannotation.cpp
  qgsmaptoolhtmlannotation.cpp
  qgsmaptoolidentifyaction.cpp
  qgsmaptoolmeasureangle.cpp
  qgsmaptoolmovefeature.cpp
  qgsmaptooloffsetcurve.cpp
  qgsmaptooloffsetpointsymbol.cpp
  qgsmaptoolpointsymbol.cpp
  qgsmaptoolreshape.cpp
  qgsmaptoolreverseline.cpp
  qgsmaptoolrotatefeature.cpp
  qgsmaptoolrotatepointsymbols.cpp
  qgsmaptoolselect.cpp
  qgsmaptoolselectionhandler.cpp
  qgsmaptoolselectutils.cpp
  qgsmaptoolsimplify.cpp
  qgsmaptoolsplitfeatures.cpp
  qgsmaptoolsplitparts.cpp
  qgsmaptoolsvgannotation.cpp
  qgsmaptooltextannotation.cpp

  decorations/qgsdecorationitem.cpp
  decorations/qgsdecorationtitle.cpp
  decorations/qgsdecorationtitledialog.cpp
  decorations/qgsdecorationcopyright.cpp
  decorations/qgsdecorationcopyrightdialog.cpp
  decorations/qgsdecorationimage.cpp
  decorations/qgsdecorationimagedialog.cpp
  decorations/qgsdecorationlayoutextent.cpp
  decorations/qgsdecorationlayoutextentdialog.cpp
  decorations/qgsdecorationnortharrow.cpp
  decorations/qgsdecorationnortharrowdialog.cpp
  decorations/qgsdecorationscalebar.cpp
  decorations/qgsdecorationscalebardialog.cpp
  decorations/qgsdecorationgrid.cpp
  decorations/qgsdecorationgriddialog.cpp

  vertextool/qgslockedfeature.cpp
  vertextool/qgsvertexeditor.cpp
  vertextool/qgsvertextool.cpp

  qgslayerstylingwidget.cpp
  qgsmeasuredialog.cpp
  qgsmeasuretool.cpp
  qgsmergeattributesdialog.cpp
  qgsoptions.cpp
  qgspointmarkeritem.cpp
  qgspointrotationitem.cpp
  qgspluginmetadata.cpp
  qgspluginregistry.cpp
  qgsprojectlayergroupdialog.cpp
  qgsprojectproperties.cpp
  qgsrastercalcdialog.cpp
  qgsrelationmanagerdialog.cpp
  qgsrelationadddlg.cpp
  qgsselectbyformdialog.cpp
  qgsstatisticalsummarydockwidget.cpp
  qgstextannotationdialog.cpp
  qgssvgannotationdialog.cpp
  qgsundowidget.cpp
  qgsvectorlayerlegendwidget.cpp
  qgsvectorlayersavestyledialog.cpp
  qgsvectorlayerproperties.cpp
  qgsmapthemes.cpp
  qgshandlebadlayers.cpp
  qgsmaptooladdcircularstring.cpp
  qgsmaptoolcircularstringcurvepoint.cpp
  qgsmaptoolcircularstringradius.cpp
  qgsmaptooladdcircle.cpp
  qgsmaptoolcircle2points.cpp
  qgsmaptoolcircle3points.cpp
  qgsmaptoolcircle3tangents.cpp
  qgsmaptoolcircle2tangentspoint.cpp
  qgsmaptoolcirclecenterpoint.cpp
  qgsmaptooladdellipse.cpp
  qgsmaptoolellipsefoci.cpp
  qgsmaptoolellipseextent.cpp
  qgsmaptoolellipsecenterpoint.cpp
  qgsmaptoolellipsecenter2points.cpp
  qgsmaptooladdrectangle.cpp
  qgsmaptooladdregularpolygon.cpp
  qgsmaptoolrectanglecenter.cpp
  qgsmaptoolrectangleextent.cpp
  qgsmaptoolrectangle3points.cpp
  qgsmaptoolregularpolygon2points.cpp
  qgsmaptoolregularpolygoncenterpoint.cpp
  qgsmaptoolregularpolygoncentercorner.cpp

  browser/qgsinbuiltdataitemproviders.cpp

  labeling/qgslabelpropertydialog.cpp
  labeling/qgslabelengineconfigdialog.cpp
  labeling/qgslabelingwidget.cpp
  labeling/qgsmaptoolchangelabelproperties.cpp
  labeling/qgsmaptoolpinlabels.cpp
  labeling/qgsmaptoolshowhidelabels.cpp
  labeling/qgsmaptoollabel.cpp
  labeling/qgsmaptoolmovelabel.cpp
  labeling/qgsmaptoolrotatelabel.cpp
  labeling/qgsrulebasedlabelingwidget.cpp

  layout/qgsgeopdflayertreemodel.cpp
  layout/qgslayoutaddpagesdialog.cpp
  layout/qgslayoutapputils.cpp
  layout/qgslayoutatlaswidget.cpp
  layout/qgslayoutattributeselectiondialog.cpp
  layout/qgslayoutattributetablewidget.cpp
  layout/qgslayoutdesignerdialog.cpp
  layout/qgslayoutguidewidget.cpp
  layout/qgslayouthtmlwidget.cpp
  layout/qgslayoutimagedrophandler.cpp
  layout/qgslayoutimageexportoptionsdialog.cpp
  layout/qgslayoutitemslistview.cpp
  layout/qgslayoutappmenuprovider.cpp
  layout/qgslayoutlabelwidget.cpp
  layout/qgslayoutlegendlayersdialog.cpp
  layout/qgslayoutlegendwidget.cpp
  layout/qgslayoutmanagerdialog.cpp
  layout/qgslayoutmanualtablewidget.cpp
  layout/qgslayoutmapwidget.cpp
  layout/qgslayoutmapgridwidget.cpp
  layout/qgslayoutpagepropertieswidget.cpp
  layout/qgslayoutpdfexportoptionsdialog.cpp
  layout/qgslayoutpicturewidget.cpp
  layout/qgslayoutpolygonwidget.cpp
  layout/qgslayoutpolylinewidget.cpp
  layout/qgslayoutpropertieswidget.cpp
  layout/qgslayoutqptdrophandler.cpp
  layout/qgslayoutscalebarwidget.cpp
  layout/qgslayoutshapewidget.cpp
  layout/qgslayouttablebackgroundcolorsdialog.cpp
  layout/qgslayoutvaliditychecks.cpp
  layout/qgsreportfieldgroupsectionwidget.cpp
  layout/qgsreportlayoutsectionwidget.cpp
  layout/qgsreportorganizerwidget.cpp
  layout/qgsreportsectionmodel.cpp
  layout/qgsreportsectionwidget.cpp

  locator/qgsinbuiltlocatorfilters.cpp
  locator/qgslocatoroptionswidget.cpp

  gps/qgsgpsbearingitem.cpp
  gps/qgsgpsinformationwidget.cpp
  gps/qgsgpsmarker.cpp

  dwg/qgsdwgimportdialog.cpp
  dwg/qgsdwgimporter.cpp

  pluginmanager/qgspluginmanager.cpp
  pluginmanager/qgspluginmanager_texts.cpp
  pluginmanager/qgsapppluginmanagerinterface.cpp
  pluginmanager/qgspluginsortfilterproxymodel.cpp
  pluginmanager/qgspluginitemdelegate.cpp

  qgsnewspatialitelayerdialog.cpp

  qgssettingstree.cpp
  qgsvariantdelegate.cpp
  qgscrashhandler.cpp

  mesh/qgsmeshlayerproperties.cpp
  mesh/qgsrenderermeshpropertieswidget.cpp
  mesh/qgsmeshrenderer3daveragingwidget.cpp
  mesh/qgsmeshrenderermeshsettingswidget.cpp
  mesh/qgsmeshrendererscalarsettingswidget.cpp
  mesh/qgsmeshrenderervectorsettingswidget.cpp
  mesh/qgsmeshrendereractivedatasetwidget.cpp
  mesh/qgsmeshdatasetgrouptreeview.cpp
  mesh/qgsmeshcalculatordialog.cpp
  mesh/qgsmeshtimeformatdialog.cpp
)

SET(QGIS_APP_HDRS
  qgisapp.h
  qgisappinterface.h
  qgisappstylesheet.h
  qgsabout.h
  qgsaddattrdialog.h
  qgsaddtaborgroup.h
  qgsannotationwidget.h
  qgsappbrowserproviders.h
  qgsattributeactiondialog.h
  qgsattributerelationedit.h
  qgsattributesforminitcode.h
  qgsattributesformproperties.h
  qgsattributetabledialog.h
  qgsattributetypedialog.h
  qgsbookmarkeditordialog.h
  qgsbookmarks.h
  qgsclipboard.h
  qgscrashhandler.h
  qgscustomization.h
  qgscustomprojectiondialog.h
  qgsdatumtransformtablewidget.h
  qgsdelattrdialog.h
  qgsdiagramproperties.h
  qgsdiscoverrelationsdialog.h
  qgsdisplayangle.h
  qgsfeatureaction.h
  qgsfieldcalculator.h
  qgsfirstrundialog.h
  qgsformannotationdialog.h
  qgshandlebadlayers.h
  qgshtmlannotationdialog.h
  qgsidentifyresultsdialog.h
  qgsjoindialog.h
  qgslayercapabilitiesmodel.h
  qgslayerstylingwidget.h
  qgsmapcanvasdockwidget.h
  qgsmapsavedialog.h
  qgsmapthemes.h
  qgsmaptooladdcircle.h
  qgsmaptooladdcircularstring.h
  qgsmaptooladdellipse.h
  qgsmaptooladdfeature.h
  qgsmaptooladdpart.h
  qgsmaptooladdrectangle.h
  qgsmaptooladdregularpolygon.h
  qgsmaptooladdring.h
  qgsmaptoolannotation.h
  qgsmaptoolcircle2points.h
  qgsmaptoolcircle3points.h
  qgsmaptoolcirclecenterpoint.h
  qgsmaptoolcircularstringcurvepoint.h
  qgsmaptoolcircularstringradius.h
  qgsmaptooldeletepart.h
  qgsmaptooldeletering.h
  qgsmaptoolellipsecenter2points.h
  qgsmaptoolellipsecenterpoint.h
  qgsmaptoolellipseextent.h
  qgsmaptoolellipsefoci.h
  qgsmaptoolfeatureaction.h
  qgsmaptoolfillring.h
  qgsmaptoolformannotation.h
  qgsmaptoolhtmlannotation.h
  qgsmaptoolidentifyaction.h
  qgsmaptoolmeasureangle.h
  qgsmaptoolmovefeature.h
  qgsmaptooloffsetcurve.h
  qgsmaptooloffsetpointsymbol.h
  qgsmaptoolpointsymbol.h
  qgsmaptoolrectangle3points.h
  qgsmaptoolrectanglecenter.h
  qgsmaptoolrectangleextent.h
  qgsmaptoolregularpolygon2points.h
  qgsmaptoolregularpolygoncentercorner.h
  qgsmaptoolregularpolygoncenterpoint.h
  qgsmaptoolreshape.h
  qgsmaptoolreverseline.h
  qgsmaptoolrotatefeature.h
  qgsmaptoolrotatepointsymbols.h
  qgsmaptoolselect.h
  qgsmaptoolsimplify.h
  qgsmaptoolsplitfeatures.h
  qgsmaptoolsvgannotation.h
  qgsmaptooltextannotation.h
  qgsmaptooltrimextendfeature.h
  qgsmeasuredialog.h
  qgsmeasuretool.h
  qgsmergeattributesdialog.h
  qgsnewspatialitelayerdialog.h
  qgsoptions.h
  qgspluginmetadata.h
  qgspluginregistry.h
  qgspointmarkeritem.h
  qgspointrotationitem.h
  qgsprojectlayergroupdialog.h
  qgsprojectproperties.h
  qgsrastercalcdialog.h
  qgsrelationadddlg.h
  qgsrelationmanagerdialog.h
  qgsselectbyformdialog.h
  qgssnappinglayertreemodel.h
  qgssnappingwidget.h
  qgssourcefieldsproperties.h
  qgsstatisticalsummarydockwidget.h
  qgsstatusbarcoordinateswidget.h
  qgsstatusbarmagnifierwidget.h
  qgsstatusbarscalewidget.h
  qgssvgannotationdialog.h
  qgstextannotationdialog.h
  qgsundowidget.h
  qgsvectorlayerloadstyledialog.h
  qgsvectorlayerproperties.h
  qgsversionmigration.h
  qgswmsdimensiondialog.h

  decorations/qgsdecorationcopyright.h
  decorations/qgsdecorationcopyrightdialog.h
  decorations/qgsdecorationgrid.h
  decorations/qgsdecorationgriddialog.h
  decorations/qgsdecorationimage.h
  decorations/qgsdecorationimagedialog.h
  decorations/qgsdecorationitem.h
  decorations/qgsdecorationlayoutextent.h
  decorations/qgsdecorationlayoutextentdialog.h
  decorations/qgsdecorationnortharrow.h
  decorations/qgsdecorationnortharrowdialog.h
  decorations/qgsdecorationscalebar.h
  decorations/qgsdecorationscalebardialog.h
  decorations/qgsdecorationtitle.h
  decorations/qgsdecorationtitledialog.h

  gps/qgsgpsinformationwidget.h

  labeling/qgslabelengineconfigdialog.h
  labeling/qgslabelpropertydialog.h
  labeling/qgsmaptoolchangelabelproperties.h
  labeling/qgsmaptoollabel.h
  labeling/qgsmaptoolmovelabel.h
  labeling/qgsmaptoolpinlabels.h
  labeling/qgsmaptoolrotatelabel.h
  labeling/qgsmaptoolshowhidelabels.h
  labeling/qgsrulebasedlabelingwidget.h

  layout/qgsgeopdflayertreemodel.h
  layout/qgslayoutvaliditychecks.h

  locator/qgsinbuiltlocatorfilters.h

  mesh/qgsmeshcalculatordialog.h
  mesh/qgsmeshdatasetgrouptreeview.h
  mesh/qgsmeshlayerproperties.h
  mesh/qgsmeshrenderer3daveragingwidget.h
  mesh/qgsmeshrendereractivedatasetwidget.h
  mesh/qgsmeshrenderermeshsettingswidget.h
  mesh/qgsmeshrendererscalarsettingswidget.h
  mesh/qgsmeshrenderervectorsettingswidget.h
  mesh/qgsmeshtimeformatdialog.h
  mesh/qgsrenderermeshpropertieswidget.h

  vertextool/qgsvertexeditor.h
  vertextool/qgsvertextool.h
)

IF (WITH_3D)
  SET(QGIS_APP_SRCS
    ${QGIS_APP_SRCS}
    3d/qgs3danimationwidget.cpp
    3d/qgs3danimationexportdialog.cpp
    3d/qgs3dmapcanvas.cpp
    3d/qgs3dmapcanvasdockwidget.cpp
    3d/qgs3dmapconfigwidget.cpp
    3d/qgs3dmaptool.cpp
    3d/qgs3dmaptoolidentify.cpp
    3d/qgs3dmaptoolmeasureline.cpp
    3d/qgs3dmeasuredialog.cpp
    3d/qgs3dnavigationwidget.cpp
    3d/qgslightswidget.cpp
    3d/qgsline3dsymbolwidget.cpp
    3d/qgsmesh3dsymbolwidget.cpp
    3d/qgspoint3dsymbolwidget.cpp
    3d/qgspolygon3dsymbolwidget.cpp
    3d/qgsphongmaterialwidget.cpp
    3d/qgsrulebased3drendererwidget.cpp
    3d/qgssymbol3dwidget.cpp
    3d/qgsvectorlayer3drendererwidget.cpp
    3d/qgsmeshlayer3drendererwidget.cpp
    layout/qgslayout3dmapwidget.cpp
  )
ENDIF (WITH_3D)


SET (WITH_QWTPOLAR FALSE CACHE BOOL "Determines whether QwtPolar should be built")

IF (WITH_QWTPOLAR)
  ADD_DEFINITIONS(-DWITH_QWTPOLAR)
  # Try to find QwtPolar on the system
  FIND_PACKAGE(QwtPolar)
  # If not found on the system, offer the possibility to build QwtPolar
  # internally
  IF(NOT FOUND_QwtPolar)
    SET(DEFAULT_WITH_INTERNAL_QWTPOLAR TRUE)
  ELSE(NOT FOUND_QwtPolar)
    SET(DEFAULT_WITH_INTERNAL_QWTPOLAR FALSE)
  ENDIF(NOT FOUND_QwtPolar)
  SET (WITH_INTERNAL_QWTPOLAR DEFAULT_WITH_INTERNAL_QWTPOLAR CACHE BOOL "Use internal build of QwtPolar")

  IF(WITH_INTERNAL_QWTPOLAR)
    SET(QGIS_APP_SRCS
      ${QGIS_APP_SRCS}
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_canvas.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_curve.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_fitter.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_grid.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_item.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_itemdict.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_layout.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_magnifier.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_marker.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_panner.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_picker.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_plot.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_renderer.cpp
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_spectrogram.cpp
    )

    SET(QWTPOLAR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1)

    SET(QWTPOLAR_LIBRARY "")
  ENDIF(WITH_INTERNAL_QWTPOLAR)
ELSE(WITH_QWTPOLAR)
  SET(QWTPOLAR_LIBRARY "")
  SET(QWTPOLAR_INCLUDE_DIR "")
ENDIF(WITH_QWTPOLAR)

IF (POSTGRES_FOUND)
  IF(HAVE_PGCONFIG)
    ADD_DEFINITIONS(-DHAVE_PGCONFIG=1)
  ENDIF(HAVE_PGCONFIG)
ENDIF (POSTGRES_FOUND)

SET(QGIS_APP_UI_HDRS  
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgisapp.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgs25drendererwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsabout.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsaddattrdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsaddtaborgroupbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsadvanceddigitizingdockwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsadvanceddigitizingfloaterbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsannotationwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsarrowsymbollayerwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsattributeactiondialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsattributeloadfrommap.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsattributerelationedit.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsattributesforminitcode.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsattributesformproperties.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsattributetabledialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsattributetypeedit.h
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
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthmasterpassresetdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthmethodplugins.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthserverseditor.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthsettingswidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthsslconfigwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthsslerrorsdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthsslimportdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsauthtrustedcasdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsbasicnumericformatwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsbearingnumericformatwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsbookmarkeditordialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsbookmarksbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsbrowserdirectorypropertiesbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsbrowserdockwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsbrowserlayerpropertiesbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsbrowserpropertiesdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscategorizedsymbolrendererwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscharacterselectdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscheckboxconfigdlgbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscolorbrewercolorrampwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscolordialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscolorrampshaderwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscompoundcolorwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsconfigureshortcutsdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscoordinateoperationwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscptcitycolorrampdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscredentialdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscurrencynumericformatwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscustomizationdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgscustomprojectiondialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdashspacewidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdatadefinedsizelegendwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdatasourcemanagerdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdatasourceselectdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdatetimeeditconfig.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdatumtransformdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdatumtransformtablewidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdecorationcopyrightdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdecorationgriddialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdecorationimagedialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdecorationlayoutextentdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdecorationnortharrowdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdecorationscalebardialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdecorationtitledialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdelattrdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdetaileditemwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdiagrampropertiesbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdiscoverrelationsdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdisplayanglebase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdualviewbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsdummyconfigdlgbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgseditconditionalformatrulewidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgseffectpropertieswidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgseffectstackpropertieswidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgserrordialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionbuilder.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionbuilderdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionselectiondialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexpressionstoredialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsextentgroupboxwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsexternalresourceconfigdlg.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsfeaturefilterwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsfeatureselectiondlg.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsfieldcalculatorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsfieldconditionalformatwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsfindfilesbypatternwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsfirstrundialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsformannotationdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgenericprojectionselectorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgeometrygeneratorwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgpsinformationwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgradientcolorrampdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgraduatedsymbolrendererwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsgroupwmsdatadialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgshandlebadlayersbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsheatmaprendererwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgshillshaderendererwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgshistogramwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsidentifyresultsbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsinvertedpolygonrendererwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsjoindialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslabelengineconfigdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslabelingrulepropswidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslabelobstaclesettingswidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslabelpropertydialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslayertreeembeddedconfigwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslayoutitemwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslayoutnewitemproperties.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgslimitedrandomcolorrampwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsludialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmanageconnectionsdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmapcanvasdockwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmapsavedialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmapstylingwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmapunitscalewidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmasksymbollayerwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmeasurebase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmergeattributesdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmeshcalculatordialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmeshlayerpropertiesbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmeshrenderer3daveragingwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmeshrendereractivedatasetwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmeshrenderermeshsettingswidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmeshrendererscalarsettingswidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmeshrenderervectorsettingswidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmeshtimeformatdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessagelogviewer.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmessageviewer.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmetadatawidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsmultibandcolorrendererwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsnewauxiliaryfielddialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsnewauxiliarylayerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsnewgeopackagelayerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsnewhttpconnectionbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsnewmemorylayerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsnewogrconnectionbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsnewspatialitelayerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsnewvectorlayerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsnumericformatselectorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsoffsetuserinputwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsoptionsbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsorderbydialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsorganizetablecolumnsdialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsowssourceselectbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgspalettedrendererwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgspercentagenumericformatwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgspointclusterrendererwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgspointdisplacementrendererwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgspresetcolorrampwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprocessingalgorithmdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprocessingalgorithmprogressdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprocessingmatrixparameterdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprocessingmultipleselectiondialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprojectionselectorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprojectlayergroupdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsprojectpropertiesbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgspropertyassistantwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgspropertycolorassistantwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgspropertygenericnumericassistantwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgspropertysizeassistantwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsquerybuilderbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrangeconfigdlgbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrastercalcdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrasterformatsaveoptionswidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrasterhistogramwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrasterlayerpropertiesbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrasterlayersaveasdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrasterminmaxwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrasterpyramidsoptionswidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrastertransparencywidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrelationmanagerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrelationreferenceconfigdlgbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrenderercontainerbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrenderermeshpropswidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrendererpropsdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrendererrasterpropswidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrendererrulepropsdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrulebasedlabelingwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsrulebasedrendererwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsscientificnumericformatwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssimplifytolerancedialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssinglebandgrayrendererwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssinglebandpseudocolorrendererwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssmartgroupconditionwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssmartgroupeditordialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssourcefieldsproperties.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssqlcomposerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsstatisticalsummarybase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsstyleexportimportdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsstylegroupselectiondialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsstyleitemslistwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsstylemanagerdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsstylesavedialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssublayersdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssubstitutionlistwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssymbollevelsdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgssymbolselectordialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgstableeditorbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgstableeditorformattingwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgstablewidgetuibase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgstextannotationdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgstexteditconfigdlg.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgstextformatwidgetbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsuniquevaluesconfigdlgbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsunitselectionwidget.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsvaliditycheckresultsbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsvaluemapconfigdlgbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsvaluerelationconfigdlgbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsvectorlayerloadstyledialog.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsvectorlayerpropertiesbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgsvectorlayersaveasdialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_qgswmsdimensiondialogbase.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_blur.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_centroidfill.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_coloreffects.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_drawsource.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_ellipse.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_filledmarker.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_fontmarker.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_glow.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_gradientfill.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_hashline.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_layerproperties.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_linepatternfill.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_markerline.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_pointpatternfill.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_randommarkerfill.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_rasterfill.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_rastermarker.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_set_dd_value.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_shadoweffect.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_shapeburstfill.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_simplefill.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_simpleline.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_simplelinecallout.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_simplemarker.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_svgfill.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_svgmarker.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_svgselector.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_symbolslist.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_transform.h
  ${CMAKE_CURRENT_BINARY_DIR}/../ui/ui_widget_vectorfield.h
)

# Test data dir for QgsAppScreenShots
ADD_DEFINITIONS(-DTEST_DATA_DIR="${TEST_DATA_DIR}")


SET(IMAGE_RCCS ../../images/images.qrc)

QT5_ADD_RESOURCES(IMAGE_RCC_SRCS ${IMAGE_RCCS})

# add test resources, e.g. standard test font
SET(TEST_RCCS ../../tests/testdata/testdata.qrc)
QT5_ADD_RESOURCES(TEST_RCC_SRCS ${TEST_RCCS})

IF (WIN32)
  SET (QGIS_APP_SRCS ${QGIS_APP_SRCS} main.cpp ${IMAGE_RCC_SRCS} ${TEST_RCC_SRCS})
  SET (QGIS_APPMAIN_SRCS mainwin.cpp)

  IF (MSVC)
    SET (QGIS_APPMAIN_SRCS ${QGIS_APPMAIN_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/qgis_win32.rc)
  ELSE(MSVC)
    # Icon for windows MINGW
    # Note: can't include .rc directly to source files
    # as it's ignored when used MinGW

    IF (NOT WINDRES)
      FIND_PROGRAM(WINDRES windres)
      IF (NOT WINDRES AND NOT CMAKE_RC_COMPILER)
        MESSAGE(FATAL_ERROR "windres not found - aborting")
      ENDIF (NOT WINDRES AND NOT CMAKE_RC_COMPILER)
    ENDIF (NOT WINDRES)

    IF (WINDRES)
        #############################################################
        # application icon
        # resource compilation for MinGW
        ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/icon.o
          COMMAND ${WINDRES} -I${CMAKE_CURRENT_SOURCE_DIR} -i${CMAKE_CURRENT_SOURCE_DIR}/qgis_win32.rc
          -o ${CMAKE_CURRENT_BINARY_DIR}/icon.o )
        SET(QGIS_APPMAIN_SRCS ${QGIS_APPMAIN_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/icon.o)
    ELSE (WINDRES)
        # MXE
    ENDIF (WINDRES)
  ENDIF (MSVC)
ELSE(WIN32)
  SET (QGIS_APPMAIN_SRCS main.cpp ${IMAGE_RCC_SRCS} ${TEST_RCC_SRCS})
ENDIF (WIN32)

IF(PEDANTIC)
  # disable deprecation warnings
  IF(MSVC)
    SET_SOURCE_FILES_PROPERTIES(
        qgisappinterface.cpp
        ${CMAKE_BINARY_DIR}/src/app/moc_qgisappinterface.cxx
    PROPERTIES COMPILE_FLAGS "/wd4996"
    )
  ELSE(MSVC)
    SET_SOURCE_FILES_PROPERTIES(
        qgisappinterface.cpp
        ${CMAKE_BINARY_DIR}/src/app/moc_qgisappinterface.cxx
        PROPERTIES COMPILE_FLAGS "-Wno-deprecated-declarations"
    )
  ENDIF(MSVC)
ENDIF(PEDANTIC)

IF(MSVC)
# -wd4091 Avoid 'typedef' ignored on left of '' when no variable is declared warning in dbghelp.h
SET_SOURCE_FILES_PROPERTIES(
  qgisapp.cpp
  main.cpp
  qgscrashhandler.cpp
  PROPERTIES COMPILE_FLAGS -wd4091)
ENDIF(MSVC)

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/external/libdxfrw
  ${CMAKE_SOURCE_DIR}/external/nmea
  ${CMAKE_SOURCE_DIR}/external

  ${CMAKE_SOURCE_DIR}/src/app
  ${CMAKE_SOURCE_DIR}/src/app/browser
  ${CMAKE_SOURCE_DIR}/src/app/decorations
  ${CMAKE_SOURCE_DIR}/src/app/labeling
  ${CMAKE_SOURCE_DIR}/src/app/layout
  ${CMAKE_SOURCE_DIR}/src/app/pluginmanager
  ${CMAKE_SOURCE_DIR}/src/app/gps
  ${CMAKE_SOURCE_DIR}/src/app/dwg
  ${CMAKE_SOURCE_DIR}/src/app/mesh
  ${CMAKE_SOURCE_DIR}/src/app/locator
  ${CMAKE_SOURCE_DIR}/src/app/vertextool
  ${CMAKE_SOURCE_DIR}/src/analysis
  ${CMAKE_SOURCE_DIR}/src/analysis/raster
  ${CMAKE_SOURCE_DIR}/src/analysis/mesh
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/annotations
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/gps
  ${CMAKE_SOURCE_DIR}/src/core/diagram
  ${CMAKE_SOURCE_DIR}/src/core/dxf
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/mesh
  ${CMAKE_SOURCE_DIR}/src/core/numericformats
  ${CMAKE_SOURCE_DIR}/src/core/labeling
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/locator
  ${CMAKE_SOURCE_DIR}/src/core/numericformats
  ${CMAKE_SOURCE_DIR}/src/core/pal
  ${CMAKE_SOURCE_DIR}/src/core/providers/memory
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/scalebar
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/effects
  ${CMAKE_SOURCE_DIR}/src/core/validity
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/symbology
  ${CMAKE_SOURCE_DIR}/src/gui/attributetable
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/gui/labeling
  ${CMAKE_SOURCE_DIR}/src/gui/numericformats
  ${CMAKE_SOURCE_DIR}/src/gui/ogr
  ${CMAKE_SOURCE_DIR}/src/gui/numericformats
  ${CMAKE_SOURCE_DIR}/src/gui/processing
  ${CMAKE_SOURCE_DIR}/src/gui/raster
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets/core
  ${CMAKE_SOURCE_DIR}/src/gui/layertree
  ${CMAKE_SOURCE_DIR}/src/gui/locator
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/python
  ${CMAKE_SOURCE_DIR}/src/native

  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/python
  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/app
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/native
)

IF (WITH_3D)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/src/app/3d
    ${CMAKE_SOURCE_DIR}/src/core/3d
    ${CMAKE_SOURCE_DIR}/src/3d
    ${CMAKE_SOURCE_DIR}/src/3d/symbols
    ${CMAKE_SOURCE_DIR}/src/3d/terrain
    ${CMAKE_SOURCE_DIR}/src/3d/chunks
    ${CMAKE_BINARY_DIR}/src/3d
  )
  INCLUDE_DIRECTORIES(SYSTEM
    ${QT5_3DEXTRA_INCLUDE_DIR}
  )
ENDIF (WITH_3D)

INCLUDE_DIRECTORIES(SYSTEM
  ${QWT_INCLUDE_DIR}
  ${QT_QTUITOOLS_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/external/qt-unix-signals
)
INCLUDE_DIRECTORIES(
  ../analysis/processing
  ../analysis/raster
  ../analysis/vector/geometry_checker
  ../core
  ../core/annotations
  ../core/auth
  ../core/gps
  ../core/diagram
  ../core/dxf
  ../core/geometry
  ../core/geocms/geonode
  ../core/layout
  ../core/metadata
  ../core/layertree
  ../core/processing
  ../core/providers/memory
  ../core/raster
  ../core/mesh
  ../core/scalebar
  ../core/symbology
  ../gui
  ../gui/symbology
  ../gui/attributetable
  ../gui/auth
  ../gui/ogr
  ../gui/raster
  ../gui/editorwidgets
  ../gui/editorwidgets/core
  ../gui/layertree
  ../gui/layout
  ../gui/tableeditor
  ../plugins
  ../python
  gps
  dwg
  ${CMAKE_SOURCE_DIR}/external/libdxfrw
  ${CMAKE_SOURCE_DIR}/external/nlohmann
  ${CMAKE_SOURCE_DIR}/src/native

  ${CMAKE_BINARY_DIR}/src/native
)
INCLUDE_DIRECTORIES(SYSTEM
  ${PROJ_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${SPATIALITE_INCLUDE_DIR}
  ${SQLITE3_INCLUDE_DIR}
  ${GEOS_INCLUDE_DIR}
  ${QWTPOLAR_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
)

IF(HAVE_OPENCL)
    INCLUDE_DIRECTORIES(SYSTEM ${OpenCL_INCLUDE_DIRS})
ENDIF(HAVE_OPENCL)

IF(ENABLE_MODELTEST)
  INCLUDE_DIRECTORIES(../../tests/qt_modeltest)
ENDIF(ENABLE_MODELTEST)

IF (ANDROID)
  INCLUDE_DIRECTORIES(SYSTEM ${ANDROID_NDK_TOOLCHAIN_ROOT}/sysroot/usr/include)
ENDIF (ANDROID)

IF (APPLE)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/native/mac)
ENDIF (APPLE)

IF (POSTGRES_FOUND)
  INCLUDE_DIRECTORIES(SYSTEM ${POSTGRES_INCLUDE_DIR})
ENDIF (POSTGRES_FOUND)

#############
IF (ANDROID)
  ADD_LIBRARY(${QGIS_APP_NAME} SHARED ${QGIS_APPMAIN_SRCS})
ELSE (ANDROID)
  ADD_EXECUTABLE(${QGIS_APP_NAME} MACOSX_BUNDLE WIN32 ${QGIS_APPMAIN_SRCS})
ENDIF (ANDROID)

# Putting IMAGE_RCC_SRCS into qgis_app lib is causing problems when the lib is
# loaded (by plugin for example) in test unit (qgis_layoutpicturetest).
ADD_LIBRARY(qgis_app SHARED ${QGIS_APP_SRCS} ${QGIS_APP_HDRS})

TARGET_LINK_LIBRARIES(qgis_app
  ${QWT_LIBRARY}
  ${Qt5Sql_LIBRARIES}
  ${Qt5UiTools_LIBRARIES}
  ${OPTIONAL_QTWEBKIT}
  #should only be needed for win
  ${QT_QTMAIN_LIBRARY}
  ${QWTPOLAR_LIBRARY}
  ${GDAL_LIBRARY}
  qgis_core
  qgis_gui
  qgis_analysis
  qgis_native
  libdxfrw
)

TARGET_COMPILE_DEFINITIONS(qgis_app PRIVATE "-DQT_NO_FOREACH")

IF(ENABLE_MODELTEST)
  TARGET_LINK_LIBRARIES(qgis_app ${Qt5Test_LIBRARIES})
ENDIF(ENABLE_MODELTEST)

IF (WITH_3D)
  TARGET_LINK_LIBRARIES(qgis_app
    qgis_3d
  )
ENDIF (WITH_3D)

GENERATE_EXPORT_HEADER(
   qgis_app
   BASE_NAME APP
   EXPORT_FILE_NAME qgis_app.h
)


IF (NOT ANDROID)
  SET_TARGET_PROPERTIES(qgis_app PROPERTIES
    VERSION ${COMPLETE_VERSION}
    SOVERSION ${COMPLETE_VERSION}
    )
ENDIF (NOT ANDROID)

IF (APPLE)
  TARGET_LINK_LIBRARIES(qgis_app ${APP_SERVICES_LIBRARY})
ENDIF(APPLE)

if(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)
  SET_TARGET_PROPERTIES(qgis_app PROPERTIES STATIC_LIBRARY_FLAGS "/machine:x64")
ENDIF(MSVC AND CMAKE_SIZEOF_VOID_P EQUAL 8)

# clang-tidy
IF(CLANG_TIDY_EXE)
  SET_TARGET_PROPERTIES(
    qgis_app PROPERTIES
    CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
  )
ENDIF(CLANG_TIDY_EXE)

IF(NOT WIN32)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} qgis_app)
ENDIF(NOT WIN32)

IF (ANDROID)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} log)
ENDIF (ANDROID)

IF(WIN32)
  ADD_DEFINITIONS(-DQWT_DLL)
  ADD_DEFINITIONS(-DQSCINTILLA_DLL)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} dbghelp)
  TARGET_LINK_LIBRARIES(qgis_app dbghelp)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} qgis_native)

IF (NOT WIN32)
TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} QTSignal)
ENDIF (NOT WIN32)

IF (APPLE)
  SET_TARGET_PROPERTIES(${QGIS_APP_NAME} PROPERTIES
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
    INSTALL_RPATH_USE_LINK_PATH true
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/mac/app.info.plist.in")
ENDIF(APPLE)

IF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  FIND_LIBRARY(EXECINFO_LIBRARY NAMES execinfo)
  TARGET_LINK_LIBRARIES(${QGIS_APP_NAME} ${EXECINFO_LIBRARY})
ENDIF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")

IF (POSTGRES_FOUND)
  TARGET_LINK_LIBRARIES (qgis_app ${POSTGRES_LIBRARY})
ENDIF (POSTGRES_FOUND)

IF(MSVC)
  INSTALL(FILES qgis.ico qgis-mime.ico qgis-qgs.ico qgis-qlr.ico qgis-qml.ico qgis-qpt.ico DESTINATION ${CMAKE_INSTALL_PREFIX}/icons)
ENDIF(MSVC)

INSTALL(TARGETS qgis_app
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})

INSTALL(TARGETS ${QGIS_APP_NAME}
  BUNDLE DESTINATION ${QGIS_INSTALL_PREFIX}
  RUNTIME DESTINATION ${QGIS_BIN_DIR}
  LIBRARY DESTINATION ${QGIS_LIB_DIR}
  ARCHIVE DESTINATION ${QGIS_LIB_DIR}
  FRAMEWORK DESTINATION ${QGIS_FW_SUBDIR}
  PUBLIC_HEADER DESTINATION ${QGIS_INCLUDE_DIR})
  
# ui headers to install, don't exist at target definition time, install manually
IF (APPLE)
  INSTALL(FILES ${QGIS_APP_UI_HDRS} DESTINATION ${QGIS_FW_SUBDIR}/qgis_app.framework/Headers)
ELSE (APPLE)
  INSTALL(FILES ${QGIS_APP_UI_HDRS} DESTINATION ${QGIS_INCLUDE_DIR})
ENDIF (APPLE)

IF (APPLE)
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND /bin/echo -n \"APPLQGIS\" OUTPUT_FILE \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/../PkgInfo\")")
ENDIF (APPLE)
