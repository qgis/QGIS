ADD_SUBDIRECTORY(../../external/libdxfrw dwg/libdxfrw)

SET(QGIS_APP_SRCS
  ${CMAKE_SOURCE_DIR}/external/nmea/gmath.c

  qgisapp.cpp
  qgisappinterface.cpp
  qgisappstylesheet.cpp
  qgsabout.cpp
  qgsalignrasterdialog.cpp
  qgsappauthrequesthandler.cpp
  qgsappbrowserproviders.cpp
  qgsapplayertreeviewmenuprovider.cpp
  qgsappwindowmanager.cpp
  qgsaddattrdialog.cpp
  qgsaddtaborgroup.cpp
  qgsappscreenshots.cpp
  qgsjoindialog.cpp
  qgsannotationwidget.cpp
  qgsappsslerrorhandler.cpp
  qgsattributeactiondialog.cpp
  qgsattributeactionpropertiesdialog.cpp
  qgsattributetypedialog.cpp
  qgsattributerelationedit.cpp
  qgsattributesforminitcode.cpp
  qgsattributetabledialog.cpp
  qgsbookmarks.cpp
  qgsclipboard.cpp
  qgscustomization.cpp
  qgscustomprojectiondialog.cpp
  qgsdatumtransformtablewidget.cpp
  qgsdiscoverrelationsdlg.cpp
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
  qgsfeatureaction.cpp
  qgslabelpropertydialog.cpp
  qgslabelengineconfigdialog.cpp
  qgslabelinggui.cpp
  qgslabelingwidget.cpp
  qgslayercapabilitiesmodel.cpp
  qgslayertreeviewindicatorprovider.cpp
  qgslayertreeviewembeddedindicator.cpp
  qgslayertreeviewfilterindicator.cpp
  qgslayertreeviewmemoryindicator.cpp
  qgslayertreeviewnonremovableindicator.cpp
  qgslayertreeviewbadlayerindicator.cpp
  qgsmapcanvasdockwidget.cpp
  qgsmaplayerstylecategoriesmodel.cpp
  qgsmaplayerstyleguiutils.cpp
  qgsmapsavedialog.cpp
  qgspuzzlewidget.cpp
  qgsversionmigration.cpp
  qgsrulebasedlabelingwidget.cpp
  qgssnappinglayertreemodel.cpp
  qgssnappingwidget.cpp
  qgsstatusbarcoordinateswidget.cpp
  qgsstatusbarmagnifierwidget.cpp
  qgsstatusbarscalewidget.cpp
  qgsvectorlayerloadstyledialog.cpp
  qgsversioninfo.cpp
  qgswelcomepageitemsmodel.cpp
  qgswelcomepage.cpp

  qgsmaptooladdfeature.cpp
  qgsmaptooldigitizefeature.cpp
  qgsmaptooladdpart.cpp
  qgsmaptooladdring.cpp
  qgsmaptoolfillring.cpp
  qgsmaptoolannotation.cpp
  qgsmaptoolchangelabelproperties.cpp
  qgsmaptooldeletering.cpp
  qgsmaptooldeletepart.cpp
  qgsmaptooltrimextendfeature.cpp
  qgsmaptoolfeatureaction.cpp
  qgsmaptoolformannotation.cpp
  qgsmaptoolhtmlannotation.cpp
  qgsmaptoolpinlabels.cpp
  qgsmaptoolshowhidelabels.cpp
  qgsmaptoolidentifyaction.cpp
  qgsmaptoollabel.cpp
  qgsmaptoolmeasureangle.cpp
  qgsmaptoolmovefeature.cpp
  qgsmaptoolmovelabel.cpp
  qgsmaptooloffsetcurve.cpp
  qgsmaptooloffsetpointsymbol.cpp
  qgsmaptoolpointsymbol.cpp
  qgsmaptoolreshape.cpp
  qgsmaptoolreverseline.cpp
  qgsmaptoolrotatefeature.cpp
  qgsmaptoolrotatelabel.cpp
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
  decorations/qgsdecorationlayoutextent.cpp
  decorations/qgsdecorationlayoutextentdialog.cpp
  decorations/qgsdecorationnortharrow.cpp
  decorations/qgsdecorationnortharrowdialog.cpp
  decorations/qgsdecorationscalebar.cpp
  decorations/qgsdecorationscalebardialog.cpp
  decorations/qgsdecorationgrid.cpp
  decorations/qgsdecorationgriddialog.cpp

  vertextool/qgsselectedfeature.cpp
  vertextool/qgsvertexentry.cpp
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
  qgsrasterlayerproperties.cpp
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

  layout/qgslayoutaddpagesdialog.cpp
  layout/qgslayoutapputils.cpp
  layout/qgslayoutatlaswidget.cpp
  layout/qgslayoutattributeselectiondialog.cpp
  layout/qgslayoutattributetablewidget.cpp
  layout/qgslayoutdesignerdialog.cpp
  layout/qgslayoutguidewidget.cpp
  layout/qgslayouthtmlwidget.cpp
  layout/qgslayoutimageexportoptionsdialog.cpp
  layout/qgslayoutitemslistview.cpp
  layout/qgslayoutappmenuprovider.cpp
  layout/qgslayoutlabelwidget.cpp
  layout/qgslayoutlegendlayersdialog.cpp
  layout/qgslayoutlegendwidget.cpp
  layout/qgslayoutmanagerdialog.cpp
  layout/qgslayoutmapwidget.cpp
  layout/qgslayoutmapgridwidget.cpp
  layout/qgslayoutpagepropertieswidget.cpp
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
  mesh/qgsmeshrenderermeshsettingswidget.cpp
  mesh/qgsmeshrendererscalarsettingswidget.cpp
  mesh/qgsmeshrenderervectorsettingswidget.cpp
  mesh/qgsmeshrendereractivedatasetwidget.cpp
  mesh/qgsmeshdatasetgrouptreeview.cpp
  mesh/qgsmeshcalculatordialog.cpp
)

SET (QGIS_APP_MOC_HDRS
  qgisapp.h
  qgisappinterface.h
  qgisappstylesheet.h
  qgsabout.h
  qgsaddattrdialog.h
  qgsalignrasterdialog.h
  qgsappbrowserproviders.h
  qgsappscreenshots.h
  qgsjoindialog.h
  qgsaddtaborgroup.h
  qgsannotationwidget.h
  qgsapplayertreeviewmenuprovider.h
  qgsattributeactiondialog.h
  qgsattributeactionpropertiesdialog.h
  qgsattributetypedialog.h
  qgsattributerelationedit.h
  qgsattributesforminitcode.h
  qgsattributetabledialog.h
  qgsbookmarks.h
  qgsclipboard.h
  qgscustomization.h
  qgscustomprojectiondialog.h
  qgsdatumtransformtablewidget.h
  qgsdelattrdialog.h
  qgsdiagramproperties.h
  qgsdiscoverrelationsdlg.h
  qgsdisplayangle.h
  qgsdxfexportdialog.h
  qgsfeatureaction.h
  qgsfieldcalculator.h
  qgsfirstrundialog.h
  qgssourcefieldsproperties.h
  qgsattributesformproperties.h
  qgsformannotationdialog.h
  qgsguivectorlayertools.h
  qgsgeometryvalidationservice.h
  qgsgeometryvalidationdock.h
  qgsgeometryvalidationmodel.h
  qgshtmlannotationdialog.h
  qgsidentifyresultsdialog.h
  qgslabelengineconfigdialog.h
  qgslabelinggui.h
  qgslabelingwidget.h
  qgslabelpropertydialog.h
  qgslayercapabilitiesmodel.h
  qgslayertreeviewindicatorprovider.h
  qgslayertreeviewembeddedindicator.h
  qgslayertreeviewmemoryindicator.h
  qgslayertreeviewfilterindicator.h
  qgslayertreeviewnonremovableindicator.h
  qgslayertreeviewbadlayerindicator.h
  qgsmapcanvasdockwidget.h
  qgsmaplayerstylecategoriesmodel.h
  qgsmaplayerstyleguiutils.h
  qgsmapsavedialog.h
  qgspuzzlewidget.h
  qgsrulebasedlabelingwidget.h
  qgssnappinglayertreemodel.h
  qgssnappingwidget.h
  qgsstatusbarcoordinateswidget.h
  qgsstatusbarmagnifierwidget.h
  qgsstatusbarscalewidget.h
  qgsvectorlayerloadstyledialog.h
  qgsversioninfo.h
  qgswelcomepageitemsmodel.h
  qgswelcomepage.h

  qgsmaptooladdfeature.h
  qgsmaptooldigitizefeature.h
  qgsmaptoolannotation.h
  qgsmaptoolcircularstringradius.h
  qgsmaptooladdpart.h
  qgsmaptooladdring.h
  qgsmaptoolfillring.h
  qgsmaptoolchangelabelproperties.h
  qgsmaptooldeletepart.h
  qgsmaptooldeletering.h
  qgsmaptooltrimextendfeature.h
  qgsmaptoolfeatureaction.h
  qgsmaptoolformannotation.h
  qgsmaptoolhtmlannotation.h
  qgsmaptoolpinlabels.h
  qgsmaptoolshowhidelabels.h
  qgsmaptoolidentifyaction.h
  qgsmaptoolmeasureangle.h
  qgsmaptoolmovefeature.h
  qgsmaptoolmovelabel.h
  qgsmaptoollabel.h
  qgsmaptooloffsetcurve.h
  qgsmaptooloffsetpointsymbol.h
  qgsmaptoolpointsymbol.h
  qgsmaptoolreshape.h
  qgsmaptoolreverseline.h
  qgsmaptoolrotatefeature.h
  qgsmaptoolrotatelabel.h
  qgsmaptoolrotatepointsymbols.h
  qgsmaptoolselect.h
  qgsmaptoolselectionhandler.h
  qgsmaptoolsimplify.h
  qgsmaptoolsplitfeatures.h
  qgsmaptoolsplitparts.h
  qgsmaptoolsvgannotation.h
  qgsmaptooltextannotation.h
  qgsmaptooladdcircularstring.h
  qgsmaptoolcircularstringcurvepoint.h
  qgsmaptooladdcircle.h
  qgsmaptoolcircle2points.h
  qgsmaptoolcircle3points.h
  qgsmaptoolcircle3tangents.h
  qgsmaptoolcircle2tangentspoint.h
  qgsmaptoolcirclecenterpoint.h
  qgsmaptooladdellipse.h
  qgsmaptoolellipsefoci.h
  qgsmaptoolellipseextent.h
  qgsmaptoolellipsecenterpoint.h
  qgsmaptoolellipsecenter2points.h
  qgsmaptooladdrectangle.h
  qgsmaptooladdregularpolygon.h
  qgsmaptoolrectanglecenter.h
  qgsmaptoolrectangleextent.h
  qgsmaptoolrectangle3points.h
  qgsmaptoolregularpolygon2points.h
  qgsmaptoolregularpolygoncenterpoint.h
  qgsmaptoolregularpolygoncentercorner.h

  decorations/qgsdecorationitem.h
  decorations/qgsdecorationtitle.h
  decorations/qgsdecorationtitledialog.h
  decorations/qgsdecorationcopyright.h
  decorations/qgsdecorationcopyrightdialog.h
  decorations/qgsdecorationlayoutextent.h
  decorations/qgsdecorationlayoutextentdialog.h
  decorations/qgsdecorationnortharrow.h
  decorations/qgsdecorationnortharrowdialog.h
  decorations/qgsdecorationscalebar.h
  decorations/qgsdecorationscalebardialog.h
  decorations/qgsdecorationgrid.h
  decorations/qgsdecorationgriddialog.h

  vertextool/qgsselectedfeature.h
  vertextool/qgsvertexeditor.h
  vertextool/qgsvertextool.h

  qgslayerstylingwidget.h
  qgsmeasuredialog.h
  qgsmeasuretool.h
  qgsmergeattributesdialog.h
  qgsoptions.h
  qgsprojectlayergroupdialog.h
  qgsprojectproperties.h
  qgsrastercalcdialog.h
  qgsrasterlayerproperties.h
  qgsrelationmanagerdialog.h
  qgsrelationadddlg.h
  qgsselectbyformdialog.h
  qgsstatisticalsummarydockwidget.h
  qgssvgannotationdialog.h
  qgstextannotationdialog.h
  qgsundowidget.h
  qgsvectorlayerlegendwidget.h
  qgsvectorlayersavestyledialog.h
  qgsvectorlayerproperties.h
  qgsmapthemes.h
  qgshandlebadlayers.h

  browser/qgsinbuiltdataitemproviders.h

  layout/qgslayoutaddpagesdialog.h
  layout/qgslayoutappmenuprovider.h
  layout/qgslayoutatlaswidget.h
  layout/qgslayoutattributeselectiondialog.h
  layout/qgslayoutattributetablewidget.h
  layout/qgslayoutdesignerdialog.h
  layout/qgslayoutguidewidget.h
  layout/qgslayouthtmlwidget.h
  layout/qgslayoutimageexportoptionsdialog.h
  layout/qgslayoutitemslistview.h
  layout/qgslayoutlabelwidget.h
  layout/qgslayoutlegendwidget.h
  layout/qgslayoutlegendlayersdialog.h
  layout/qgslayoutmanagerdialog.h
  layout/qgslayoutmapwidget.h
  layout/qgslayoutmapgridwidget.h
  layout/qgslayoutpagepropertieswidget.h
  layout/qgslayoutpicturewidget.h
  layout/qgslayoutpolygonwidget.h
  layout/qgslayoutpolylinewidget.h
  layout/qgslayoutpropertieswidget.h
  layout/qgslayoutqptdrophandler.h
  layout/qgslayoutscalebarwidget.h
  layout/qgslayoutshapewidget.h
  layout/qgslayouttablebackgroundcolorsdialog.h
  layout/qgsreportfieldgroupsectionwidget.h
  layout/qgsreportlayoutsectionwidget.h
  layout/qgsreportorganizerwidget.h
  layout/qgsreportsectionmodel.h
  layout/qgsreportsectionwidget.h

  locator/qgsinbuiltlocatorfilters.h
  locator/qgslocatoroptionswidget.h

  gps/qgsgpsinformationwidget.h

  dwg/qgsdwgimportdialog.h

  pluginmanager/qgspluginmanager.h
  pluginmanager/qgsapppluginmanagerinterface.h
  pluginmanager/qgspluginsortfilterproxymodel.h
  pluginmanager/qgspluginitemdelegate.h

  qgsnewspatialitelayerdialog.h

  qgssettingstree.h
  qgsvariantdelegate.h

  mesh/qgsmeshlayerproperties.h
  mesh/qgsrenderermeshpropertieswidget.h
  mesh/qgsmeshrenderermeshsettingswidget.h
  mesh/qgsmeshrendererscalarsettingswidget.h
  mesh/qgsmeshrenderervectorsettingswidget.h
  mesh/qgsmeshrendereractivedatasetwidget.h
  mesh/qgsmeshdatasetgrouptreeview.h
  mesh/qgsmeshcalculatordialog.h
)


IF (WITH_3D)
  SET(QGIS_APP_SRCS
    ${QGIS_APP_SRCS}
    3d/qgs3danimationsettings.cpp
    3d/qgs3danimationwidget.cpp
    3d/qgs3dmapcanvas.cpp
    3d/qgs3dmapcanvasdockwidget.cpp
    3d/qgs3dmapconfigwidget.cpp
    3d/qgs3dmaptool.cpp
    3d/qgs3dmaptoolidentify.cpp
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

  SET (QGIS_APP_MOC_HDRS
    ${QGIS_APP_MOC_HDRS}
    3d/qgs3danimationwidget.h
    3d/qgs3dmapcanvas.h
    3d/qgs3dmapcanvasdockwidget.h
    3d/qgs3dmapconfigwidget.h
    3d/qgs3dmaptool.h
    3d/qgs3dmaptoolidentify.h
    3d/qgslightswidget.h
    3d/qgsline3dsymbolwidget.h
    3d/qgsmesh3dsymbolwidget.h
    3d/qgspoint3dsymbolwidget.h
    3d/qgspolygon3dsymbolwidget.h
    3d/qgsphongmaterialwidget.h
    3d/qgsrulebased3drendererwidget.h
    3d/qgssymbol3dwidget.h
    3d/qgsvectorlayer3drendererwidget.h
    3d/qgsmeshlayer3drendererwidget.h
    layout/qgslayout3dmapwidget.h
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

    SET (QGIS_APP_MOC_HDRS
      ${QGIS_APP_MOC_HDRS}
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_canvas.h
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_magnifier.h
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_panner.h
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_picker.h
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_plot.h
      ${CMAKE_SOURCE_DIR}/external/qwtpolar-1.1.1/qwt_polar_renderer.h
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

IF (PROJ_HAS_INFO)
  ADD_DEFINITIONS(-DPROJ_HAS_INFO)
ENDIF (PROJ_HAS_INFO)

# Test data dir for QgsAppScreenShots
ADD_DEFINITIONS(-DTEST_DATA_DIR="${TEST_DATA_DIR}")


SET(IMAGE_RCCS ../../images/images.qrc)

QT5_ADD_RESOURCES(IMAGE_RCC_SRCS ${IMAGE_RCCS})

# add test resources, e.g. standard test font
SET(TEST_RCCS ../../tests/testdata/testdata.qrc)
QT5_ADD_RESOURCES(TEST_RCC_SRCS ${TEST_RCCS})

QT5_WRAP_CPP(QGIS_APP_MOC_SRCS ${QGIS_APP_MOC_HDRS})

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

  ${CMAKE_SOURCE_DIR}/src/app
  ${CMAKE_SOURCE_DIR}/src/app/decorations
  ${CMAKE_SOURCE_DIR}/src/app/layout
  ${CMAKE_SOURCE_DIR}/src/app/pluginmanager
  ${CMAKE_SOURCE_DIR}/src/app/gps
  ${CMAKE_SOURCE_DIR}/src/app/dwg
  ${CMAKE_SOURCE_DIR}/src/app/mesh
  ${CMAKE_SOURCE_DIR}/src/app/locator
  ${CMAKE_SOURCE_DIR}/src/analysis
  ${CMAKE_SOURCE_DIR}/src/analysis/raster
  ${CMAKE_SOURCE_DIR}/src/analysis/mesh
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/annotations
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/gps
  ${CMAKE_SOURCE_DIR}/src/core/dxf
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/geocms/geonode
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/mesh
  ${CMAKE_SOURCE_DIR}/src/core/layertree
  ${CMAKE_SOURCE_DIR}/src/core/locator
  ${CMAKE_SOURCE_DIR}/src/core/providers/memory
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/scalebar
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/validity
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/symbology
  ${CMAKE_SOURCE_DIR}/src/gui/attributetable
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/gui/ogr
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
)
INCLUDE_DIRECTORIES(
  ../analysis/processing
  ../analysis/raster
  ../analysis/vector/geometry_checker
  ../core
  ../core/annotations
  ../core/auth
  ../core/gps
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
  ../plugins
  ../python
  gps
  dwg
  ${CMAKE_SOURCE_DIR}/external/libdxfrw
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
ADD_LIBRARY(qgis_app SHARED ${QGIS_APP_SRCS} ${QGIS_APP_MOC_SRCS} ${QGIS_APP_HDRS} ${QGIS_APP_MOC_HDRS})

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

IF (APPLE)
  INSTALL(CODE "EXECUTE_PROCESS(COMMAND /bin/echo -n \"APPLQGIS\" OUTPUT_FILE \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/../PkgInfo\")")
ENDIF (APPLE)
