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

# a simple app for testing GUI of renderers
# These tests are old and are never run so removed for now.
#set(rendererv2gui_SRCS testrenderergui.cpp)
#set(rendererv2gui_HDRS testrenderergui.h)
#add_executable(qgis_rendererv2gui ${rendererv2gui_SRCS} ${rendererv2gui_MOC_SRCS})

#ADD_QGIS_TEST(histogramtest testqgsrasterhistogram.cpp)

set(TESTS
  testqgsadvanceddigitizingdockwidget.cpp
  testqgsannotationitemguiregistry.cpp
  testqgsmaptoolzoom.cpp
  testqgsmaptooledit.cpp
  testqgscategorizedrendererwidget.cpp
  testqgsdatumtransformdialog.cpp
  testqgsdoublespinbox.cpp
  testqgsdualview.cpp
  testqgsattributeform.cpp
  testqgsdatetimeedit.cpp
  testqgsdockwidget.cpp
  testqgsfieldexpressionwidget.cpp
  testqgsfilewidget.cpp
  testqgsexternalstoragefilewidget.cpp
  testqgsfocuswatcher.cpp
  testqgshtmlwidgetwrapper.cpp
  testqgsmapcanvas.cpp
  testqgsmaptoolcapture.cpp
  testqgsmessagebar.cpp
  testprojectionissues.cpp
  testqgsgui.cpp
  testprocessinggui.cpp
  testqgsprocessingmodel.cpp
  testqgsrubberband.cpp
  testqgsscalecombobox.cpp
  testqgsscalerangewidget.cpp
  testqgsrangewidgetwrapper.cpp
  testqgsspinbox.cpp
  testqgssqlcomposerdialog.cpp
  testqgseditorwidgetregistry.cpp
  testqgskeyvaluewidget.cpp
  testqgsfiledownloader.cpp
  testqgslayoutgui.cpp
  testqgslayoutview.cpp
  testqgsvaluemapwidgetwrapper.cpp
  testqgsvaluemapconfigdlg.cpp
  testqgsvaluerelationwidgetwrapper.cpp
  testqgsrelationeditorwidget.cpp
  testqgsrelationreferencewidget.cpp
  testqgsfeaturelistcombobox.cpp
  testqgstexteditwrapper.cpp
  testqgstableeditor.cpp
  testqgsnewdatabasetablewidget.cpp
  testqgsogrprovidergui.cpp
  testqgssinglebandpseudocolorrendererwidget.cpp
  testqgsdoublevalidator.cpp
  testqgsmeshlayerpropertiesdialog.cpp
  testqgsexternalresourcewidgetwrapper.cpp
  testqgsquerybuilder.cpp
  testqgsqueryresultwidget.cpp
)

foreach(TESTSRC ${TESTS})
  add_qgis_test( ${TESTSRC} MODULE gui LINKEDLIBRARIES qgis_gui qgis_analysis)
endforeach(TESTSRC)

add_qgis_test(testqgsrasterlayersaveasdialog.cpp MODULE gui LINKEDLIBRARIES qgis_gui qgis_analysis provider_postgres_gui_a)
add_qgis_test(testqgslistwidget.cpp MODULE gui LINKEDLIBRARIES qgis_gui qgis_analysis LABELS "POSTGRES")


if (ENABLE_PGTEST)
  SET_TESTS_PROPERTIES(
    test_gui_processinggui
    test_gui_valuerelationwidgetwrapper
    test_gui_texteditwrapper
    test_gui_newdatabasetablewidget
    test_gui_queryresultwidget
    PROPERTIES LABELS "POSTGRES")
endif()
