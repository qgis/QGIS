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
from qgis.core import (
    QgsFeature, QgsGeometry, QgsSettings, QgsApplication, QgsMemoryProviderUtils, QgsWkbTypes, QgsField, QgsFields, QgsProcessingFeatureSourceDefinition, QgsProcessingContext, QgsProcessingFeedback, QgsCoordinateReferenceSystem, QgsProject, QgsProcessingException
)
from processing.core.Processing import Processing
from processing.gui.AlgorithmExecutor import execute_in_place_run
from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy
from qgis.analysis import QgsNativeAlgorithms

start_app()


class TestQgsProcessingInPlace(unittest.TestCase):

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
            'mylayer', fields, QgsWkbTypes.Point, QgsCoordinateReferenceSystem(4326))

        f1 = QgsFeature(cls.vl.fields())
        f1['int_f'] = 1
        f1.setGeometry(QgsGeometry.fromWkt('point(9 45)'))
        f2 = QgsFeature(cls.vl.fields())
        f2['int_f'] = 2
        f2.setGeometry(QgsGeometry.fromWkt('point(9.5 45.6)'))
        cls.vl.dataProvider().addFeatures([f1, f2])

        assert cls.vl.isValid()
        assert cls.vl.featureCount() == 2

        # Multipolygon layer

        cls.multipoly_vl = QgsMemoryProviderUtils.createMemoryLayer(
            'mymultiplayer', fields, QgsWkbTypes.MultiPolygon, QgsCoordinateReferenceSystem(4326))

        f3 = QgsFeature(cls.multipoly_vl.fields())
        f3.setGeometry(QgsGeometry.fromWkt('MultiPolygon (((2.81856297539240419 41.98170998812887689, 2.81874467773035464 41.98167537995160359, 2.81879535908157752 41.98154066615795443, 2.81866433873670452 41.98144056064155905, 2.81848263699778379 41.98147516865246587, 2.81843195500470811 41.98160988234612034, 2.81856297539240419 41.98170998812887689)),((2.81898589063455907 41.9815711567298635, 2.81892080450418803 41.9816030048432367, 2.81884192631866437 41.98143737613141724, 2.8190679469505846 41.98142270931093378, 2.81898589063455907 41.9815711567298635)))'))
        f4 = QgsFeature(cls.multipoly_vl.fields())
        f4.setGeometry(QgsGeometry.fromWkt('MultiPolygon (((2.81823679385631332 41.98133290154246566, 2.81830770255185703 41.98123540208609228, 2.81825871989355159 41.98112524362621656, 2.81813882853970243 41.98111258462271422, 2.81806791984415872 41.98121008407908761, 2.81811690250246416 41.98132024253896333, 2.81823679385631332 41.98133290154246566)),((2.81835835162010895 41.98123286963267731, 2.8183127674586852 41.98108725356146209, 2.8184520523963692 41.98115436357689134, 2.81835835162010895 41.98123286963267731)))'))
        cls.multipoly_vl.dataProvider().addFeatures([f3, f4])

        assert cls.multipoly_vl.isValid()
        assert cls.multipoly_vl.featureCount() == 2

        QgsProject.instance().addMapLayers([cls.vl, cls.multipoly_vl])

    def _alg_tester(self, alg_name, input_layer, parameters):

        alg = self.registry.createAlgorithmById(alg_name)

        self.assertIsNotNone(alg)
        parameters['INPUT'] = QgsProcessingFeatureSourceDefinition(
            input_layer.id(), True)
        parameters['OUTPUT'] = 'memory:'

        old_features = [f for f in input_layer.getFeatures()]
        input_layer.selectByIds([old_features[0].id()])
        # Check selected
        self.assertEqual(input_layer.selectedFeatureIds(), [old_features[0].id()], alg_name)

        context = QgsProcessingContext()
        context.setProject(QgsProject.instance())
        feedback = QgsProcessingFeedback()

        with self.assertRaises(QgsProcessingException) as cm:
            execute_in_place_run(
                alg, input_layer, parameters, context=context, feedback=feedback, raise_exceptions=True)

        ok = False
        input_layer.startEditing()
        ok, _ = execute_in_place_run(
            alg, input_layer, parameters, context=context, feedback=feedback, raise_exceptions=True)
        new_features = [f for f in input_layer.getFeatures()]

        # Check ret values
        self.assertTrue(ok, alg_name)

        # Check geometry types (drop Z or M)
        self.assertEqual(new_features[0].geometry().wkbType(), old_features[0].geometry().wkbType())

        return old_features, new_features

    def test_execute_in_place_run(self):
        """Test the execution in place"""

        self.vl.rollBack()

        old_features, new_features = self._alg_tester(
            'native:translategeometry',
            self.vl,
            {
                'DELTA_X': 1.1,
                'DELTA_Y': 1.1,
            }
        )

        # First feature was selected and modified
        self.assertEqual(new_features[0].id(), old_features[0].id())
        self.assertAlmostEqual(new_features[0].geometry().asPoint().x(), old_features[0].geometry().asPoint().x() + 1.1, delta=0.01)
        self.assertAlmostEqual(new_features[0].geometry().asPoint().y(), old_features[0].geometry().asPoint().y() + 1.1, delta=0.01)

        # Second feature was not selected and not modified
        self.assertEqual(new_features[1].id(), old_features[1].id())
        self.assertEqual(new_features[1].geometry().asPoint().x(), old_features[1].geometry().asPoint().x())
        self.assertEqual(new_features[1].geometry().asPoint().y(), old_features[1].geometry().asPoint().y())

        # Check selected
        self.assertEqual(self.vl.selectedFeatureIds(), [old_features[0].id()])

        # Check that if the only change is Z or M then we should fail
        with self.assertRaises(QgsProcessingException) as cm:
            self._alg_tester(
                'native:translategeometry',
                self.vl,
                {
                    'DELTA_Z': 1.1,
                }
            )
        self.vl.rollBack()

        # Check that if the only change is Z or M then we should fail
        with self.assertRaises(QgsProcessingException) as cm:
            self._alg_tester(
                'native:translategeometry',
                self.vl,
                {
                    'DELTA_M': 1.1,
                }
            )
        self.vl.rollBack()

        old_features, new_features = self._alg_tester(
            'native:translategeometry',
            self.vl,
            {
                'DELTA_X': 1.1,
                'DELTA_Z': 1.1,
            }
        )

    def test_multi_to_single(self):
        """Check that the geometry type is still multi after the alg is run"""

        old_features, new_features = self._alg_tester(
            'native:multiparttosingleparts',
            self.multipoly_vl,
            {
            }
        )

        self.assertEqual(len(new_features), 3)

        # Check selected
        self.assertEqual(len(self.multipoly_vl.selectedFeatureIds()), 2)

    def test_arrayfeatures(self):
        """Check that this runs correctly and additional attributes are dropped"""

        old_count = self.vl.featureCount()

        old_features, new_features = self._alg_tester(
            'native:arrayfeatures',
            self.vl,
            {
                'COUNT': 2,
                'DELTA_X': 1.1,
                'DELTA_Z': 1.1,
            }
        )

        self.assertEqual(len(new_features), old_count + 2)

        # Check selected
        self.assertEqual(len(self.vl.selectedFeatureIds()), 3)

    def test_make_compatible(self):
        """Test fixer function"""
        pass


if __name__ == '__main__':
    unittest.main()
