#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_SOURCE_DIR}/tests/src/gui
  ${CMAKE_CURRENT_BINARY_DIR}
)



set(TESTS
  testqgisapp.cpp
  testqgisappwidgetnames.cpp
  testqgsappbrowserproviders.cpp
  testqgisappclipboard.cpp
  testqgisappdockwidgets.cpp
  testqgsversioninfo.cpp
  testqgsqgisprofileimporter.cpp
  testqgsadvanceddigitizing.cpp
  testqgsattributetable.cpp
  testqgsapplocatorfilters.cpp
  testqgscustomization.cpp
  testqgsdecorationscalebar.cpp
  testqgsdwgimportdialog.cpp
  testqgselevationprofilewidget.cpp
  testqgsfieldcalculator.cpp
  testqgsidentify.cpp
  testqgslayerpropertiesdialogs.cpp
  testqgsmapcanvasdockwidget.cpp
  testqgsmapsavedialog.cpp
  testqgsmaptooladdpart.cpp
  testqgsmaptooldeletepart.cpp
  testqgsmaptooladdring.cpp
  testqgsmaptooldeletering.cpp
  testqgsmaptooleditannotation.cpp
  testqgsmaptoollabel.cpp
  testqgsmaptoolselect.cpp
  testqgsmaptoolreshape.cpp
  testqgsmaptoolrotatefeature.cpp
  testqgsmaptoolselectannotation.cpp
  testqgsmaptoolscalefeature.cpp
  testqgsmaptoolcircularstring.cpp
  testqgsmaptooleditmesh.cpp
  testqgsmaptoolmovefeature.cpp
  testqgsmaptoolfeaturearray.cpp
  testqgsmaptoolellipse.cpp
  testqgsmaptoolcircle.cpp
  testqgsmaptoolnurbs.cpp
  testqgsmaptoolrectangle.cpp
  testqgsmaptoolregularpolygon.cpp
  testqgsmaptoolsplitparts.cpp
  testqgsmaptoolsplitfeatures.cpp
  testqgsmaptooloffsetcurve.cpp
  testqgsmaptoolchamferfillet.cpp
  testqgsmeasuretool.cpp
  testqgsmeasurebearingtool.cpp
  testqgsvertexeditor.cpp
  testqgsvertextool.cpp
  testqgsvectorlayersaveasdialog.cpp
  testqgsmaptoolreverseline.cpp
  testqgsmaptooltrimextendfeature.cpp
  testqgsprojectproperties.cpp
  testqgsapplayoutvaliditychecks.cpp
  testqgsmeshcalculatordialog.cpp
  testqgsgpsintegration.cpp
  testqgslabelpropertydialog.cpp
  testqgsmergeattributesdialog.cpp
)

if (ENABLE_AI_ASSISTANT)
  set(TESTS ${TESTS} testqgsaireviewpatchengine.cpp)
  set(TESTS ${TESTS} testqgsaimodelrouter.cpp)
  set(TESTS ${TESTS} testqgsaiagentsessionmanager.cpp)
  set(TESTS ${TESTS} testqgsaichathistorystore.cpp)
  set(TESTS ${TESTS} testqgsaichatdockwidget.cpp)
  set(TESTS ${TESTS} testqgsaigissuggestionengine.cpp)
  set(TESTS ${TESTS} testqgsaitoolregistry.cpp)
  set(TESTS ${TESTS} testqgsaimessagelogbuffer.cpp)
  set(TESTS ${TESTS} testqgsaiworkspaceindex.cpp)
  set(TESTS ${TESTS} testqgsaicloudindexclient.cpp)
  set(TESTS ${TESTS} testqgsaiembeddingclient.cpp)
  set(TESTS ${TESTS} testqgsailayerchunker.cpp)
  set(TESTS ${TESTS} testqgsailayerindexcoordinator.cpp)
  set(TESTS ${TESTS} testqgsaiopenroutermodelcatalog.cpp)
  set(TESTS ${TESTS} testqgsaiplanclient.cpp)
  set(TESTS ${TESTS} testqgsaipythonapprovaldialog.cpp)
  set(TESTS ${TESTS} testqgsaisecretstore.cpp)
endif()

if (HAVE_GEOREFERENCER)
  set(TESTS ${TESTS} testqgsgeoreferencer.cpp)
endif()

if (WITH_BINDINGS)
  set(TESTS ${TESTS} testqgisapppython.cpp)
  set(TESTS ${TESTS} testqgsprojectexpressions.cpp)
endif()

foreach(TESTSRC ${TESTS})
  add_qgis_test(${TESTSRC} MODULE app LINKEDLIBRARIES qgis_app)
endforeach(TESTSRC)

add_subdirectory(maptooladdfeaturepoint)
add_subdirectory(maptooladdfeatureline)
