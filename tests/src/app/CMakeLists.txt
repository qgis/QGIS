#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_CURRENT_BINARY_DIR}
)



set(TESTS
  testqgisapp.cpp
  testqgsappbrowserproviders.cpp
  testqgisappclipboard.cpp
  testqgisappdockwidgets.cpp
  testqgsattributetable.cpp
  testqgsapplocatorfilters.cpp
  testqgsdecorationscalebar.cpp
  testqgsfieldcalculator.cpp
  testqgsgeoreferencer.cpp
  testqgsmaptooleditannotation.cpp
  testqgsmaptoolidentifyaction.cpp
  testqgsmaptoollabel.cpp
  testqgsmaptoolselect.cpp
  testqgsmaptoolreshape.cpp
  testqgsmaptoolrotatefeature.cpp
  testqgsmaptoolscalefeature.cpp
  testqgsmaptoolcircularstring.cpp
  testqgsmaptooleditmesh.cpp
  testqgsmaptoolmovefeature.cpp
  testqgsmaptoolellipse.cpp
  testqgsmaptoolcircle.cpp
  testqgsmaptoolrectangle.cpp
  testqgsmaptoolregularpolygon.cpp
  testqgsmaptoolsplitparts.cpp
  testqgsmaptoolsplitfeatures.cpp
  testqgsmeasuretool.cpp
  testqgsmeasurebearingtool.cpp
  testqgsvertextool.cpp
  testqgsvectorlayersaveasdialog.cpp
  testqgsmaptoolreverseline.cpp
  testqgsmaptooltrimextendfeature.cpp
  testqgsprojectproperties.cpp
  testqgsapplayoutvaliditychecks.cpp
  testqgsmeshcalculatordialog.cpp
  testqgsgpsinformationwidget.cpp
  testqgslabelpropertydialog.cpp
)
if (WITH_BINDINGS)
  set(TESTS ${TESTS} testqgisapppython.cpp)
endif()

foreach(TESTSRC ${TESTS})
  add_qgis_test(${TESTSRC} MODULE app LINKEDLIBRARIES qgis_app)
endforeach(TESTSRC)

add_subdirectory(maptooladdfeaturepoint)
add_subdirectory(maptooladdfeatureline)
