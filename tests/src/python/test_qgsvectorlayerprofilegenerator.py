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

    def testPolygonGenerationAbsolute(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in [
            'PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
            'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))',
            'PolygonZ ((321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3))',
            'PolygonZ ((321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4))',
                'PolygonZ ((322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5))']:
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
                         {884.1318765564429: 15.000000000000002, 919.3431683416284: 17.5,
                          1033.7715194706846: 20.000000000000004, 915.7825093365038: 14.999999999999998,
                          1029.4609990659083: 20.00000000006354, 1342.1465639888847: 22.5,
                          1027.2108421957003: 20.000000000101235, 921.5565012488089: 17.50000000021578,
                          905.8450977646135: 15.0, 912.2596841546431: 15.0, 931.6676768014222: 17.5,
                          1039.0310942115923: 20.0000000001776, 890.4232520687127: 15.0, 882.5230631598915: 15.0,
                          1334.3459450808623: 22.5, 1338.3733883565753: 22.50000000008145})

    def testPolygonGenerationTerrain(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in [
            'PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
            'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))',
            'PolygonZ ((321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3))',
            'PolygonZ ((321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4))',
                'PolygonZ ((322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5))']:
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
                         {1342.1465639888847: 53.0, 915.7825093365038: 55.24999999999999,
                          921.5565012488089: 54.500000000672, 1334.3459450808623: 53.0,
                          1338.3733883565753: 53.00000000019186, 884.1318765564429: 55.25, 1033.7715194706846: 49.25,
                          919.3431683416284: 54.5, 1029.4609990659083: 49.25000000015647,
                          1027.2108421957003: 49.25000000024929, 905.8450977646135: 55.25,
                          912.2596841546431: 55.25000000000001, 931.6676768014222: 54.5, 890.4232520687127: 55.25,
                          882.5230631598915: 55.24999999999999, 1039.0310942115923: 49.25000000043733})

    def testPolygonGenerationRelative(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in [
            'PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
            'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))',
            'PolygonZ ((321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3))',
            'PolygonZ ((321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4))',
                'PolygonZ ((322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5))']:
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
                         {1027.2108421957003: 59.250000000299906, 915.7825093365038: 60.24999999999999,
                          882.5230631598915: 60.25, 905.8450977646135: 60.25000000000001, 1334.3459450808623: 65.5,
                          919.3431683416284: 62.0, 931.6676768014222: 62.00000000000001, 884.1318765564429: 60.25,
                          1029.4609990659083: 59.25000000018824, 912.2596841546431: 60.25,
                          1039.0310942115923: 59.250000000526136, 1342.1465639888847: 65.5,
                          921.5565012488089: 62.00000000076449, 890.4232520687127: 60.25, 1033.7715194706846: 59.25,
                          1338.3733883565753: 65.50000000023711})

        self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                              ['LineStringZ (-346857.4 6632507.8 60.3, -346851.4 6632505.7 60.3)',
                               'LineStringZ (-346858.9 6632508.3 60.3, -346857.4 6632507.8 60.3)',
                               'LineStringZ (-346830.8 6632498.6 60.3, -346827.5 6632497.4 60.2)',
                               'LineStringZ (-346836.9 6632500.7 60.3, -346830.8 6632498.6 60.3)',
                               'LineStringZ (-346824.1 6632496.3 62, -346822 6632495.5 62)',
                               'LineStringZ (-346822 6632495.5 62, -346812.5 6632492.2 62)',
                               'LineStringZ (-346722.2 6632461 59.3, -346720.1 6632460.3 59.3)',
                               'LineStringZ (-346720.1 6632460.3 59.3, -346716 6632458.8 59.3, -346710.7 6632458.4 59.3)',
                               'LineStringZ (-346523.8 6632315.5 65.5, -346523.1 6632311.5 65.5)',
                               'LineStringZ (-346523.1 6632311.5 65.5, -346522.4 6632307.8 65.5)'])

    def testPolygonGenerationRelativeExtrusion(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in [
            'PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
            'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))',
            'PolygonZ ((321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3))',
            'PolygonZ ((321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4))',
                'PolygonZ ((322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5))']:
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
                         {1027.2108421957003: 59.250000000299906, 915.7825093365038: 60.24999999999999,
                          882.5230631598915: 60.25, 905.8450977646135: 60.25000000000001, 1334.3459450808623: 65.5,
                          919.3431683416284: 62.0, 931.6676768014222: 62.00000000000001, 884.1318765564429: 60.25,
                          1029.4609990659083: 59.25000000018824, 912.2596841546431: 60.25,
                          1039.0310942115923: 59.250000000526136, 1342.1465639888847: 65.5,
                          921.5565012488089: 62.00000000076449, 890.4232520687127: 60.25, 1033.7715194706846: 59.25,
                          1338.3733883565753: 65.50000000023711})

        self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                              [
                                  'PolygonZ ((-346857.4 6632507.8 60.3, -346851.4 6632505.7 60.3, -346851.4 6632505.7 67.3, -346857.4 6632507.8 67.3, -346857.4 6632507.8 60.3))',
                                  'PolygonZ ((-346858.9 6632508.3 60.3, -346857.4 6632507.8 60.3, -346857.4 6632507.8 67.3, -346858.9 6632508.3 67.3, -346858.9 6632508.3 60.3))',
                                  'PolygonZ ((-346830.8 6632498.6 60.3, -346827.5 6632497.4 60.2, -346827.5 6632497.4 67.3, -346830.8 6632498.6 67.3, -346830.8 6632498.6 60.3))',
                                  'PolygonZ ((-346836.9 6632500.7 60.3, -346830.8 6632498.6 60.3, -346830.8 6632498.6 67.3, -346836.9 6632500.7 67.3, -346836.9 6632500.7 60.3))',
                                  'PolygonZ ((-346824.1 6632496.3 62, -346822 6632495.5 62, -346822 6632495.5 69, -346824.1 6632496.3 69, -346824.1 6632496.3 62))',
                                  'PolygonZ ((-346822 6632495.5 62, -346812.5 6632492.2 62, -346812.5 6632492.2 69, -346822 6632495.5 69, -346822 6632495.5 62))',
                                  'PolygonZ ((-346722.2 6632461 59.3, -346720.1 6632460.3 59.3, -346720.1 6632460.3 66.3, -346722.2 6632461 66.3, -346722.2 6632461 59.3))',
                                  'PolygonZ ((-346720.1 6632460.3 59.3, -346716 6632458.8 59.3, -346710.7 6632458.4 59.3, -346710.7 6632458.4 66.3, -346716 6632458.8 66.3, -346720.1 6632460.3 66.3, -346720.1 6632460.3 59.3))',
                                  'PolygonZ ((-346523.8 6632315.5 65.5, -346523.1 6632311.5 65.5, -346523.1 6632311.5 72.5, -346523.8 6632315.5 72.5, -346523.8 6632315.5 65.5))',
                                  'PolygonZ ((-346523.1 6632311.5 65.5, -346522.4 6632307.8 65.5, -346522.4 6632307.8 72.5, -346523.1 6632311.5 72.5, -346523.1 6632311.5 65.5))'])


if __name__ == '__main__':
    unittest.main()
