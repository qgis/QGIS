# Standard includes and utils to compile into all tests.
set (util_SRCS)


#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
include_directories(
  ${CMAKE_SOURCE_DIR}/tests/src/core #for render checker class
  ${CMAKE_SOURCE_DIR}/src/test

  ${CMAKE_CURRENT_BINARY_DIR}
)

if(ENABLE_MODELTEST)
  include_directories(${CMAKE_SOURCE_DIR}/tests/qt_modeltest)
endif()

#note for tests we should not include the moc of our
#qtests in the executable file list as the moc is
#directly included in the sources
#and should not be compiled twice. Trying to include
#them in will cause an error at build time
#############################################################
# Tests:

#
# QgsQuickPrint test
#
#set(qgis_quickprinttest_SRCS testqgsquickprint.cpp ${util_SRCS})
#set(qgis_quickprinttest_MOC_CPPS testqgsquickprint.cpp)
#QT5_WRAP_CPP(qgis_quickprinttest_MOC_SRCS ${qgis_quickprinttest_MOC_CPPS})
#add_custom_target(qgis_quickprinttestmoc ALL DEPENDS ${qgis_quickprinttest_MOC_SRCS})
#add_executable(qgis_quickprinttest ${qgis_quickprinttest_SRCS})
#add_dependencies(qgis_quickprinttest qgis_quickprinttestmoc)
#target_link_libraries(qgis_quickprinttest ${QT_LIBRARIES} qgis_core qgis_gui)
#set_target_properties(qgis_quickprinttest
#  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
#  INSTALL_RPATH_USE_LINK_PATH true)
#if (APPLE)
#  # For Mac OS X, the executable must be at the root of the bundle's executable folder
#  install(TARGETS qgis_quickprinttest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
#  add_test(qgis_quickprinttest ${CMAKE_INSTALL_PREFIX}/qgis_quickprinttest)
#else()
#  install(TARGETS qgis_quickprinttest RUNTIME DESTINATION ${QGIS_BIN_DIR})
#  add_test(qgis_quickprinttest ${CMAKE_INSTALL_PREFIX}/bin/qgis_quickprinttest)
#endif()

macro (ADD_QGIS_TEST testname testsrc)
  set(qgis_${testname}_SRCS ${testsrc} ${util_SRCS})
  set(qgis_${testname}_MOC_CPPS ${testsrc})
  add_executable(qgis_${testname} ${qgis_${testname}_SRCS})
  target_compile_features(qgis_${testname} PRIVATE cxx_std_17)
  set_target_properties(qgis_${testname} PROPERTIES AUTORCC TRUE)
  target_link_libraries(qgis_${testname}
    ${Qt5Xml_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${OPTIONAL_QTWEBKIT}
    ${PROJ_LIBRARY}
    ${GEOS_LIBRARY}
    ${GDAL_LIBRARY}
    ${QWT_LIBRARY}
    qgis_analysis
    qgis_core
    qgis_gui
    qgis_native)
  add_test(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
  target_link_libraries(qgis_${testname} qgis_native)
  add_test(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname})
  #set_target_properties(qgis_${testname} PROPERTIES
  #  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  #  INSTALL_RPATH_USE_LINK_PATH true )
endmacro (ADD_QGIS_TEST)

ADD_QGIS_TEST(zoomtest testqgsmaptoolzoom.cpp)
ADD_QGIS_TEST(edittooltest testqgsmaptooledit.cpp)

# a simple app for testing GUI of renderers
# These tests are old and are never run so removed for now.
#set(rendererv2gui_SRCS testrenderergui.cpp)
#set(rendererv2gui_HDRS testrenderergui.h)
#QT5_WRAP_CPP(rendererv2gui_MOC_SRCS ${rendererv2gui_HDRS})
#add_executable(qgis_rendererv2gui ${rendererv2gui_SRCS} ${rendererv2gui_MOC_SRCS})

#ADD_QGIS_TEST(histogramtest testqgsrasterhistogram.cpp)
ADD_QGIS_TEST(categorizedrendererwidget testqgscategorizedrendererwidget.cpp)
ADD_QGIS_TEST(datumtransformdialog testqgsdatumtransformdialog.cpp)
ADD_QGIS_TEST(doublespinbox testqgsdoublespinbox.cpp)
ADD_QGIS_TEST(dualviewtest testqgsdualview.cpp)
ADD_QGIS_TEST(attributeformtest testqgsattributeform.cpp)
ADD_QGIS_TEST(datetimedittest testqgsdatetimeedit.cpp)
ADD_QGIS_TEST(dockwidget testqgsdockwidget.cpp)
ADD_QGIS_TEST(fieldexpressionwidget testqgsfieldexpressionwidget.cpp)
ADD_QGIS_TEST(filewidget testqgsfilewidget.cpp)
ADD_QGIS_TEST(focuswatcher testqgsfocuswatcher.cpp)
ADD_QGIS_TEST(mapcanvastest testqgsmapcanvas.cpp)
ADD_QGIS_TEST(messagebartest testqgsmessagebar.cpp)
ADD_QGIS_TEST(projectionissues testprojectionissues.cpp)
ADD_QGIS_TEST(qgsguitest testqgsgui.cpp)
ADD_QGIS_TEST(processingguitest testprocessinggui.cpp)
ADD_QGIS_TEST(processingmodeltest testqgsprocessingmodel.cpp)
ADD_QGIS_TEST(rubberbandtest testqgsrubberband.cpp)
ADD_QGIS_TEST(scalecombobox testqgsscalecombobox.cpp)
ADD_QGIS_TEST(scalerangewidget testqgsscalerangewidget.cpp)
ADD_QGIS_TEST(rangewidgetwrapper testqgsrangewidgetwrapper.cpp)
ADD_QGIS_TEST(spinbox testqgsspinbox.cpp)
ADD_QGIS_TEST(sqlcomposerdialog testqgssqlcomposerdialog.cpp)
ADD_QGIS_TEST(editorwidgetregistrytest testqgseditorwidgetregistry.cpp)
ADD_QGIS_TEST(keyvaluewidgettest testqgskeyvaluewidget.cpp)
ADD_QGIS_TEST(listwidgettest testqgslistwidget.cpp)
ADD_QGIS_TEST(filedownloader testqgsfiledownloader.cpp)
ADD_QGIS_TEST(layoutgui testqgslayoutgui.cpp)
ADD_QGIS_TEST(layoutview testqgslayoutview.cpp)
ADD_QGIS_TEST(valuemapwidgetwrapper testqgsvaluemapwidgetwrapper.cpp)
ADD_QGIS_TEST(rasterlayersaveasdialog testqgsrasterlayersaveasdialog.cpp)
ADD_QGIS_TEST(valuerelationwidgetwrapper testqgsvaluerelationwidgetwrapper.cpp)
ADD_QGIS_TEST(relationreferencewidget testqgsrelationreferencewidget.cpp)
ADD_QGIS_TEST(featurelistcombobox testqgsfeaturelistcombobox.cpp)
ADD_QGIS_TEST(texteditwrapper testqgstexteditwrapper.cpp)
ADD_QGIS_TEST(tableeditorwidget testqgstableeditor.cpp)
ADD_QGIS_TEST(newdatabasetablewidget testqgsnewdatabasetablewidget.cpp)
ADD_QGIS_TEST(ogrproviderguitest testqgsogrprovidergui.cpp)
ADD_QGIS_TEST(singlebandpseudocolorrendererwidget testqgssinglebandpseudocolorrendererwidget.cpp)
ADD_QGIS_TEST(doublevalidator testqgsdoublevalidator.cpp)
ADD_QGIS_TEST(meshlayerpropertiesdialog testqgsmeshlayerpropertiesdialog.cpp)
ADD_QGIS_TEST(externalresourcewidgetwrapper testqgsexternalresourcewidgetwrapper.cpp)

SET_TESTS_PROPERTIES(qgis_listwidgettest PROPERTIES LABELS "POSTGRES")
if (ENABLE_PGTEST)
  SET_TESTS_PROPERTIES(qgis_processingguitest qgis_valuerelationwidgetwrapper qgis_texteditwrapper
    qgis_newdatabasetablewidget
    PROPERTIES LABELS "POSTGRES")
endif()
