# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayer profile generation

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
from qgis.core import (
    QgsRasterLayer,
    QgsLineString,
    QgsProfileRequest,
    QgsCoordinateReferenceSystem,
    QgsVectorLayer,
    Qgis,
    QgsRasterDemTerrainProvider,
    QgsFeature,
    QgsGeometry
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()


class TestQgsVectorLayerProfileGenerator(unittest.TestCase):

    def testPointGenerationAbsolute(self):
        vl = QgsVectorLayer(os.path.join(unitTestDataPath(), '3d', 'points_with_z.shp'), 'trees')
        self.assertTrue(vl.isValid())

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setZScale(2.5)
        vl.elevationProperties().setZOffset(10)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346715.98066368483705446 6632458.84484416991472244, -346546.9152928851544857 6632444.29659010842442513, -346522.40419847227167338 6632307.79493184108287096)')
        req = QgsProfileRequest(curve)

        generator = vl.createProfileGenerator(req)
        self.assertIsNotNone(generator)
        # the request did not have the crs of the linestring set, so the whole linestring falls outside the vector extent

        # TODO
        # self.assertFalse(generator.generateProfile())

        # set correct crs for linestring and re-try
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        # no tolerance => no hits
        results = generator.takeResults()
        self.assertFalse(results.distanceToHeightMap())

        req.setTolerance(20)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(results.distanceToHeightMap(),
                         {167.60402625236236: 274.0, 22.897466329720825: 274.0, 1159.1281061593656: 232.75000000000003,
                          1171.6873910420782: 235.5, 1197.5553515228498: 241.0, 1245.704759018164: 227.25,
                          1291.0771889584548: 221.75})

        # lower tolerance
        req.setTolerance(4.5)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()
        self.assertEqual(results.distanceToHeightMap(),
                         {167.60402625236236: 274.0, 1171.6873910420782: 235.5, 1197.5553515228498: 241.0})

    def testPointGenerationTerrain(self):
        """
        Points layer with terrain clamping
        """
        vl = QgsVectorLayer(os.path.join(unitTestDataPath(), '3d', 'points_with_z.shp'), 'trees')
        self.assertTrue(vl.isValid())

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Terrain)
        vl.elevationProperties().setZScale(2.5)
        vl.elevationProperties().setZOffset(10)

        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346715.98066368483705446 6632458.84484416991472244, -346546.9152928851544857 6632444.29659010842442513, -346522.40419847227167338 6632307.79493184108287096)')
        req = QgsProfileRequest(curve)
        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(rl)
        terrain_provider.setScale(0.3)
        terrain_provider.setOffset(-5)

        req.setTerrainProvider(terrain_provider)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        # no tolerance => no hits
        results = generator.takeResults()
        self.assertFalse(results.distanceToHeightMap())

        req.setTolerance(4.5)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(results.distanceToHeightMap(),
                         {167.60402625236236: 69.5, 1171.6873910420782: 58.99999999999999, 1197.5553515228498: 60.5})

    def testPointGenerationRelative(self):
        """
        Points layer with relative clamping
        """
        vl = QgsVectorLayer(os.path.join(unitTestDataPath(), '3d', 'points_with_z.shp'), 'trees')
        self.assertTrue(vl.isValid())

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Relative)
        vl.elevationProperties().setZScale(2.5)
        vl.elevationProperties().setZOffset(10)

        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346715.98066368483705446 6632458.84484416991472244, -346546.9152928851544857 6632444.29659010842442513, -346522.40419847227167338 6632307.79493184108287096)')
        req = QgsProfileRequest(curve)
        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(rl)
        terrain_provider.setScale(0.3)
        terrain_provider.setOffset(-5)

        req.setTerrainProvider(terrain_provider)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        # no tolerance => no hits
        results = generator.takeResults()
        self.assertFalse(results.distanceToHeightMap())

        req.setTolerance(4.5)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()
        self.assertEqual(results.distanceToHeightMap(),
                         {167.60402625236236: 333.5, 1171.6873910420782: 284.5, 1197.5553515228498: 291.5})

    def testPointGenerationRelativeExtrusion(self):
        """
        Points layer with relative clamping and extrusion
        """
        vl = QgsVectorLayer(os.path.join(unitTestDataPath(), '3d', 'points_with_z.shp'), 'trees')
        self.assertTrue(vl.isValid())

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Relative)
        vl.elevationProperties().setZScale(2.5)
        vl.elevationProperties().setZOffset(10)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(7)

        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346715.98066368483705446 6632458.84484416991472244, -346546.9152928851544857 6632444.29659010842442513, -346522.40419847227167338 6632307.79493184108287096)')
        req = QgsProfileRequest(curve)
        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(rl)
        terrain_provider.setScale(0.3)
        terrain_provider.setOffset(-5)

        req.setTerrainProvider(terrain_provider)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        req.setTolerance(4.5)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()
        self.assertEqual(results.distanceToHeightMap(),
                         {167.60402625236236: 333.5, 1171.6873910420782: 284.5, 1197.5553515228498: 291.5})

        self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                              ['LineStringZ (-347535.1 6632740.6 333.5, -347535.1 6632740.6 340.5)',
                               'LineStringZ (-346578.2 6632450.9 284.5, -346578.2 6632450.9 291.5)',
                               'LineStringZ (-346552.5 6632448.8 291.5, -346552.5 6632448.8 298.5)'])

    def testPointGenerationMultiPoint(self):
        vl = QgsVectorLayer('MultipointZ?crs=EPSG:27700', 'trees', 'memory')
        self.assertTrue(vl.isValid())

        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('MultiPointZ(322069 129893 89.1, 322077 129889 90.2, 322093 129888 92.4)'))
        self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setZScale(2.5)
        vl.elevationProperties().setZOffset(10)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346715.98066368483705446 6632458.84484416991472244, -346546.9152928851544857 6632444.29659010842442513, -346522.40419847227167338 6632307.79493184108287096)')
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        req.setTolerance(20)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        results = generator.takeResults()
        self.assertEqual(results.distanceToHeightMap(),
                         {1158.036781085277: 232.75, 1171.3215208424097: 235.5, 1196.7677444714948: 241.0})

    def testLineGenerationAbsolute(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in ['LineStringZ(322006 129874 12, 322008 129910 13, 322038 129909 14, 322037 129868 15)',
                     'LineStringZ(322068 129900 16, 322128 129813 17)',
                     'LineStringZ(321996 129914 11, 321990 129896 15)',
                     'LineStringZ(321595 130176 1, 321507 130104 10)',
                     'LineStringZ(321558 129930 1, 321568 130029 10, 321516 130049 5)',
                     'LineStringZ(321603 129967 3, 321725 130042 9)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setZScale(2.5)
        vl.elevationProperties().setZOffset(10)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346715.98066368483705446 6632458.84484416991472244, -346546.9152928851544857 6632444.29659010842442513, -346522.40419847227167338 6632307.79493184108287096)')
        req = QgsProfileRequest(curve)

        generator = vl.createProfileGenerator(req)
        self.assertIsNotNone(generator)
        # the request did not have the crs of the linestring set, so the whole linestring falls outside the vector extent
        self.assertFalse(generator.generateProfile())

        # set correct crs for linestring and re-try
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(results.distanceToHeightMap(),
                         {1108.6091142774105: 46.0657436711823, 1170.8608501804001: 50.35685788534323,
                          1261.837923505105: 51.45554352378668, 499.14451783650344: 26.272194098518074,
                          1032.846388258636: 47.24575163804771, 1060.026172715397: 41.43036484322663})

    def testLineGenerationTerrain(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in ['LineStringZ(322006 129874 12, 322008 129910 13, 322038 129909 14, 322037 129868 15)',
                     'LineStringZ(322068 129900 16, 322128 129813 17)',
                     'LineStringZ(321996 129914 11, 321990 129896 15)',
                     'LineStringZ(321595 130176 1, 321507 130104 10)',
                     'LineStringZ(321558 129930 1, 321568 130029 10, 321516 130049 5)',
                     'LineStringZ(321603 129967 3, 321725 130042 9)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Terrain)
        vl.elevationProperties().setZScale(2.5)
        vl.elevationProperties().setZOffset(10)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346715.98066368483705446 6632458.84484416991472244, -346546.9152928851544857 6632444.29659010842442513, -346522.40419847227167338 6632307.79493184108287096)')
        req = QgsProfileRequest(curve)

        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())
        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(rl)
        terrain_provider.setScale(0.3)
        terrain_provider.setOffset(-5)
        req.setTerrainProvider(terrain_provider)

        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(results.distanceToHeightMap(),
                         {1060.026172715397: 50.0, 1170.8608501804001: 58.25, 499.14451783650344: 65.75,
                          1261.837923505105: 56.0, 1108.6091142774105: 53.75, 1032.846388258636: 49.25})

    def testLineGenerationRelative(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in ['LineStringZ(322006 129874 12, 322008 129910 13, 322038 129909 14, 322037 129868 15)',
                     'LineStringZ(322068 129900 16, 322128 129813 17)',
                     'LineStringZ(321996 129914 11, 321990 129896 15)',
                     'LineStringZ(321595 130176 1, 321507 130104 10)',
                     'LineStringZ(321558 129930 1, 321568 130029 10, 321516 130049 5)',
                     'LineStringZ(321603 129967 3, 321725 130042 9)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Relative)
        vl.elevationProperties().setZScale(2.5)
        vl.elevationProperties().setZOffset(10)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346715.98066368483705446 6632458.84484416991472244, -346546.9152928851544857 6632444.29659010842442513, -346522.40419847227167338 6632307.79493184108287096)')
        req = QgsProfileRequest(curve)

        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())
        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(rl)
        terrain_provider.setScale(0.3)
        terrain_provider.setOffset(-5)
        req.setTerrainProvider(terrain_provider)

        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(results.distanceToHeightMap(),
                         {1170.8608501804001: 98.60685788534323, 1108.6091142774105: 89.81574367118229,
                          1261.837923505105: 97.45554352378669, 1060.026172715397: 81.43036484322663,
                          499.14451783650344: 82.02219409851807, 1032.846388258636: 86.49575163804771})

    def testLineGenerationRelativeExtrusion(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in ['LineStringZ(322006 129874 12, 322008 129910 13, 322038 129909 14, 322037 129868 15)',
                     'LineStringZ(322068 129900 16, 322128 129813 17)',
                     'LineStringZ(321996 129914 11, 321990 129896 15)',
                     'LineStringZ(321595 130176 1, 321507 130104 10)',
                     'LineStringZ(321558 129930 1, 321568 130029 10, 321516 130049 5)',
                     'LineStringZ(321603 129967 3, 321725 130042 9)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Relative)
        vl.elevationProperties().setZScale(2.5)
        vl.elevationProperties().setZOffset(10)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(7)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346715.98066368483705446 6632458.84484416991472244, -346546.9152928851544857 6632444.29659010842442513, -346522.40419847227167338 6632307.79493184108287096)')
        req = QgsProfileRequest(curve)

        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())
        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(rl)
        terrain_provider.setScale(0.3)
        terrain_provider.setOffset(-5)
        req.setTerrainProvider(terrain_provider)

        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(results.distanceToHeightMap(),
                         {1170.8608501804001: 98.60685788534323, 1108.6091142774105: 89.81574367118229,
                          1261.837923505105: 97.45554352378669, 1060.026172715397: 81.43036484322663,
                          499.14451783650344: 82.02219409851807, 1032.846388258636: 86.49575163804771})

        self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                              ['LineStringZ (-346689.8 6632456.6 81.4, -346689.8 6632456.6 88.4)',
                               'LineStringZ (-346641.4 6632452.4 89.8, -346641.4 6632452.4 96.8)',
                               'LineStringZ (-346579.4 6632447.1 98.6, -346579.4 6632447.1 105.6)',
                               'LineStringZ (-346536.6 6632386.8 97.5, -346536.6 6632386.8 104.5)',
                               'LineStringZ (-346716.9 6632459.1 86.5, -346716.9 6632459.1 93.5)',
                               'LineStringZ (-347221.2 6632633.7 82, -347221.2 6632633.7 89)'])


if __name__ == '__main__':
    unittest.main()
