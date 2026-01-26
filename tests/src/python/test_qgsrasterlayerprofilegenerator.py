"""QGIS Unit tests for QgsRasterLayer profile generation

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "18/03/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

import os

from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsLineString,
    QgsProfileIdentifyContext,
    QgsProfilePoint,
    QgsProfileRequest,
    QgsProfileSnapContext,
    QgsRasterLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsRasterLayerProfileGenerator(QgisTestCase):

    def testGeneration(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)"
        )
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
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
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

        features = r.asFeatures(Qgis.ProfileExportType.Features3D)
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].layerIdentifier, rl.id())
        self.assertEqual(features[0].geometry.constGet().numPoints(), 1394)
        self.assertEqual(
            features[0].geometry.constGet().pointN(0).asWkt(-2),
            "Point Z (-348100 6633700 200)",
        )
        self.assertEqual(
            features[0].geometry.constGet().pointN(1393).asWkt(-2),
            "Point Z (-345800 6631600 100)",
        )

        features = r.asFeatures(Qgis.ProfileExportType.Profile2D)
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].layerIdentifier, rl.id())
        self.assertEqual(features[0].geometry.constGet().numPoints(), 1394)
        self.assertEqual(
            features[0].geometry.constGet().pointN(0).asWkt(-2), "Point (0 200)"
        )
        self.assertEqual(
            features[0].geometry.constGet().pointN(1393).asWkt(-2), "Point (3400 100)"
        )

        features = r.asFeatures(Qgis.ProfileExportType.DistanceVsElevationTable)
        self.assertEqual(len(features), 1394)
        self.assertEqual(features[0].layerIdentifier, rl.id())
        self.assertAlmostEqual(features[0].attributes["distance"], 0, 0)
        self.assertAlmostEqual(features[0].attributes["elevation"], 154.0, 0)
        self.assertEqual(
            features[0].geometry.asWkt(-2), "Point Z (-348100 6633700 200)"
        )
        self.assertEqual(
            features[-1].geometry.asWkt(-2), "Point Z (-345800 6631600 100)"
        )
        self.assertAlmostEqual(features[-1].attributes["distance"], 3392.69, -1)
        self.assertAlmostEqual(features[-1].attributes["elevation"], 99.0, 0)

    def testGenerationWithStepSize(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (-348095.18706532847136259 6633687.0235139261931181, -347271.57799367723055184 6633093.13086318597197533, -346140.60267287614988163 6632697.89590711053460836, -345777.013075890194159 6631575.50219972990453243)"
        )
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
        req.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
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

        features = r.asFeatures(Qgis.ProfileExportType.Features3D)
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].layerIdentifier, rl.id())
        self.assertEqual(features[0].geometry.constGet().numPoints(), 341)
        self.assertEqual(
            features[0].geometry.constGet().pointN(0).asWkt(-2),
            "Point Z (-348100 6633700 200)",
        )
        self.assertEqual(
            features[0].geometry.constGet().pointN(340).asWkt(-2),
            "Point Z (-345800 6631600 100)",
        )

        features = r.asFeatures(Qgis.ProfileExportType.Profile2D)
        self.assertEqual(len(features), 1)
        self.assertEqual(features[0].layerIdentifier, rl.id())
        self.assertEqual(features[0].geometry.constGet().numPoints(), 341)
        self.assertEqual(
            features[0].geometry.constGet().pointN(0).asWkt(-2), "Point (0 200)"
        )
        self.assertEqual(
            features[0].geometry.constGet().pointN(340).asWkt(-2), "Point (3400 100)"
        )

        features = r.asFeatures(Qgis.ProfileExportType.DistanceVsElevationTable)
        self.assertEqual(len(features), 341)
        self.assertEqual(features[0].layerIdentifier, rl.id())
        self.assertAlmostEqual(features[0].attributes["distance"], 0.0, 2)
        self.assertAlmostEqual(features[0].attributes["elevation"], 154.0, 2)
        self.assertEqual(
            features[0].geometry.asWkt(-2), "Point Z (-348100 6633700 200)"
        )
        self.assertEqual(
            features[-1].geometry.asWkt(-2), "Point Z (-345800 6631600 100)"
        )
        self.assertAlmostEqual(features[-1].attributes["distance"], 3393.2639, 2)
        self.assertAlmostEqual(features[-1].attributes["elevation"], 99.0, 2)

    def testGenerationWithVerticalLine(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (321878.13400000002002344 130592.75222520538954996, 321878.13400000002002344 129982.02943174661777448)"
        )
        req = QgsProfileRequest(curve)
        req.setStepDistance(10)

        rl.elevationProperties().setEnabled(True)

        req.setCrs(QgsCoordinateReferenceSystem("EPSG:27700"))
        generator = rl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()
        results = r.distanceToHeightMap()
        self.assertEqual(len(results), 63)
        first_point = min(results.keys())
        last_point = max(results.keys())
        self.assertEqual(results[first_point], 120)
        self.assertEqual(results[last_point], 86)

        self.assertEqual(r.zRange().lower(), 74)
        self.assertEqual(r.zRange().upper(), 120)

    def testGenerationWithHorizontalLine(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (321471.82703730149660259 130317.67500000000291038, 322294.53625493601430207 130317.67500000000291038)"
        )
        req = QgsProfileRequest(curve)
        req.setStepDistance(10)

        rl.elevationProperties().setEnabled(True)

        req.setCrs(QgsCoordinateReferenceSystem("EPSG:27700"))
        generator = rl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()
        results = r.distanceToHeightMap()
        self.assertEqual(len(results), 84)
        first_point = min(results.keys())
        last_point = max(results.keys())
        self.assertEqual(results[first_point], 122)
        self.assertEqual(results[last_point], 122)

        self.assertEqual(r.zRange().lower(), 76)
        self.assertEqual(r.zRange().upper(), 130)

    def testSnapping(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (321621.3770066662109457 129734.87810317709227093, 321894.21278918092139065 129858.49142702402605209)"
        )
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

    def testIdentify(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), "3d", "dtm.tif"), "DTM")
        self.assertTrue(rl.isValid())
        rl.elevationProperties().setEnabled(True)

        curve = QgsLineString()
        curve.fromWkt(
            "LineString (321621.3770066662109457 129734.87810317709227093, 321894.21278918092139065 129858.49142702402605209)"
        )
        req = QgsProfileRequest(curve)

        generator = rl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()

        # try identifying
        context = QgsProfileIdentifyContext()
        res = r.identify(QgsProfilePoint(-10, -10), context)
        self.assertFalse(res)

        context.maximumSurfaceDistanceDelta = 0
        context.maximumSurfaceElevationDelta = 3
        context.maximumPointDistanceDelta = 0
        context.maximumPointElevationDelta = 0
        res = r.identify(QgsProfilePoint(0, 70), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), rl)
        self.assertEqual(res[0].results(), [{"distance": 0.0, "elevation": 72.0}])

        context.maximumSurfaceDistanceDelta = 0
        context.maximumSurfaceElevationDelta = 5
        res = r.identify(QgsProfilePoint(200, 79), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), rl)
        self.assertEqual(res[0].results(), [{"distance": 200.0, "elevation": 75.0}])

        res = r.identify(QgsProfilePoint(200, 85), context)
        self.assertFalse(res)


if __name__ == "__main__":
    unittest.main()
