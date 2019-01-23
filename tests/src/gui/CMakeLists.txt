# Standard includes and utils to compile into all tests.
SET (util_SRCS)


#####################################################
# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/tests/core #for render checker class
  ${CMAKE_SOURCE_DIR}/src/analysis/processing
  ${CMAKE_SOURCE_DIR}/src/gui
  ${CMAKE_SOURCE_DIR}/src/gui/auth
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets
  ${CMAKE_SOURCE_DIR}/src/gui/editorwidgets/core
  ${CMAKE_SOURCE_DIR}/src/gui/layout
  ${CMAKE_SOURCE_DIR}/src/gui/symbology
  ${CMAKE_SOURCE_DIR}/src/gui/processing
  ${CMAKE_SOURCE_DIR}/src/gui/raster
  ${CMAKE_SOURCE_DIR}/src/core
  ${CMAKE_SOURCE_DIR}/src/core/expression
  ${CMAKE_SOURCE_DIR}/src/core/auth
  ${CMAKE_SOURCE_DIR}/src/core/geometry
  ${CMAKE_SOURCE_DIR}/src/core/layout
  ${CMAKE_SOURCE_DIR}/src/core/metadata
  ${CMAKE_SOURCE_DIR}/src/core/processing
  ${CMAKE_SOURCE_DIR}/src/core/raster
  ${CMAKE_SOURCE_DIR}/src/core/symbology
  ${CMAKE_SOURCE_DIR}/src/core/fieldformatter
  ${CMAKE_SOURCE_DIR}/src/test
  ${CMAKE_SOURCE_DIR}/src/native

  ${CMAKE_BINARY_DIR}/src/analysis
  ${CMAKE_BINARY_DIR}/src/core
  ${CMAKE_BINARY_DIR}/src/gui
  ${CMAKE_BINARY_DIR}/src/ui
  ${CMAKE_BINARY_DIR}/src/ui/auth
  ${CMAKE_BINARY_DIR}/src/native
  ${CMAKE_CURRENT_BINARY_DIR}
)
INCLUDE_DIRECTORIES(SYSTEM
  ${QT_INCLUDE_DIR}
  ${GDAL_INCLUDE_DIR}
  ${QWT_INCLUDE_DIR}
  ${QCA_INCLUDE_DIR}
  ${QTKEYCHAIN_INCLUDE_DIR}
  ${QSCINTILLA_INCLUDE_DIR}
)

IF(ENABLE_MODELTEST)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/tests/qt_modeltest)
ENDIF(ENABLE_MODELTEST)

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
#SET(qgis_quickprinttest_SRCS testqgsquickprint.cpp ${util_SRCS})
#SET(qgis_quickprinttest_MOC_CPPS testqgsquickprint.cpp)
#QT5_WRAP_CPP(qgis_quickprinttest_MOC_SRCS ${qgis_quickprinttest_MOC_CPPS})
#ADD_CUSTOM_TARGET(qgis_quickprinttestmoc ALL DEPENDS ${qgis_quickprinttest_MOC_SRCS})
#ADD_EXECUTABLE(qgis_quickprinttest ${qgis_quickprinttest_SRCS})
#ADD_DEPENDENCIES(qgis_quickprinttest qgis_quickprinttestmoc)
#TARGET_LINK_LIBRARIES(qgis_quickprinttest ${QT_LIBRARIES} qgis_core qgis_gui)
#SET_TARGET_PROPERTIES(qgis_quickprinttest
#  PROPERTIES INSTALL_RPATH ${QGIS_LIB_DIR}
#  INSTALL_RPATH_USE_LINK_PATH true)
#IF (APPLE)
#  # For Mac OS X, the executable must be at the root of the bundle's executable folder
#  INSTALL(TARGETS qgis_quickprinttest RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
#  ADD_TEST(qgis_quickprinttest ${CMAKE_INSTALL_PREFIX}/qgis_quickprinttest)
#ELSE (APPLE)
#  INSTALL(TARGETS qgis_quickprinttest RUNTIME DESTINATION ${QGIS_BIN_DIR})
#  ADD_TEST(qgis_quickprinttest ${CMAKE_INSTALL_PREFIX}/bin/qgis_quickprinttest)
#ENDIF (APPLE)

MACRO (ADD_QGIS_TEST testname testsrc)
  SET(qgis_${testname}_SRCS ${testsrc} ${util_SRCS})
  SET(qgis_${testname}_MOC_CPPS ${testsrc})
  ADD_EXECUTABLE(qgis_${testname} ${qgis_${testname}_SRCS})
  SET_TARGET_PROPERTIES(qgis_${testname} PROPERTIES AUTOMOC TRUE)
  SET_TARGET_PROPERTIES(qgis_${testname} PROPERTIES AUTORCC TRUE)
  TARGET_LINK_LIBRARIES(qgis_${testname}
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
    qgis_gui)
  ADD_TEST(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname} -maxwarnings 10000)
  TARGET_LINK_LIBRARIES(qgis_${testname} qgis_native)
  ADD_TEST(qgis_${testname} ${CMAKE_CURRENT_BINARY_DIR}/../../../output/bin/qgis_${testname})
  #SET_TARGET_PROPERTIES(qgis_${testname} PROPERTIES
  #  INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/${QGIS_LIB_DIR}
  #  INSTALL_RPATH_USE_LINK_PATH true )
ENDMACRO (ADD_QGIS_TEST)

ADD_QGIS_TEST(zoomtest testqgsmaptoolzoom.cpp)
ADD_QGIS_TEST(edittooltest testqgsmaptooledit.cpp)

# a simple app for testing GUI of renderers
# These tests are old and are never run so removed for now.
#SET(rendererv2gui_SRCS testrenderergui.cpp)
#SET(rendererv2gui_HDRS testrenderergui.h)
#QT5_WRAP_CPP(rendererv2gui_MOC_SRCS ${rendererv2gui_HDRS})
#ADD_EXECUTABLE(qgis_rendererv2gui ${rendererv2gui_SRCS} ${rendererv2gui_MOC_SRCS})

#ADD_QGIS_TEST(histogramtest testqgsrasterhistogram.cpp)
ADD_QGIS_TEST(categorizedrendererwidget testqgscategorizedrendererwidget.cpp)
ADD_QGIS_TEST(doublespinbox testqgsdoublespinbox.cpp)
ADD_QGIS_TEST(dualviewtest testqgsdualview.cpp)
ADD_QGIS_TEST(attributeformtest testqgsattributeform.cpp)
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
ADD_QGIS_TEST(valuerelationwidgetwrapper testqgsvaluerelationwidgetwrapper.cpp)
ADD_QGIS_TEST(relationreferencewidget testqgsrelationreferencewidget.cpp)
ADD_QGIS_TEST(featurelistcombobox testqgsfeaturelistcombobox.cpp)
