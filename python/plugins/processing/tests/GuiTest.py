"""
***************************************************************************
    ParametersTest
    ---------------------
    Date                 : August 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "August 2017"
__copyright__ = "(C) 2017, Nyall Dawson"

import os
import tempfile
import unittest
from time import sleep
from qgis.testing import start_app, QgisTestCase
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsProcessingContext,
    QgsProcessingAlgorithm,
    QgsProcessingUtils,
    QgsCoordinateReferenceSystem,
    QgsProcessingParameterMatrix,
    QgsTaskManager,
    QgsProcessingParameterRange,
    QgsFeature,
    QgsProcessingModelAlgorithm,
    QgsUnitTypes,
    QgsProject,
)
from qgis.PyQt.QtWidgets import QMainWindow
from qgis.gui import QgisInterface, QgsMapCanvas, QgsMessageBar
from qgis.analysis import QgsNativeAlgorithms

from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.ProcessingPlugin import ProcessingPlugin
from processing.gui.BatchPanel import BatchPanel
from processing.gui.wrappers import (
    BandWidgetWrapper,
    BooleanWidgetWrapper,
    CrsWidgetWrapper,
    DistanceWidgetWrapper,
    EnumWidgetWrapper,
    ExpressionWidgetWrapper,
    ExtentWidgetWrapper,
    FeatureSourceWidgetWrapper,
    FileWidgetWrapper,
    FixedTableWidgetWrapper,
    MapLayerWidgetWrapper,
    MeshWidgetWrapper,
    MultipleLayerWidgetWrapper,
    NumberWidgetWrapper,
    PointWidgetWrapper,
    ProcessingConfig,
    QgsProcessingFeatureSourceDefinition,
    QgsProcessingParameterBand,
    QgsProcessingParameterBoolean,
    QgsProcessingParameterCrs,
    QgsProcessingParameterDistance,
    QgsProcessingParameterDuration,
    QgsProcessingParameterEnum,
    QgsProcessingParameterExpression,
    QgsProcessingParameterExtent,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterField,
    QgsProcessingParameterFile,
    QgsProcessingParameterMapLayer,
    QgsProcessingParameterMeshLayer,
    QgsProcessingParameterMultipleLayers,
    QgsProcessingParameterNumber,
    QgsProcessingParameterPoint,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterString,
    QgsProcessingParameterVectorLayer,
    QgsVectorLayer,
    RangeWidgetWrapper,
    RasterWidgetWrapper,
    StringWidgetWrapper,
    TableFieldWidgetWrapper,
    VectorLayerWidgetWrapper,
    WidgetWrapperFactory,
)

start_app()
QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())

testDataPath = os.path.join(os.path.dirname(__file__), "testdata")


class AlgorithmDialogTest(QgisTestCase):

    def testCreation(self):
        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:centroids"
        )
        a = AlgorithmDialog(alg)
        self.assertEqual(a.mainWidget().algorithm(), alg)

    def testIndividualProcessingContext(self):
        """
        Test if we can use a self-defined QgsProcessingContext to provide input layers
        Addresses https://github.com/qgis/QGIS/issues/60764
        """
        with tempfile.TemporaryDirectory() as tempdir:

            context_normal = QgsProcessingContext()
            context_normal.setProject(QgsProject.instance())

            lyr0 = QgsVectorLayer(f"{testDataPath}/points_lines.gpkg|layername=lines")
            lyr1 = QgsVectorLayer(f"{testDataPath}/points_lines.gpkg|layername=points")
            self.assertTrue(
                lyr0.isValid() and lyr0.wkbType() == Qgis.WkbType.LineString
            )
            self.assertTrue(lyr1.isValid() and lyr1.wkbType() == Qgis.WkbType.Point)

            # add polygon layer to QgsProject.instance()
            # and use the point layer in user-defined context only.
            # algorithm results with polygons show that the user-context was not used right
            QgsProject.instance().addMapLayer(lyr0)

            def create_context_project():
                p = QgsProject()
                p.setTitle("MyProject")
                p.addMapLayer(lyr1)

                c = QgsProcessingContext()
                c.setProject(p)
                return c, p

            if True:
                myContext1, myProject1 = create_context_project()
                alg = QgsApplication.processingRegistry().createAlgorithmById(
                    "native:savefeatures"
                )
                self.assertIsInstance(alg, QgsProcessingAlgorithm)

                # test AlgorithmDialog
                d1 = AlgorithmDialog(alg, context=myContext1)
                d1.runAlgorithm()
                # wait until algorithm has been executed
                while not d1.wasExecuted():
                    QgsApplication.instance().processEvents()

                result = d1.results()

                result_lyr = QgsProcessingUtils.mapLayerFromString(
                    result["OUTPUT"], myContext1
                )

                self.assertEqual(len(QgsProject.instance().mapLayers()), 1)
                self.assertIsInstance(result_lyr, QgsVectorLayer)
                self.assertTrue(result_lyr.isValid())
                self.assertEqual(
                    result_lyr.wkbType(),
                    Qgis.WkbType.Point,
                    msg=f"Expected point geometry, but got {result_lyr.wkbType()}. "
                    f"Default input from wrong processing context?",
                )

                # d.close()

            # test BatchAlgorithmDialog
            if True:
                myContext2, myProject2 = create_context_project()
                alg2 = QgsApplication.processingRegistry().createAlgorithmById(
                    "native:savefeatures"
                )
                d2 = BatchAlgorithmDialog(alg2, context=myContext2)
                panel: BatchPanel = d2.mainWidget()
                panel.show()
                for r in range(3):
                    panel.addRow()

                result_paths = []
                for row in range(len(panel.wrappers)):
                    col = panel.parameter_to_column["OUTPUT"]
                    widget = panel.tblParameters.cellWidget(row + 1, col)
                    expected_path = f"{tempdir}/myoutput{row + 1}.geojson"
                    widget.setValue(expected_path)
                    result_paths.append(expected_path)

                d2.runAlgorithm()
                # d.exec_()
                tm: QgsTaskManager = QgsApplication.instance().taskManager()
                # wait until algorithms have been executed

                QgsApplication.instance().processEvents()
                while tm.count() > 0:
                    sleep(1)
                    QgsApplication.instance().processEvents()

                self.assertEqual(len(QgsProject.instance().mapLayers()), 1)
                self.assertEqual(len(result_paths), 4)
                for p in result_paths:
                    layer = QgsVectorLayer(p)
                    self.assertTrue(layer.isValid())
                    self.assertEqual(layer.wkbType(), Qgis.WkbType.Point)
                    del layer

            # cleanup
            QgsProject.instance().removeAllMapLayers()

    @unittest.skipIf(
        QgisTestCase.is_ci_run(), "Blocking widgets that require a user interaction"
    )
    def testProcessingPlugin_executeAlgorithm(self):

        lyr0 = QgsVectorLayer(f"{testDataPath}/points_lines.gpkg|layername=lines")
        lyr1 = QgsVectorLayer(f"{testDataPath}/points_lines.gpkg|layername=points")

        lyr0.selectByIds(lyr0.allFeatureIds()[0:2])

        lyr1.selectByIds(lyr1.allFeatureIds()[0:3])

        n_selected0 = lyr0.selectedFeatureCount()
        n_selected1 = lyr1.selectedFeatureCount()

        class MyInterface(QgisInterface):
            def __init__(self):
                super().__init__()
                self.mCanvas = QgsMapCanvas()
                self.mProject = QgsProject.instance()
                self.mMainWindow = QMainWindow()
                self.mActiveLayer = None
                self.mMessageBar = QgsMessageBar()
                # self.mMainWindow.layout().addWidget(self.mCanvas)

            def project(self) -> QgsProject:
                return self.mProject

            def messageBar(self):
                return self.mMessageBar

            def mapCanvas(self):
                return self.mCanvas

            def mainWindow(self) -> QMainWindow:
                return self.mMainWindow

            def setActiveLayer(self, lyr):
                self.mActiveLayer = lyr

            def activeLayer(self):
                return self.mActiveLayer

        normalIface = MyInterface()
        normalIface.mainWindow().setWindowTitle("Normal Iface")
        normalIface.mainWindow().show()
        normalIface.project().addMapLayer(lyr0)
        normalIface.setActiveLayer(lyr0)

        myIface = MyInterface()
        myIface.mainWindow().setWindowTitle("MyIface")
        myIface.mProject = QgsProject()
        myIface.mProject.setTitle("My Project")
        myIface.project().addMapLayer(lyr1)
        myIface.mainWindow().show()

        p = ProcessingPlugin(normalIface)

        # this algorithm create a copy of the selected features
        # the iface.activeLayer() is automatically chosen as INPUT
        alg_id = "native:saveselectedfeatures"

        myContext = QgsProcessingContext()
        myContext.setProject(myIface.project())

        self.assertEqual(len(myIface.project().mapLayers()), 1)
        self.assertEqual(len(normalIface.project().mapLayers()), 1)

        # run algorithm with standard iface
        # requires to click "run", then "close" manually
        p.executeAlgorithm(alg_id, None, in_place=False, as_batch=False)

        self.assertEqual(len(normalIface.project().mapLayers()), 2)
        self.assertEqual(len(myIface.project().mapLayers()), 1)
        for lyr in normalIface.project().mapLayers().values():
            if lyr != lyr0:
                self.assertIsInstance(lyr, QgsVectorLayer)
                self.assertEqual(lyr.wkbType(), lyr0.wkbType())
                self.assertEqual(lyr.featureCount(), n_selected0)

        # run algorithm with none-standard iface & context
        # requires to click "run", then "close" manually
        p.executeAlgorithm(
            alg_id,
            None,
            in_place=False,
            as_batch=False,
            context=myContext,
            iface=myIface,
        )

        self.assertEqual(len(myIface.project().mapLayers()), 2)
        self.assertEqual(len(normalIface.project().mapLayers()), 2)

        for lyr in myIface.project().mapLayers().values():
            if lyr != lyr1:
                self.assertIsInstance(lyr, QgsVectorLayer)
                self.assertEqual(lyr.wkbType(), lyr1.wkbType())
                self.assertEqual(lyr.featureCount(), n_selected1)


class WrappersTest(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        ProcessingConfig.initialize()

    def checkConstructWrapper(self, param, expected_wrapper_class):
        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:centroids"
        )

        # algorithm dialog
        dlg = AlgorithmDialog(alg)
        wrapper = WidgetWrapperFactory.create_wrapper_from_class(param, dlg)
        self.assertIsNotNone(wrapper)
        self.assertIsInstance(wrapper, expected_wrapper_class)
        self.assertEqual(wrapper.dialog, dlg)
        self.assertIsNotNone(wrapper.widget)
        wrapper.widget.deleteLater()
        del wrapper.widget
        del wrapper

        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:centroids"
        )
        # batch dialog
        dlg = BatchAlgorithmDialog(alg)
        wrapper = WidgetWrapperFactory.create_wrapper_from_class(param, dlg)
        self.assertIsNotNone(wrapper)
        self.assertIsInstance(wrapper, expected_wrapper_class)
        self.assertEqual(wrapper.dialog, dlg)
        self.assertIsNotNone(wrapper.widget)

        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:centroids"
        )

        # modeler dialog
        model = QgsProcessingModelAlgorithm()
        dlg = ModelerParametersDialog(alg, model)
        wrapper = WidgetWrapperFactory.create_wrapper_from_class(param, dlg)
        self.assertIsNotNone(wrapper)
        self.assertIsInstance(wrapper, expected_wrapper_class)
        self.assertEqual(wrapper.dialog, dlg)
        self.assertIsNotNone(wrapper.widget)

        wrapper.widget.deleteLater()
        del wrapper.widget

    def testBoolean(self):
        self.checkConstructWrapper(
            QgsProcessingParameterBoolean("test"), BooleanWidgetWrapper
        )

    def testCrs(self):
        self.checkConstructWrapper(QgsProcessingParameterCrs("test"), CrsWidgetWrapper)

    def testExtent(self):
        self.checkConstructWrapper(
            QgsProcessingParameterExtent("test"), ExtentWidgetWrapper
        )

    def testPoint(self):
        self.checkConstructWrapper(
            QgsProcessingParameterPoint("test"), PointWidgetWrapper
        )

    def testFile(self):
        self.checkConstructWrapper(
            QgsProcessingParameterFile("test"), FileWidgetWrapper
        )

    def testMultiInput(self):
        self.checkConstructWrapper(
            QgsProcessingParameterMultipleLayers("test"), MultipleLayerWidgetWrapper
        )

    def testRasterInput(self):
        self.checkConstructWrapper(
            QgsProcessingParameterRasterLayer("test"), RasterWidgetWrapper
        )

    def testEnum(self):
        self.checkConstructWrapper(
            QgsProcessingParameterEnum("test"), EnumWidgetWrapper
        )

    def testString(self):
        self.checkConstructWrapper(
            QgsProcessingParameterString("test"), StringWidgetWrapper
        )

    def testExpression(self):
        self.checkConstructWrapper(
            QgsProcessingParameterExpression("test"), ExpressionWidgetWrapper
        )

    def testVector(self):
        self.checkConstructWrapper(
            QgsProcessingParameterVectorLayer("test"), VectorLayerWidgetWrapper
        )

    def testField(self):
        self.checkConstructWrapper(
            QgsProcessingParameterField("test"), TableFieldWidgetWrapper
        )

    def testSource(self):
        self.checkConstructWrapper(
            QgsProcessingParameterFeatureSource("test"), FeatureSourceWidgetWrapper
        )

        # dummy layer
        layer = QgsVectorLayer("Point", "test", "memory")
        # need at least one feature in order to have a selection
        layer.dataProvider().addFeature(QgsFeature())
        layer.selectAll()

        self.assertTrue(layer.isValid())
        QgsProject.instance().addMapLayer(layer)

        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:centroids"
        )
        dlg = AlgorithmDialog(alg)
        param = QgsProcessingParameterFeatureSource("test")
        wrapper = FeatureSourceWidgetWrapper(param, dlg)
        widget = wrapper.createWidget()

        # check layer value
        widget.show()
        wrapper.setValue(layer.id())
        self.assertEqual(wrapper.value(), layer.id())

        # check selected only - expect a QgsProcessingFeatureSourceDefinition
        wrapper.setValue(QgsProcessingFeatureSourceDefinition(layer.id(), True))
        value = wrapper.value()
        self.assertIsInstance(value, QgsProcessingFeatureSourceDefinition)
        self.assertTrue(value.selectedFeaturesOnly)
        self.assertEqual(value.source.staticValue(), layer.id())

        # NOT selected only, expect a direct layer id or source value
        wrapper.setValue(QgsProcessingFeatureSourceDefinition(layer.id(), False))
        value = wrapper.value()
        self.assertEqual(value, layer.id())

        # with non-project layer
        wrapper.setValue("/home/my_layer.shp")
        value = wrapper.value()
        self.assertEqual(value, "/home/my_layer.shp")

        widget.deleteLater()
        del widget

    def testRange(self):
        # minimal test to check if wrapper generate GUI for each processign context
        self.checkConstructWrapper(
            QgsProcessingParameterRange("test"), RangeWidgetWrapper
        )

        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:centroids"
        )
        dlg = AlgorithmDialog(alg)
        param = QgsProcessingParameterRange(
            name="test",
            description="test",
            type=QgsProcessingParameterNumber.Type.Double,
            defaultValue="0.0,100.0",
        )

        wrapper = RangeWidgetWrapper(param, dlg)
        widget = wrapper.createWidget()

        # range values check

        # check initial value
        self.assertEqual(widget.getValue(), "0.0,100.0")
        # check set/get
        widget.setValue("100.0,200.0")
        self.assertEqual(widget.getValue(), "100.0,200.0")
        # check that min/max are mutually adapted
        widget.setValue("200.0,100.0")
        self.assertEqual(widget.getValue(), "100.0,100.0")
        widget.spnMax.setValue(50)
        self.assertEqual(widget.getValue(), "50.0,50.0")
        widget.spnMin.setValue(100)
        self.assertEqual(widget.getValue(), "100.0,100.0")

        # check for integers
        param = QgsProcessingParameterRange(
            name="test",
            description="test",
            type=QgsProcessingParameterNumber.Type.Integer,
            defaultValue="0.1,100.1",
        )

        wrapper = RangeWidgetWrapper(param, dlg)
        widget = wrapper.createWidget()

        # range values check

        # check initial value
        self.assertEqual(widget.getValue(), "0.0,100.0")
        # check rounding
        widget.setValue("100.1,200.1")
        self.assertEqual(widget.getValue(), "100.0,200.0")
        widget.setValue("100.6,200.6")
        self.assertEqual(widget.getValue(), "101.0,201.0")
        # check set/get
        widget.setValue("100.1,200.1")
        self.assertEqual(widget.getValue(), "100.0,200.0")
        # check that min/max are mutually adapted
        widget.setValue("200.1,100.1")
        self.assertEqual(widget.getValue(), "100.0,100.0")
        widget.spnMax.setValue(50.1)
        self.assertEqual(widget.getValue(), "50.0,50.0")
        widget.spnMin.setValue(100.1)
        self.assertEqual(widget.getValue(), "100.0,100.0")

    def testMapLayer(self):
        self.checkConstructWrapper(
            QgsProcessingParameterMapLayer("test"), MapLayerWidgetWrapper
        )

    def testMeshLayer(self):
        self.checkConstructWrapper(
            QgsProcessingParameterMeshLayer("test"), MeshWidgetWrapper
        )

    def testDistance(self):
        self.checkConstructWrapper(
            QgsProcessingParameterDistance("test"), DistanceWidgetWrapper
        )

        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:centroids"
        )
        dlg = AlgorithmDialog(alg)
        param = QgsProcessingParameterDistance("test")
        wrapper = DistanceWidgetWrapper(param, dlg)
        widget = wrapper.createWidget()

        # test units
        widget.show()

        # crs values
        widget.setUnitParameterValue("EPSG:3111")
        self.assertEqual(widget.label.text(), "meters")
        self.assertFalse(widget.warning_label.isVisible())
        self.assertTrue(widget.units_combo.isVisible())
        self.assertFalse(widget.label.isVisible())
        self.assertEqual(
            widget.units_combo.currentData(), QgsUnitTypes.DistanceUnit.DistanceMeters
        )

        widget.setUnitParameterValue("EPSG:4326")
        self.assertEqual(widget.label.text(), "degrees")
        self.assertTrue(widget.warning_label.isVisible())
        self.assertFalse(widget.units_combo.isVisible())
        self.assertTrue(widget.label.isVisible())

        widget.setUnitParameterValue(QgsCoordinateReferenceSystem("EPSG:3111"))
        self.assertEqual(widget.label.text(), "meters")
        self.assertFalse(widget.warning_label.isVisible())
        self.assertTrue(widget.units_combo.isVisible())
        self.assertFalse(widget.label.isVisible())
        self.assertEqual(
            widget.units_combo.currentData(), QgsUnitTypes.DistanceUnit.DistanceMeters
        )

        widget.setUnitParameterValue(QgsCoordinateReferenceSystem("EPSG:4326"))
        self.assertEqual(widget.label.text(), "degrees")
        self.assertTrue(widget.warning_label.isVisible())
        self.assertFalse(widget.units_combo.isVisible())
        self.assertTrue(widget.label.isVisible())

        # layer values
        vl = QgsVectorLayer("Polygon?crs=epsg:3111&field=pk:int", "vl", "memory")
        widget.setUnitParameterValue(vl)
        self.assertEqual(widget.label.text(), "meters")
        self.assertFalse(widget.warning_label.isVisible())
        self.assertTrue(widget.units_combo.isVisible())
        self.assertFalse(widget.label.isVisible())
        self.assertEqual(
            widget.units_combo.currentData(), QgsUnitTypes.DistanceUnit.DistanceMeters
        )

        vl2 = QgsVectorLayer("Polygon?crs=epsg:4326&field=pk:int", "vl", "memory")
        widget.setUnitParameterValue(vl2)
        self.assertEqual(widget.label.text(), "degrees")
        self.assertTrue(widget.warning_label.isVisible())
        self.assertFalse(widget.units_combo.isVisible())
        self.assertTrue(widget.label.isVisible())

        # unresolvable values
        widget.setUnitParameterValue(vl.id())
        self.assertEqual(widget.label.text(), "<unknown>")
        self.assertFalse(widget.warning_label.isVisible())
        self.assertFalse(widget.units_combo.isVisible())
        self.assertTrue(widget.label.isVisible())

        # resolvable text value
        QgsProject.instance().addMapLayer(vl)
        widget.setUnitParameterValue(vl.id())
        self.assertEqual(widget.label.text(), "meters")
        self.assertFalse(widget.warning_label.isVisible())
        self.assertTrue(widget.units_combo.isVisible())
        self.assertFalse(widget.label.isVisible())
        self.assertEqual(
            widget.units_combo.currentData(), QgsUnitTypes.DistanceUnit.DistanceMeters
        )

        widget.setValue(5)
        self.assertEqual(widget.getValue(), 5)
        widget.units_combo.setCurrentIndex(
            widget.units_combo.findData(QgsUnitTypes.DistanceUnit.DistanceKilometers)
        )
        self.assertEqual(widget.getValue(), 5000)
        widget.setValue(2)
        self.assertEqual(widget.getValue(), 2000)

        widget.setUnitParameterValue(vl.id())
        self.assertEqual(widget.getValue(), 2)
        widget.setValue(5)
        self.assertEqual(widget.getValue(), 5)

        widget.deleteLater()

    def testMatrix(self):
        self.checkConstructWrapper(
            QgsProcessingParameterMatrix("test"), FixedTableWidgetWrapper
        )

        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:centroids"
        )
        dlg = AlgorithmDialog(alg)
        param = QgsProcessingParameterMatrix(
            "test", "test", 2, True, ["x", "y"], [["a", "b"], ["c", "d"]]
        )
        wrapper = FixedTableWidgetWrapper(param, dlg)
        widget = wrapper.createWidget()

        # check that default value is initially set
        self.assertEqual(wrapper.value(), [["a", "b"], ["c", "d"]])

        # test widget
        widget.show()
        wrapper.setValue([[1, 2], [3, 4]])
        self.assertEqual(wrapper.value(), [[1, 2], [3, 4]])

        widget.deleteLater()

    def testNumber(self):
        self.checkConstructWrapper(
            QgsProcessingParameterNumber("test"), NumberWidgetWrapper
        )

    def testBand(self):
        self.checkConstructWrapper(
            QgsProcessingParameterBand("test"), BandWidgetWrapper
        )


if __name__ == "__main__":
    unittest.main()
