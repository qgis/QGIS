# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMeshLayer profile generation

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/03/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import os

import qgis  # NOQA

from qgis.PyQt.QtCore import QTemporaryDir

from qgis.core import (
    QgsMeshLayer,
    QgsLineString,
    QgsProfileRequest,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsFlatTerrainProvider,
    QgsMeshTerrainProvider
)

from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsMeshLayerProfileGenerator(unittest.TestCase):

    def testGeneration(self):
        ml = QgsMeshLayer(os.path.join(unitTestDataPath(), '3d', 'elev_mesh.2dm'), 'mdal', 'mdal')
        self.assertTrue(ml.isValid())
        ml.setCrs(QgsCoordinateReferenceSystem('EPSG:27700'))

        curve = QgsLineString()
        curve.fromWkt('LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)')
        req = QgsProfileRequest(curve)

        generator = ml.createProfileGenerator(req)
        self.assertIsNotNone(generator)

        # set correct crs for linestring and re-try
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        generator = ml.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()
        results = r.distanceToHeightMap()
        self.assertEqual(len(results), 102)
        first_point = min(results.keys())
        last_point = max(results.keys())
        self.assertAlmostEqual(results[first_point], 152.87405434310168, 5)
        self.assertAlmostEqual(results[last_point], 98.78085001573021, 5)

    def testStepSize(self):
        ml = QgsMeshLayer(os.path.join(unitTestDataPath(), '3d', 'elev_mesh.2dm'), 'mdal', 'mdal')
        self.assertTrue(ml.isValid())
        ml.setCrs(QgsCoordinateReferenceSystem('EPSG:27700'))

        curve = QgsLineString()
        curve.fromWkt('LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)')
        req = QgsProfileRequest(curve)
        # set a smaller step size then would be automatically calculated
        req.setStepDistance(10)

        # set correct crs for linestring and re-try
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        generator = ml.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()
        results = r.distanceToHeightMap()
        self.assertEqual(len(results), 216)
        first_point = min(results.keys())
        last_point = max(results.keys())
        self.assertAlmostEqual(results[first_point], 152.87405434310168, 5)
        self.assertAlmostEqual(results[last_point], 98.78085001573021, 5)


if __name__ == '__main__':
    unittest.main()
