# -*- coding: utf-8 -*-
"""QGIS Unit tests for Processing In-Place algorithms.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '2018-09'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QCoreApplication, QVariant
from qgis.core import QgsFeature, QgsGeometry, QgsSettings, QgsApplication, QgsMemoryProviderUtils, QgsWkbTypes, QgsField, QgsFields, QgsProcessingFeatureSourceDefinition, QgsProcessingContext, QgsProcessingFeedback
from processing.core.Processing import Processing
from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy
from qgis.analysis import QgsNativeAlgorithms

start_app()


class TestQgsProcessingRecentAlgorithmLog(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(
            "QGIS_TestPyQgsProcessingInPlace.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsProcessingInPlace")
        QgsSettings().clear()
        Processing.initialize()
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
        cls.registry = QgsApplication.instance().processingRegistry()
        fields = QgsFields()
        fields.append(QgsField('int_f', QVariant.Int))
        cls.vl = QgsMemoryProviderUtils.createMemoryLayer(
            'mylayer', fields, QgsWkbTypes.Point)

        f1 = QgsFeature(cls.vl.fields())
        f1['int_f'] = 1
        f1.setGeometry(QgsGeometry.fromWkt('point(9 45)'))
        f2 = QgsFeature(cls.vl.fields())
        f2['int_f'] = 2
        f2.setGeometry(QgsGeometry.fromWkt('point(9.5 45.6)'))
        cls.vl.dataProvider().addFeatures([f1, f2])

        assert cls.vl.isValid()
        assert cls.vl.featureCount() == 2

    def test_algs(self):
        # Clone?
        alg = self.registry.algorithmById('native:translategeometry')
        self.assertIsNotNone(alg)
        parameters = {}
        parameters['INPUT'] = QgsProcessingFeatureSourceDefinition(self.vl.id(), True)
        parameters['OUTPUT'] = 'memory:'
        parameters['DELTA_X'] = 1.1
        parameters['DELTA_Y'] = 1.1

        context = QgsProcessingContext()
        feedback = QgsProcessingFeedback()
        self.assertTrue(alg.prepare(parameters, context, feedback))

        self.assertTrue(self.vl.startEditing())

        field_idxs = range(len(self.vl.fields()))

        new_features = {}
        for f in self.vl.getFeatures():
            new_f = alg.processFeature(f, context, feedback)[0]
            new_features[f.id()] = new_f
            self.assertEqual(new_f.id(), f.id())
            # This alg does change the geometry!
            self.vl.changeGeometry(f.id(), new_f.geometry())
            # This alg does not change attrs but some other might do
            self.vl.changeAttributeValues(f.id(), dict(zip(field_idxs, new_f.attributes())), dict(zip(field_idxs, f.attributes())))

        self.assertTrue(self.vl.commitChanges())

        for f in self.vl.getFeatures():
            self.assertEqual(new_features[f.id()].geometry().asWkt(), f.geometry().asWkt())


if __name__ == '__main__':
    unittest.main()
