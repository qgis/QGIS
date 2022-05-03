# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterLayer profile generation

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
    QgsRasterLayer,
    QgsLineString,
    QgsProfileRequest,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsFlatTerrainProvider,
    QgsMeshTerrainProvider,
    QgsProfilePoint,
    QgsProfileSnapContext
)

from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsRasterLayerProfileGenerator(unittest.TestCase):

    def testGeneration(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())

        curve = QgsLineString()
        curve.fromWkt('LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)')
        req = QgsProfileRequest(curve)

        generator = rl.createProfileGenerator(req)
        # not enabled for elevation
        self.assertIsNone(generator)

        rl.elevationProperties().setEnabled(True)

        generator = rl.createProfileGenerator(req)
        self.assertIsNotNone(generator)
        # the request did not have the crs of the linestring set, so the whole linestring falls outside the raster extent
        self.assertFalse(generator.generateProfile())

        # set correct crs for linestring and re-try
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        generator = rl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()
        results = r.distanceToHeightMap()
        self.assertEqual(len(results), 1394)
        first_point = min(results.keys())
        last_point = max(results.keys())
        self.assertEqual(results[first_point], 154)
        self.assertEqual(results[last_point], 99)

        self.assertEqual(r.zRange().lower(), 74)
        self.assertEqual(r.zRange().upper(), 154)

    def testGenerationWithStepSize(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())

        curve = QgsLineString()
        curve.fromWkt('LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)')
        req = QgsProfileRequest(curve)
        req.setStepDistance(10)

        generator = rl.createProfileGenerator(req)
        # not enabled for elevation
        self.assertIsNone(generator)

        rl.elevationProperties().setEnabled(True)

        generator = rl.createProfileGenerator(req)
        self.assertIsNotNone(generator)
        # the request did not have the crs of the linestring set, so the whole linestring falls outside the raster extent
        self.assertFalse(generator.generateProfile())

        # set correct crs for linestring and re-try
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        generator = rl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()
        results = r.distanceToHeightMap()
        self.assertEqual(len(results), 341)
        first_point = min(results.keys())
        last_point = max(results.keys())
        self.assertEqual(results[first_point], 154)
        self.assertEqual(results[last_point], 99)

        self.assertEqual(r.zRange().lower(), 74)
        self.assertEqual(r.zRange().upper(), 154)

    def testSnapping(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        curve = QgsLineString()
        curve.fromWkt('LineString (321621.3770066662109457 129734.87810317709227093, 321894.21278918092139065 129858.49142702402605209)')
        req = QgsProfileRequest(curve)

        generator = rl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()

        # try snapping some points
        context = QgsProfileSnapContext()
        res = r.snapPoint(QgsProfilePoint(-10, -10), context)
        self.assertFalse(res.isValid())

        context.maximumSurfaceDistanceDelta = 0
        context.maximumSurfaceElevationDelta = 3
        context.maximumPointDistanceDelta = 0
        context.maximumPointElevationDelta = 0
        res = r.snapPoint(QgsProfilePoint(0, 70), context)
        self.assertTrue(res.isValid())
        self.assertEqual(res.snappedPoint.distance(), 0)
        self.assertEqual(res.snappedPoint.elevation(), 72)

        context.maximumSurfaceDistanceDelta = 0
        context.maximumSurfaceElevationDelta = 5
        res = r.snapPoint(QgsProfilePoint(200, 79), context)
        self.assertTrue(res.isValid())
        self.assertEqual(res.snappedPoint.distance(), 200)
        self.assertEqual(res.snappedPoint.elevation(), 75)

        res = r.snapPoint(QgsProfilePoint(200, 85), context)
        self.assertFalse(res.isValid())


if __name__ == '__main__':
    unittest.main()
