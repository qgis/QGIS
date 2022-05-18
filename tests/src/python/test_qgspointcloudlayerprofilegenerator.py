# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPointCloudLayer profile generation

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
from qgis.PyQt.QtCore import QDir
from qgis.core import (
    QgsRasterLayer,
    QgsLineString,
    QgsProfileRequest,
    QgsCoordinateReferenceSystem,
    QgsVectorLayer,
    Qgis,
    QgsRasterDemTerrainProvider,
    QgsFeature,
    QgsGeometry,
    QgsCoordinateTransformContext,
    QgsProjUtils,
    QgsProfilePlotRenderer,
    QgsFillSymbol,
    QgsRenderChecker,
    QgsCategorizedSymbolRenderer,
    QgsProfilePoint,
    QgsProfileSnapContext,
    QgsProfileIdentifyContext,
    QgsDoubleRange,
    QgsProfileGenerationContext,
    QgsUnitTypes,
    QgsPointCloudLayer,
    QgsProviderRegistry
)
from qgis.PyQt.QtGui import QColor
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudLayerProfileGenerator(unittest.TestCase):

    @staticmethod
    def round_dict(val, places):
        return {round(k, places): round(val[k], places) for k in sorted(val.keys())}

    def setUp(self):
        self.report = "<h1>Python QgsPointCloudLayerProfileGenerator Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testProfileGeneration(self):
        pcl = QgsPointCloudLayer(
            os.path.join(unitTestDataPath(), 'point_clouds', 'ept', 'lone-star-laszip', 'ept.json'), 'test', 'ept')
        self.assertTrue(pcl.isValid())
        pcl.elevationProperties().setMaximumScreenError(30)
        pcl.elevationProperties().setMaximumScreenErrorUnit(QgsUnitTypes.RenderMillimeters)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (515387.94696552358800545 4918366.65919817332178354, 515389.15378401038469747 4918366.63842081092298031)')
        req = QgsProfileRequest(curve)
        req.setCrs(pcl.crs())
        # zero tolerance => no points
        generator = pcl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()
        self.assertFalse(results.distanceToHeightMap())

        req.setTolerance(0.05)
        generator = pcl.createProfileGenerator(req)

        context = QgsProfileGenerationContext()
        context.setMapUnitsPerDistancePixel(0.50)

        self.assertTrue(generator.generateProfile(context))
        results = generator.takeResults()
        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {0.0: 2325.1, 0.1: 2325.2, 0.2: 2332.4, 0.3: 2325.1, 0.4: 2325.1, 0.5: 2325.1, 0.6: 2331.4,
                          0.7: 2330.6, 0.9: 2332.7, 1.0: 2325.4, 1.1: 2325.6, 1.2: 2325.6})

        self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                              ['PointZ (515389.1 4918366.7 2326.1)', 'PointZ (515389.1 4918366.6 2325.6)',
                               'PointZ (515389 4918366.6 2325.3)', 'PointZ (515388.2 4918366.6 2325.2)',
                               'PointZ (515388.3 4918366.7 2325.1)', 'PointZ (515387.9 4918366.7 2325.2)',
                               'PointZ (515388.7 4918366.6 2330.5)', 'PointZ (515388.6 4918366.6 2331.2)',
                               'PointZ (515388.9 4918366.6 2332.7)', 'PointZ (515388.9 4918366.7 2332.7)',
                               'PointZ (515388.6 4918366.6 2331.4)', 'PointZ (515388.2 4918366.7 2332.2)',
                               'PointZ (515388.2 4918366.7 2332.6)', 'PointZ (515388.2 4918366.6 2335)',
                               'PointZ (515388.6 4918366.6 2334.6)', 'PointZ (515389.1 4918366.6 2326.1)',
                               'PointZ (515389.1 4918366.6 2325.4)', 'PointZ (515389.1 4918366.6 2325.5)',
                               'PointZ (515389 4918366.6 2325.2)', 'PointZ (515388.3 4918366.6 2325.1)',
                               'PointZ (515388.6 4918366.6 2330.9)', 'PointZ (515388.7 4918366.6 2330.5)',
                               'PointZ (515388.6 4918366.6 2330.4)', 'PointZ (515389.1 4918366.6 2325.5)',
                               'PointZ (515389.1 4918366.6 2325.9)', 'PointZ (515389.1 4918366.6 2325.8)',
                               'PointZ (515389.1 4918366.7 2325.6)', 'PointZ (515389.1 4918366.6 2325.4)',
                               'PointZ (515389.1 4918366.7 2325.2)', 'PointZ (515389.1 4918366.6 2326)',
                               'PointZ (515389.1 4918366.6 2326)', 'PointZ (515389.1 4918366.6 2325.4)',
                               'PointZ (515389.1 4918366.7 2325.3)', 'PointZ (515389 4918366.6 2325.3)',
                               'PointZ (515389.1 4918366.7 2325.2)', 'PointZ (515389 4918366.6 2325.4)',
                               'PointZ (515389 4918366.6 2325.4)', 'PointZ (515389 4918366.7 2325.2)',
                               'PointZ (515389 4918366.7 2325.4)', 'PointZ (515388.6 4918366.6 2325.2)',
                               'PointZ (515388.6 4918366.7 2325.2)', 'PointZ (515388.5 4918366.7 2325.2)',
                               'PointZ (515388.5 4918366.7 2325.2)', 'PointZ (515388.4 4918366.6 2325.1)',
                               'PointZ (515388.3 4918366.6 2325.1)', 'PointZ (515388.3 4918366.7 2325.1)',
                               'PointZ (515388.2 4918366.6 2325.1)', 'PointZ (515388.2 4918366.6 2325.2)',
                               'PointZ (515388.2 4918366.6 2325.2)', 'PointZ (515388.2 4918366.7 2325.2)',
                               'PointZ (515388.1 4918366.6 2325.2)', 'PointZ (515388.1 4918366.6 2325.2)',
                               'PointZ (515388 4918366.7 2325.2)', 'PointZ (515388 4918366.6 2325.1)',
                               'PointZ (515388 4918366.6 2325.2)', 'PointZ (515388.7 4918366.6 2330.6)',
                               'PointZ (515388.7 4918366.6 2330.5)', 'PointZ (515388.6 4918366.7 2331)',
                               'PointZ (515388.7 4918366.7 2330.9)', 'PointZ (515388.6 4918366.6 2330.9)',
                               'PointZ (515388.6 4918366.6 2330.8)', 'PointZ (515388.7 4918366.7 2330.7)',
                               'PointZ (515388.6 4918366.7 2330.6)', 'PointZ (515389.1 4918366.6 2325.5)',
                               'PointZ (515389.1 4918366.6 2325.5)', 'PointZ (515389.1 4918366.7 2325.5)',
                               'PointZ (515389.1 4918366.7 2325.2)', 'PointZ (515389.1 4918366.6 2325.4)',
                               'PointZ (515389.1 4918366.7 2325.2)', 'PointZ (515389.1 4918366.7 2325.5)',
                               'PointZ (515389.1 4918366.6 2325.4)', 'PointZ (515389.1 4918366.7 2325.3)',
                               'PointZ (515389.1 4918366.7 2325.2)', 'PointZ (515389.1 4918366.7 2325.2)',
                               'PointZ (515389.1 4918366.6 2325.3)', 'PointZ (515389.1 4918366.7 2325.4)',
                               'PointZ (515389.1 4918366.6 2325.3)', 'PointZ (515389 4918366.7 2325.3)',
                               'PointZ (515389 4918366.7 2325.3)', 'PointZ (515389.1 4918366.7 2325.3)',
                               'PointZ (515389 4918366.7 2325.4)', 'PointZ (515389 4918366.7 2325.3)',
                               'PointZ (515389 4918366.6 2325.2)', 'PointZ (515389 4918366.6 2325.4)',
                               'PointZ (515389 4918366.6 2325.3)', 'PointZ (515389 4918366.6 2325.3)',
                               'PointZ (515389 4918366.7 2325.2)', 'PointZ (515389 4918366.7 2325.2)',
                               'PointZ (515389 4918366.6 2325.4)', 'PointZ (515389 4918366.6 2325.3)',
                               'PointZ (515389 4918366.7 2325.3)', 'PointZ (515389 4918366.7 2325.2)',
                               'PointZ (515389 4918366.6 2325.3)', 'PointZ (515389 4918366.7 2325.3)',
                               'PointZ (515389 4918366.7 2325.2)', 'PointZ (515388.6 4918366.7 2325.2)',
                               'PointZ (515388.6 4918366.7 2325.2)', 'PointZ (515388.5 4918366.7 2325.1)',
                               'PointZ (515388.4 4918366.6 2325.1)', 'PointZ (515388.4 4918366.7 2325.1)',
                               'PointZ (515388.4 4918366.6 2325.1)', 'PointZ (515388.4 4918366.7 2325.1)',
                               'PointZ (515388.4 4918366.6 2325.1)', 'PointZ (515388.3 4918366.6 2325.1)',
                               'PointZ (515388.3 4918366.6 2325.1)', 'PointZ (515388.2 4918366.7 2325.1)',
                               'PointZ (515388.2 4918366.6 2325.2)', 'PointZ (515388.2 4918366.7 2325.2)',
                               'PointZ (515388.3 4918366.7 2325.1)', 'PointZ (515388.1 4918366.7 2325.2)',
                               'PointZ (515388.1 4918366.7 2325.1)', 'PointZ (515388.1 4918366.7 2325.1)',
                               'PointZ (515388.1 4918366.7 2325.2)', 'PointZ (515388 4918366.6 2325.1)',
                               'PointZ (515389.1 4918366.6 2325.8)', 'PointZ (515389.1 4918366.6 2325.8)',
                               'PointZ (515389.1 4918366.7 2325.8)', 'PointZ (515389.1 4918366.7 2325.6)',
                               'PointZ (515389.1 4918366.6 2325.5)', 'PointZ (515389.1 4918366.6 2325.9)',
                               'PointZ (515389.1 4918366.6 2325.9)', 'PointZ (515389.1 4918366.6 2325.9)',
                               'PointZ (515389.2 4918366.7 2325.6)', 'PointZ (515389.1 4918366.7 2325.8)',
                               'PointZ (515389.1 4918366.7 2325.5)', 'PointZ (515389.1 4918366.6 2326)',
                               'PointZ (515389.1 4918366.6 2326)', 'PointZ (515389.1 4918366.7 2326)',
                               'PointZ (515388.7 4918366.6 2330.6)', 'PointZ (515388.7 4918366.6 2330.6)',
                               'PointZ (515388.7 4918366.6 2330.5)', 'PointZ (515388.7 4918366.7 2331)',
                               'PointZ (515388.7 4918366.7 2330.7)', 'PointZ (515388.7 4918366.7 2330.8)',
                               'PointZ (515388.7 4918366.7 2330.6)', 'PointZ (515388.7 4918366.7 2330.9)',
                               'PointZ (515388.7 4918366.7 2330.8)', 'PointZ (515388.6 4918366.6 2331.1)',
                               'PointZ (515388.6 4918366.7 2331.3)', 'PointZ (515388.6 4918366.7 2331.3)',
                               'PointZ (515388.3 4918366.6 2334.7)', 'PointZ (515388.6 4918366.6 2331.1)',
                               'PointZ (515388.6 4918366.6 2331)', 'PointZ (515388.6 4918366.7 2331)',
                               'PointZ (515388.6 4918366.6 2331.3)', 'PointZ (515388.6 4918366.7 2331.2)',
                               'PointZ (515388.6 4918366.6 2331.3)', 'PointZ (515388.6 4918366.7 2331.4)',
                               'PointZ (515388.2 4918366.6 2332.4)', 'PointZ (515388.2 4918366.7 2332.2)',
                               'PointZ (515388.2 4918366.7 2332.3)', 'PointZ (515388.2 4918366.7 2332.7)',
                               'PointZ (515388.2 4918366.7 2332.7)', 'PointZ (515388.2 4918366.7 2332.7)',
                               'PointZ (515388.2 4918366.7 2332.6)', 'PointZ (515388.2 4918366.6 2332.5)',
                               'PointZ (515388.2 4918366.6 2332.5)', 'PointZ (515388.3 4918366.6 2334.7)',
                               'PointZ (515388.3 4918366.7 2334.7)', 'PointZ (515388.2 4918366.7 2335.1)',
                               'PointZ (515388.6 4918366.6 2331.2)', 'PointZ (515388.6 4918366.6 2331.1)',
                               'PointZ (515388.6 4918366.7 2331.1)', 'PointZ (515388.6 4918366.7 2331.1)',
                               'PointZ (515388.6 4918366.6 2331.3)', 'PointZ (515388.6 4918366.7 2331.3)',
                               'PointZ (515388.6 4918366.6 2331.1)', 'PointZ (515388.6 4918366.7 2331.3)',
                               'PointZ (515388.6 4918366.7 2331.2)', 'PointZ (515388.6 4918366.7 2331.4)',
                               'PointZ (515388.6 4918366.7 2331.4)', 'PointZ (515388.2 4918366.6 2332.3)',
                               'PointZ (515388.2 4918366.7 2332.4)', 'PointZ (515388.2 4918366.7 2332.4)',
                               'PointZ (515388.2 4918366.7 2332.7)', 'PointZ (515388.2 4918366.6 2332.6)',
                               'PointZ (515388.2 4918366.7 2332.6)', 'PointZ (515388.2 4918366.6 2332.5)',
                               'PointZ (515388.2 4918366.6 2332.5)', 'PointZ (515388.2 4918366.7 2332.4)',
                               'PointZ (515388.2 4918366.7 2332.6)', 'PointZ (515388.3 4918366.7 2334.7)'])
        self.assertAlmostEqual(results.zRange().lower(), 2325.1325, 2)
        self.assertAlmostEqual(results.zRange().upper(), 2335.0755, 2)

        # ensure maximum error is considered
        context.setMapUnitsPerDistancePixel(0.0001)
        self.assertTrue(generator.generateProfile(context))
        results = generator.takeResults()
        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 2),
                         {0.0: 2325.17, 0.01: 2325.14, 0.02: 2325.18, 0.03: 2325.14, 0.08: 2325.16, 0.11: 2325.16,
                          0.12: 2325.14, 0.14: 2325.16, 0.15: 2325.14, 0.18: 2325.15, 0.19: 2325.15, 0.21: 2332.45,
                          0.22: 2332.68, 0.23: 2332.44, 0.24: 2332.38, 0.25: 2332.37, 0.26: 2325.16, 0.27: 2332.27,
                          0.28: 2335.05, 0.29: 2335.08, 0.3: 2334.71, 0.31: 2325.13, 0.32: 2325.14, 0.33: 2325.14,
                          0.34: 2325.13, 0.36: 2325.14, 0.39: 2325.14, 0.41: 2325.13, 0.42: 2325.14, 0.44: 2325.14,
                          0.46: 2325.14, 0.49: 2325.14, 0.53: 2325.14, 0.56: 2325.16, 0.57: 2325.16, 0.61: 2325.17,
                          0.62: 2331.38, 0.63: 2330.44, 0.64: 2331.31, 0.65: 2331.41, 0.66: 2331.33, 0.67: 2331.13,
                          0.68: 2331.14, 0.69: 2331.01, 0.7: 2331.0, 0.71: 2330.52, 0.72: 2330.61, 0.92: 2332.72,
                          1.0: 2325.29, 1.01: 2325.25, 1.02: 2325.27, 1.03: 2325.39, 1.04: 2325.36, 1.05: 2325.24,
                          1.07: 2325.41, 1.08: 2325.38, 1.09: 2325.23, 1.1: 2325.21, 1.11: 2325.3, 1.12: 2325.28,
                          1.13: 2325.24, 1.15: 2326.11, 1.16: 2325.22, 1.17: 2325.82, 1.18: 2325.49, 1.19: 2325.55,
                          1.2: 2325.58, 1.21: 2325.62})

        # ensure distance/elevation ranges are respected
        context.setDistanceRange(QgsDoubleRange(0.3, 0.7))
        self.assertTrue(generator.generateProfile(context))
        results = generator.takeResults()
        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 2),
                         {0.3: 2334.71, 0.31: 2325.13, 0.32: 2325.14, 0.33: 2325.14, 0.34: 2325.13, 0.36: 2325.14,
                          0.39: 2325.14, 0.41: 2325.13, 0.42: 2325.14, 0.44: 2325.14, 0.46: 2325.14, 0.49: 2325.14,
                          0.53: 2325.14, 0.56: 2325.16, 0.57: 2325.16, 0.61: 2325.17, 0.62: 2331.38, 0.63: 2330.44,
                          0.64: 2331.31, 0.65: 2331.41, 0.66: 2331.33, 0.67: 2331.13, 0.68: 2331.14, 0.69: 2331.01,
                          0.7: 2330.97})

        context.setElevationRange(QgsDoubleRange(2325, 2326))
        self.assertTrue(generator.generateProfile(context))
        results = generator.takeResults()
        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 2),
                         {0.31: 2325.13, 0.32: 2325.14, 0.33: 2325.14, 0.34: 2325.13, 0.36: 2325.14, 0.39: 2325.14,
                          0.41: 2325.13, 0.42: 2325.14, 0.44: 2325.14, 0.46: 2325.14, 0.49: 2325.14, 0.53: 2325.14,
                          0.56: 2325.16, 0.57: 2325.16, 0.61: 2325.17, 0.64: 2325.18, 0.68: 2325.19})

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testSnapping(self):
        pcl = QgsPointCloudLayer(
            os.path.join(unitTestDataPath(), 'point_clouds', 'ept', 'lone-star-laszip', 'ept.json'), 'test', 'ept')
        self.assertTrue(pcl.isValid())
        pcl.elevationProperties().setMaximumScreenError(30)
        pcl.elevationProperties().setMaximumScreenErrorUnit(QgsUnitTypes.RenderMillimeters)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (515387.94696552358800545 4918366.65919817332178354, 515389.15378401038469747 4918366.63842081092298031)')
        req = QgsProfileRequest(curve)
        req.setCrs(pcl.crs())
        req.setTolerance(0.05)

        context = QgsProfileGenerationContext()
        context.setMapUnitsPerDistancePixel(0.50)

        generator = pcl.createProfileGenerator(req)
        generator.generateProfile(context)
        r = generator.takeResults()

        # try snapping some points
        context = QgsProfileSnapContext()
        res = r.snapPoint(QgsProfilePoint(0.27, 2335), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 0
        context.maximumPointElevationDelta = 0
        context.maximumSurfaceElevationDelta = 3
        context.maximumSurfaceDistanceDelta = 1
        res = r.snapPoint(QgsProfilePoint(0.27, 2335), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 1
        context.maximumPointElevationDelta = 1
        context.maximumSurfaceElevationDelta = 0
        context.maximumSurfaceDistanceDelta = 0

        res = r.snapPoint(QgsProfilePoint(0.27, 2335), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 0.2783, 2)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 2335.04575, 2)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testIdentify(self):
        pcl = QgsPointCloudLayer(
            os.path.join(unitTestDataPath(), 'point_clouds', 'ept', 'lone-star-laszip', 'ept.json'), 'test', 'ept')
        self.assertTrue(pcl.isValid())
        pcl.elevationProperties().setMaximumScreenError(30)
        pcl.elevationProperties().setMaximumScreenErrorUnit(QgsUnitTypes.RenderMillimeters)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (515387.94696552358800545 4918366.65919817332178354, 515389.15378401038469747 4918366.63842081092298031)')
        req = QgsProfileRequest(curve)
        req.setCrs(pcl.crs())
        req.setTolerance(0.05)

        context = QgsProfileGenerationContext()
        context.setMapUnitsPerDistancePixel(0.50)

        generator = pcl.createProfileGenerator(req)
        generator.generateProfile(context)
        r = generator.takeResults()

        # try identifying some points
        context = QgsProfileIdentifyContext()
        context.maximumPointDistanceDelta = 0
        context.maximumPointElevationDelta = 0
        res = r.identify(QgsProfilePoint(0.27, 2335), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 1
        context.maximumPointElevationDelta = 1

        res = r.identify(QgsProfilePoint(0.27, 2335), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), pcl)
        self.assertCountEqual(res[0].results(), [
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1612, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.2245, 'Y': 4918366.61, 'Z': 2335.04575},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 199, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.60825, 'Y': 4918366.628, 'Z': 2334.60175},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1678, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.27575, 'Y': 4918366.6325, 'Z': 2334.728},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1605, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.25025, 'Y': 4918366.62825, 'Z': 2334.7095},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1633, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.28575, 'Y': 4918366.66725, 'Z': 2334.7065},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1547, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.238, 'Y': 4918366.6555, 'Z': 2335.0755},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1603, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.26675, 'Y': 4918366.685, 'Z': 2334.69125}])

        context.maximumPointDistanceDelta = 0
        context.maximumPointElevationDelta = 0

        res = r.identify(QgsDoubleRange(0.2, 0.3), QgsDoubleRange(2330, 2360), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), pcl)
        self.assertCountEqual(res[0].results(), [
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 565, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.21275, 'Y': 4918366.65675, 'Z': 2332.19075},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1357, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.17375, 'Y': 4918366.679, 'Z': 2332.56025},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1612, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.2245, 'Y': 4918366.61, 'Z': 2335.04575},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1452, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.1985, 'Y': 4918366.61025, 'Z': 2332.38325},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 501, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.2145, 'Y': 4918366.66275, 'Z': 2332.16675},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1197, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.22125, 'Y': 4918366.68675, 'Z': 2332.2715},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 202, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.16825, 'Y': 4918366.6625, 'Z': 2332.73325},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 922, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.165, 'Y': 4918366.65025, 'Z': 2332.6565},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 955, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.1715, 'Y': 4918366.673, 'Z': 2332.6835},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1195, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.1785, 'Y': 4918366.6955, 'Z': 2332.6125},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1432, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.15825, 'Y': 4918366.62575, 'Z': 2332.501},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1413, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.1615, 'Y': 4918366.63675, 'Z': 2332.453},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1547, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.238, 'Y': 4918366.6555, 'Z': 2335.0755},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1259, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.20925, 'Y': 4918366.646, 'Z': 2332.29025},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1369, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.1895, 'Y': 4918366.662, 'Z': 2332.38325},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1394, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.2015, 'Y': 4918366.703, 'Z': 2332.36625},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 688, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.166, 'Y': 4918366.654, 'Z': 2332.7065},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1399, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.17225, 'Y': 4918366.60575, 'Z': 2332.57475},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1024, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.17475, 'Y': 4918366.683, 'Z': 2332.636},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1274, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.1585, 'Y': 4918366.62625, 'Z': 2332.5265},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1443, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.15875, 'Y': 4918366.62725, 'Z': 2332.4765},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1332, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.18, 'Y': 4918366.69875, 'Z': 2332.43775},
            {'Classification': 0, 'EdgeOfFlightLine': 0, 'GpsTime': 0.0, 'Intensity': 1295, 'NumberOfReturns': 1,
             'OriginId': 3, 'PointSourceId': 0, 'ReturnNumber': 1, 'ScanAngleRank': 0.0, 'ScanDirectionFlag': 1,
             'UserData': 0, 'X': 515388.1725, 'Y': 4918366.67475, 'Z': 2332.5855}])

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testProfileRenderFixedColor(self):
        pcl = QgsPointCloudLayer(
            os.path.join(unitTestDataPath(), 'point_clouds', 'ept', 'lone-star-laszip', 'ept.json'), 'test', 'ept')
        self.assertTrue(pcl.isValid())
        pcl.elevationProperties().setMaximumScreenError(30)
        pcl.elevationProperties().setMaximumScreenErrorUnit(QgsUnitTypes.RenderMillimeters)
        pcl.elevationProperties().setPointSymbol(Qgis.PointCloudSymbol.Square)
        pcl.elevationProperties().setPointColor(QColor(255, 0, 255))
        pcl.elevationProperties().setPointSize(3)
        pcl.elevationProperties().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)
        pcl.elevationProperties().setRespectLayerColors(False)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (515387.94696552358800545 4918366.65919817332178354, 515389.15378401038469747 4918366.63842081092298031)')
        req = QgsProfileRequest(curve)
        req.setCrs(pcl.crs())
        req.setTolerance(0.05)
        plot_renderer = QgsProfilePlotRenderer([pcl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 2320, 2330)
        self.assertTrue(self.imageCheck('point_cloud_layer_fixed_color', 'point_cloud_layer_fixed_color', res))

    def imageCheck(self, name, reference_image, image):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'profile_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("profile_chart")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print((self.report))
        return result


if __name__ == '__main__':
    unittest.main()
