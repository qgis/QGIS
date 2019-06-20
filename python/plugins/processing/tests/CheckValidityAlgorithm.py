# -*- coding: utf-8 -*-
"""QGIS Unit tests for Processing CheckValidity algorithm.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '2018-09'
__copyright__ = 'Copyright 2018, The QGIS Project'

from qgis.PyQt.QtCore import QCoreApplication, QVariant
from qgis.core import (
    QgsFeature,
    QgsGeometry,
    QgsApplication,
    QgsMemoryProviderUtils,
    QgsWkbTypes,
    QgsField,
    QgsFields,
    QgsProcessingContext,
    QgsProcessingFeedback,
    QgsCoordinateReferenceSystem,
    QgsProject,
    QgsProcessingException,
    QgsProcessingUtils,
    QgsSettings
)
from processing.core.Processing import Processing
from processing.gui.AlgorithmExecutor import execute
from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy
from qgis.analysis import QgsNativeAlgorithms

start_app()


class ConsoleFeedBack(QgsProcessingFeedback):

    def reportError(self, error, fatalError=False):
        print(error)


class TestQgsProcessingCheckValidity(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(
            "QGIS_TestPyQgsProcessingCheckValidity.com")
        QCoreApplication.setApplicationName(
            "QGIS_TestPyQgsProcessingCheckValidity")
        QgsSettings().clear()
        Processing.initialize()
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
        cls.registry = QgsApplication.instance().processingRegistry()

    def _make_layer(self, layer_wkb_name):
        fields = QgsFields()
        wkb_type = getattr(QgsWkbTypes, layer_wkb_name)
        fields.append(QgsField('int_f', QVariant.Int))
        layer = QgsMemoryProviderUtils.createMemoryLayer(
            '%s_layer' % layer_wkb_name, fields, wkb_type, QgsCoordinateReferenceSystem(4326))
        self.assertTrue(layer.isValid())
        self.assertEqual(layer.wkbType(), wkb_type)
        return layer

    def test_check_validity(self):
        """Test that the output invalid contains the error reason"""

        polygon_layer = self._make_layer('Polygon')
        self.assertTrue(polygon_layer.startEditing())
        f = QgsFeature(polygon_layer.fields())
        f.setAttributes([1])
        # Flake!
        f.setGeometry(QgsGeometry.fromWkt(
            'POLYGON ((0 0, 2 2, 0 2, 2 0, 0 0))'))
        self.assertTrue(f.isValid())
        f2 = QgsFeature(polygon_layer.fields())
        f2.setAttributes([1])
        f2.setGeometry(QgsGeometry.fromWkt(
            'POLYGON((1.1 1.1, 1.1 2.1, 2.1 2.1, 2.1 1.1, 1.1 1.1))'))
        self.assertTrue(f2.isValid())
        self.assertTrue(polygon_layer.addFeatures([f, f2]))
        polygon_layer.commitChanges()
        polygon_layer.rollBack()
        self.assertEqual(polygon_layer.featureCount(), 2)

        QgsProject.instance().addMapLayers([polygon_layer])

        alg = self.registry.createAlgorithmById('qgis:checkvalidity')

        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = ConsoleFeedBack()

        self.assertIsNotNone(alg)
        parameters = {}
        parameters['INPUT_LAYER'] = polygon_layer.id()
        parameters['VALID_OUTPUT'] = 'memory:'
        parameters['INVALID_OUTPUT'] = 'memory:'
        parameters['ERROR_OUTPUT'] = 'memory:'

        # QGIS method
        parameters['METHOD'] = 1
        ok, results = execute(
            alg, parameters, context=context, feedback=feedback)
        self.assertTrue(ok)
        invalid_layer = QgsProcessingUtils.mapLayerFromString(
            results['INVALID_OUTPUT'], context)
        self.assertEqual(invalid_layer.fields().names()[-1], '_errors')
        self.assertEqual(invalid_layer.featureCount(), 1)
        f = next(invalid_layer.getFeatures())
        self.assertEqual(f.attributes(), [
                         1, 'segments 0 and 2 of line 0 intersect at 1, 1'])

        # GEOS method
        parameters['METHOD'] = 2
        ok, results = execute(
            alg, parameters, context=context, feedback=feedback)
        self.assertTrue(ok)
        invalid_layer = QgsProcessingUtils.mapLayerFromString(
            results['INVALID_OUTPUT'], context)
        self.assertEqual(invalid_layer.fields().names()[-1], '_errors')
        self.assertEqual(invalid_layer.featureCount(), 1)
        f = next(invalid_layer.getFeatures())
        self.assertEqual(f.attributes(), [1, 'Self-intersection'])


if __name__ == '__main__':
    unittest.main()
