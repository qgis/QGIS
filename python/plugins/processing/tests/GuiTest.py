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
import unittest
from qgis.testing import start_app, QgisTestCase
from time import sleep
import tempfile

from processing.gui.BatchPanel import BatchPanel
from processing.gui.ParametersPanel import ParametersPanel
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
from qgis.analysis import QgsNativeAlgorithms

from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
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

        with tempfile.TemporaryDirectory() as tempdir:

            context_normal = QgsProcessingContext()
            context_normal.setProject(QgsProject.instance())

            lyr0 = QgsVectorLayer(f'{testDataPath}/custom/overlay1_a.geojson')
            lyr1 = QgsVectorLayer(f'{testDataPath}/custom/points.shp')
            self.assertTrue(lyr0.isValid())
            self.assertTrue(lyr1.isValid())

            # add polygon layer to QgsProject.instance()
            # and use the point layer in user-defined context only.
            # algorithm results with polygons show that the user-context was not used right
            QgsProject.instance().addMapLayer(lyr0)

            def create_context_project():
                p = QgsProject()
                p.setTitle('MyProject')
                p.addMapLayer(lyr1)

                c = QgsProcessingContext()
                c.setProject(p)
                return c, p

            myContext, myProject = create_context_project()

            alg = QgsApplication.processingRegistry().createAlgorithmById(
                "native:centroids"
            )
            self.assertIsInstance(alg, QgsProcessingAlgorithm)

            if False:
                # test AlgorithmDialog
                d = AlgorithmDialog(alg, context = myContext)
                d.runAlgorithm()
                # wait until algorithm has been executed
                while not d.wasExecuted():
                    QgsApplication.instance().processEvents()

                result = d.results()
                d.close()
                del d

                result_lyr1 = QgsProcessingUtils.mapLayerFromString(result['OUTPUT'], myContext)
                result_lyr2 = QgsProcessingUtils.mapLayerFromString(result['OUTPUT'], context_normal)

                self.assertEqual(len(QgsProject.instance().mapLayers()), 1)
                self.assertIsInstance(result_lyr1, QgsVectorLayer)
                self.assertTrue(result_lyr1.isValid())
                self.assertEqual(result_lyr1.wkbType(), Qgis.WkbType.Point)
                self.assertTrue(result_lyr2 is None)

                del context_normal
                del result_lyr1
                del result_lyr2

            # test BatchAlgorithmDialog
            if True:
                myContext, myProject = create_context_project()

                d = BatchAlgorithmDialog(alg)
                panel: BatchPanel = d.mainWidget()
                panel.show()
                for r in range(3):
                    panel.addRow()

                result_paths = []
                for row in range(len(panel.wrappers)):
                    col = panel.parameter_to_column['OUTPUT']
                    widget = panel.tblParameters.cellWidget(row + 1, col)
                    expected_path = f'{tempdir}/myoutput{row+1}.gpkg'
                    widget.setValue(expected_path)
                    result_paths.append(expected_path)

                d.runAlgorithm()
                d.exec_()
                tm: QgsTaskManager = QgsApplication.instance().taskManager()
                # wait until algorithms have been executed

                QgsApplication.instance().processEvents()
                while tm.count() > 0:
                    sleep(1)
                    QgsApplication.instance().processEvents()
                d.close()
                del d
                self.assertEqual(len(QgsProject.instance().mapLayers()), 1)
                for p in result_paths:
                    layer = QgsVectorLayer(p)
                    self.assertTrue(layer.isValid())
                    self.assertEqual(layer.wkbType(), Qgis.WkbType.Point)
                    del layer


            # cleanup
            QgsProject.instance().removeAllMapLayers()
            del lyr1
            del lyr0


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
