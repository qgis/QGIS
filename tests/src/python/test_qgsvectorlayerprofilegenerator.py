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


if __name__ == '__main__':
    unittest.main()
