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
import math

from qgis.PyQt.QtCore import QDir
from qgis.core import (
    Qgis,
    QgsCategorizedSymbolRenderer,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransformContext,
    QgsDoubleRange,
    QgsFeature,
    QgsFillSymbol,
    QgsGeometry,
    QgsLineString,
    QgsLineSymbol,
    QgsMapLayerElevationProperties,
    QgsMarkerSymbol,
    QgsProfileIdentifyContext,
    QgsProfilePlotRenderer,
    QgsProfilePoint,
    QgsProfileRequest,
    QgsProfileSnapContext,
    QgsProjUtils,
    QgsProperty,
    QgsRasterDemTerrainProvider,
    QgsRasterLayer,
    QgsRendererCategory,
    QgsSymbolLayer,
    QgsVectorLayer,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsVectorLayerProfileGenerator(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "profile_chart"

    @staticmethod
    def round_dict(val, places):
        return {round(k, places): round(val[k], places) for k in val.keys() if not math.isnan(val[k])}

    def create_transform_context(self):
        context = QgsCoordinateTransformContext()
        # ensure grids are never used, so that we have a common transformation result
        context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:27700'),
                                       QgsCoordinateReferenceSystem('EPSG:4326'),
                                       '+proj=pipeline +step +inv +proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 +ellps=airy +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1')
        context.addCoordinateOperation(QgsCoordinateReferenceSystem('EPSG:27700'),
                                       QgsCoordinateReferenceSystem('EPSG:3857'),
                                       '+proj=pipeline +step +inv +proj=tmerc +lat_0=49 +lon_0=-2 +k=0.9996012717 +x_0=400000 +y_0=-100000 +ellps=airy +step +proj=webmerc +lat_0=0 +lon_0=0 +x_0=0 +y_0=0 +ellps=WGS84')
        return context

    def testPointGenerationAbsolute(self):
        vl = QgsVectorLayer(os.path.join(unitTestDataPath(), '3d', 'points_with_z.shp'), 'trees')
        self.assertTrue(vl.isValid())

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setZScale(2.5)
        vl.elevationProperties().setZOffset(10)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-347557.39478182751918212 6632716.59644229710102081, -346435.3875386503059417 6632277.86440025269985199, -346234.1061912341392599 6632242.03022097796201706, -346185.31071307259844616 6632150.53869942482560873)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        generator = vl.createProfileGenerator(req)
        self.assertIsNotNone(generator)
        # the request did not have the crs of the linestring set, so the whole linestring falls outside the vector extent
        self.assertFalse(generator.generateProfile())

        # set correct crs for linestring and re-try
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        # no tolerance => no hits
        results = generator.takeResults()
        self.assertFalse(results.distanceToHeightMap())

        req.setTolerance(90)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {31.2: 274.0, 1223.2: 227.2, 1213.4: 241.0, 175.6: 274.0, 1242.5: 221.8, 1172.3: 235.5,
                          1159.1: 232.8})

        # lower tolerance
        req.setTolerance(15)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()
        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {31.2: 274.0, 175.6: 274.0, 1242.5: 221.8})

        self.assertAlmostEqual(results.zRange().lower(), 221.75, 2)
        self.assertAlmostEqual(results.zRange().upper(), 274.0, 2)

        features = results.asFeatures(Qgis.ProfileExportType.Features3D)
        self.assertEqual(len(features), 3)
        self.assertEqual(features[0].layerIdentifier, vl.id())
        self.assertCountEqual([f.geometry.asWkt(-2) for f in features],
                              ['PointZ (-347400 6632600 300)', 'PointZ (-347500 6632700 300)', 'PointZ (-346400 6632300 200)'])

        features = results.asFeatures(Qgis.ProfileExportType.Profile2D)
        self.assertEqual(len(features), 3)
        self.assertEqual(features[0].layerIdentifier, vl.id())
        self.assertCountEqual([f.geometry.asWkt(-2) for f in features],
                              ['Point (200 300)', 'Point (0 300)', 'Point (1200 200)'])

        features = results.asFeatures(Qgis.ProfileExportType.DistanceVsElevationTable)
        self.assertEqual(len(features), 3)
        self.assertEqual(features[0].layerIdentifier, vl.id())
        self.assertCountEqual([f.attributes['distance'] for f in features],
                              [31.204980351872074, 175.61729584080234,
                               1242.5349752853108])
        self.assertCountEqual([f.attributes['elevation'] for f in features],
                              [274.0, 274.0,
                               221.75])
        self.assertCountEqual([f.geometry.asWkt(-1) for f in features],
                              ['PointZ (-347530 6632710 270)', 'PointZ (-347390 6632650 270)', 'PointZ (-346400 6632270 220)'])

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
            'LineString (-347557.39478182751918212 6632716.59644229710102081, -346435.3875386503059417 6632277.86440025269985199, -346234.1061912341392599 6632242.03022097796201706, -346185.31071307259844616 6632150.53869942482560873)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())
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

        # 15 meters tolerance
        req.setTolerance(15)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {175.6: 69.5, 31.2: 69.5, 1242.5: 55.2})
            self.assertAlmostEqual(results.zRange().lower(), 55.249, 2)
            self.assertAlmostEqual(results.zRange().upper(), 69.5, 2)
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {31.2: 67.2, 175.6: 65.8, 1242.5: 52.2})
            self.assertAlmostEqual(results.zRange().lower(), 52.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 67.25, 2)

        # 70 meters tolerance
        req.setTolerance(70)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {175.6: 69.5, 31.2: 69.5, 1223.2: 56.8, 1242.5: 55.2})
            self.assertAlmostEqual(results.zRange().lower(), 55.249, 2)
            self.assertAlmostEqual(results.zRange().upper(), 69.5, 2)
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {31.2: 67.2, 175.6: 65.8, 1242.5: 52.2})
            self.assertAlmostEqual(results.zRange().lower(), 52.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 67.25, 2)

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
            'LineString (-347557.39478182751918212 6632716.59644229710102081, -346435.3875386503059417 6632277.86440025269985199, -346234.1061912341392599 6632242.03022097796201706, -346185.31071307259844616 6632150.53869942482560873)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())
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

        req.setTolerance(15)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()
        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {31.2: 333.5, 175.6: 333.5, 1242.5: 267.0})
            self.assertAlmostEqual(results.zRange().lower(), 267.0, 2)
            self.assertAlmostEqual(results.zRange().upper(), 333.5, 2)
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {31.2: 331.2, 175.6: 329.8, 1242.5: 264.0})
            self.assertAlmostEqual(results.zRange().lower(), 264.0, 2)
            self.assertAlmostEqual(results.zRange().upper(), 331.25, 2)

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
            'LineString (-347557.39478182751918212 6632716.59644229710102081, -346435.3875386503059417 6632277.86440025269985199, -346234.1061912341392599 6632242.03022097796201706, -346185.31071307259844616 6632150.53869942482560873)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())
        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(rl)
        terrain_provider.setScale(0.3)
        terrain_provider.setOffset(-5)

        req.setTerrainProvider(terrain_provider)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        req.setTolerance(15)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {31.2: 333.5, 175.6: 333.5, 1242.5: 267.0})
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {31.2: 331.2, 175.6: 329.8, 1242.5: 264.0})

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  ['LineStringZ (-347395 6632649.6 333.5, -347395 6632649.6 340.5)',
                                   'LineStringZ (-347533.4 6632692.2 333.5, -347533.4 6632692.2 340.5)',
                                   'LineStringZ (-346399.2 6632265.6 267, -346399.2 6632265.6 274)'])
            self.assertAlmostEqual(results.zRange().lower(), 267.0, 2)
            self.assertAlmostEqual(results.zRange().upper(), 340.5, 2)
        else:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  ['LineStringZ (-347395 6632649.6 329.8, -347395 6632649.6 336.8)',
                                   'LineStringZ (-347533.4 6632692.2 331.3, -347533.4 6632692.2 338.3)',
                                   'LineStringZ (-346399.2 6632265.6 264, -346399.2 6632265.6 271)'])
            self.assertAlmostEqual(results.zRange().lower(), 264.0, 2)
            self.assertAlmostEqual(results.zRange().upper(), 338.25, 2)

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
            'LineString (-347557.39478182751918212 6632716.59644229710102081, -346435.3875386503059417 6632277.86440025269985199, -346234.1061912341392599 6632242.03022097796201706, -346185.31071307259844616 6632150.53869942482560873)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        req.setTolerance(110)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        results = generator.takeResults()
        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {1158.2: 232.8, 1172.4: 235.5, 1196.5: 241.0})

        self.assertAlmostEqual(results.zRange().lower(), 232.75, 2)
        self.assertAlmostEqual(results.zRange().upper(), 241.0, 2)

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
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346576.99897185183363035 6632367.38372825458645821, -346396.02439485350623727 6632344.35087973903864622, -346374.34608158958144486 6632220.09952207934111357)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        generator = vl.createProfileGenerator(req)
        self.assertIsNotNone(generator)
        # the request did not have the crs of the linestring set, so the whole linestring falls outside the vector extent
        self.assertFalse(generator.generateProfile())

        # set correct crs for linestring and re-try
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {675.2: 27.7, 1195.7: 47.5, 1223.1: 41.4, 1272.0: 46.2, 1339.4: 50.5, 1444.4: 51.8})

        self.assertAlmostEqual(results.zRange().lower(), 27.7064, 2)
        self.assertAlmostEqual(results.zRange().upper(), 51.7598, 2)

    def testLineGenerationFollowingLinestringExactly(self):
        vl = QgsVectorLayer('MultiLineString?crs=EPSG:3857', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in ['MultiLineString ((872381.44973557780031115 6035318.57090197317302227, 868258.19322114891838282 6039288.30190788581967354, 870254.02483185648452491 6048565.62906535062938929, 884224.8461068095639348 6045999.55985158402472734, 876087.99415546329692006 6035472.0964104887098074))']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString(872381.44973557780031115 6035318.57090197317302227, 868258.19322114891838282 6039288.30190788581967354, 870254.02483185648452491 6048565.62906535062938929, 884224.8461068095639348 6045999.55985158402472734, 876087.99415546329692006 6035472.0964104887098074)')
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {0.0: 0.0, 5723.6: 0.0, 15213.2: 0.0, 29417.7: 0.0, 42723.2: 0.0})

        self.assertAlmostEqual(results.zRange().lower(), 0.0, 2)
        self.assertAlmostEqual(results.zRange().upper(), 0.0, 2)

        # try with just a part of the original linestring as the capture curve
        curve = QgsLineString()
        curve.fromWkt(
            'LineString (872381.44973557780031115 6035318.57090197317302227, 868258.19322114891838282 6039288.30190788581967354, 870254.02483185648452491 6048565.62906535062938929)')
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {0.0: 0.0, 5723.6: 0.0, 15213.2: 0.0})

        self.assertAlmostEqual(results.zRange().lower(), 0.0, 2)
        self.assertAlmostEqual(results.zRange().upper(), 0.0, 2)

    def testLineGenerationFollowingLinestringZExactly(self):
        vl = QgsVectorLayer('MultiLineStringZ?crs=EPSG:3857', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in ['MultiLineStringZ ((872381.44973557780031115 6035318.57090197317302227 690, 868258.19322114891838282 6039288.30190788581967354 705, 870254.02483185648452491 6048565.62906535062938929 680, 884224.8461068095639348 6045999.55985158402472734 700, 876087.99415546329692006 6035472.0964104887098074 710))']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)

        curve = QgsLineString()
        curve.fromWkt(
            'LineStringZ (872381.44973557780031115 6035318.57090197317302227 690, 868258.19322114891838282 6039288.30190788581967354 705, 870254.02483185648452491 6048565.62906535062938929 680, 884224.8461068095639348 6045999.55985158402472734 700, 876087.99415546329692006 6035472.0964104887098074 710)')
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {0.0: 690.0, 5723.6: 705.0, 15213.2: 680.0, 29417.7: 700.0, 42723.2: 710.0})

        self.assertAlmostEqual(results.zRange().lower(), 680.0, 2)
        self.assertAlmostEqual(results.zRange().upper(), 710.0, 2)

        # try with just a part of the original linestring as the capture curve
        curve = QgsLineString()
        curve.fromWkt(
            'LineString (872381.44973557780031115 6035318.57090197317302227, 868258.19322114891838282 6039288.30190788581967354, 870254.02483185648452491 6048565.62906535062938929)')
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {0.0: 690.0, 5723.6: 705.0, 15213.2: 680.0})

        self.assertAlmostEqual(results.zRange().lower(), 680.0, 2)
        self.assertAlmostEqual(results.zRange().upper(), 705.0, 2)

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
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346576.99897185183363035 6632367.38372825458645821, -346396.02439485350623727 6632344.35087973903864622, -346374.34608158958144486 6632220.09952207934111357)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

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

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {675.2: 66.5, 1195.7: 49.2, 1223.1: 50.0, 1272.0: 53.8, 1339.4: 58.2, 1444.4: 58.2})
            self.assertAlmostEqual(results.zRange().lower(), 49.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 66.5, 2)
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {675.2: 62.7, 1195.7: 53.0, 1223.1: 56.0, 1272.0: 58.2, 1339.4: 57.5, 1444.4: 52.2})
            self.assertAlmostEqual(results.zRange().lower(), 52.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 62.7499, 2)

    def testLineGenerationTerrainTolerance(self):
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
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346576.99897185183363035 6632367.38372825458645821, -346396.02439485350623727 6632344.35087973903864622, -346374.34608158958144486 6632220.09952207934111357)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())
        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(rl)
        terrain_provider.setScale(0.3)
        terrain_provider.setOffset(-5)
        req.setTerrainProvider(terrain_provider)

        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        # very small tolerance
        req.setTolerance(0.1)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 2),
                             {675.09: 66.5, 675.24: 66.5, 1195.73: 49.25, 1195.74: 49.25, 1223.14: 50.0,
                              1223.15: 50.0, 1271.97: 53.75, 1271.99: 53.75, 1339.33: 58.25, 1339.51: 58.25,
                              1444.18: 58.25, 1444.59: 58.25})
            self.assertAlmostEqual(results.zRange().lower(), 49.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 66.5, 2)
        else:
            # TODO find a way to test with an older proj version
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 2),
                             {675.09: 66.5, 675.24: 66.5, 1195.73: 49.25, 1195.74: 49.25, 1223.14: 50.0,
                              1223.15: 50.0, 1271.97: 53.75, 1271.99: 53.75, 1339.33: 58.25, 1339.51: 58.25,
                              1444.18: 58.25, 1444.59: 58.25})
            self.assertAlmostEqual(results.zRange().lower(), 52.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 62.7499, 2)

        # 1 meter tolerance
        req.setTolerance(1)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 2),
                             {674.43: 66.5, 675.91: 66.5, 1195.73: 49.25, 1195.91: 49.25, 1223.06: 50.0,
                              1223.23: 50.0, 1271.86: 53.75, 1272.1: 53.75, 1338.5: 58.25, 1340.34: 58.25,
                              1442.29: 56.75, 1446.48: 57.5})
            self.assertAlmostEqual(results.zRange().lower(), 49.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 66.5, 2)
        else:
            # TODO find a way to test with an older proj version
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 2),
                             {675.09: 66.5, 675.24: 66.5, 1195.73: 49.25, 1195.74: 49.25, 1223.14: 50.0,
                              1223.15: 50.0, 1271.97: 53.75, 1271.99: 53.75, 1339.33: 58.25, 1339.51: 58.25,
                              1444.18: 58.25, 1444.59: 58.25})
            self.assertAlmostEqual(results.zRange().lower(), 52.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 62.7499, 2)

        # 15 meters tolerance
        req.setTolerance(15)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 2),
                             {664.1: 65.75, 686.24: 67.25, 1195.73: 49.25, 1198.44: 49.25, 1221.85: 50.0,
                              1224.44: 49.25, 1270.21: 54.5, 1273.75: 53.0, 1325.61: 59.0, 1353.23: 57.5,
                              1412.92: 56.0, 1475.85: 57.5})
            self.assertAlmostEqual(results.zRange().lower(), 49.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 67.25, 2)
        else:
            # TODO find a way to test with an older proj version
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 2),
                             {675.09: 66.5, 675.24: 66.5, 1195.73: 49.25, 1195.74: 49.25, 1223.14: 50.0,
                              1223.15: 50.0, 1271.97: 53.75, 1271.99: 53.75, 1339.33: 58.25, 1339.51: 58.25,
                              1444.18: 58.25, 1444.59: 58.25})
            self.assertAlmostEqual(results.zRange().lower(), 52.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 62.7499, 2)

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
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346576.99897185183363035 6632367.38372825458645821, -346396.02439485350623727 6632344.35087973903864622, -346374.34608158958144486 6632220.09952207934111357)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

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

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {675.2: 84.2, 1195.7: 86.8, 1223.1: 81.4, 1272.0: 90.0, 1339.4: 98.7, 1444.4: 100.0})
            self.assertAlmostEqual(results.zRange().lower(), 81.358, 2)
            self.assertAlmostEqual(results.zRange().upper(), 100.009, 2)
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {675.2: 80.5, 1195.7: 90.5, 1223.1: 87.4, 1272.0: 94.5, 1339.4: 98.0, 1444.4: 94.0})
            self.assertAlmostEqual(results.zRange().lower(), 80.4564, 2)
            self.assertAlmostEqual(results.zRange().upper(), 97.9811, 2)

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
            'LineString (-347692.88994020794052631 6632796.97473032586276531, -346576.99897185183363035 6632367.38372825458645821, -346396.02439485350623727 6632344.35087973903864622, -346374.34608158958144486 6632220.09952207934111357)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

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

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {675.2: 84.2, 1195.7: 86.8, 1223.1: 81.4, 1272.0: 90.0, 1339.4: 98.7, 1444.4: 100.0})
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {675.2: 80.5, 1195.7: 90.5, 1223.1: 87.4, 1272.0: 94.5, 1339.4: 98.0, 1444.4: 94.0})

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  ['LineStringZ (-346549.8 6632363.9 81.4, -346549.8 6632363.9 88.4)',
                                   'LineStringZ (-346501.4 6632357.8 90, -346501.4 6632357.8 97)',
                                   'LineStringZ (-346434.5 6632349.2 98.7, -346434.5 6632349.2 105.7)',
                                   'LineStringZ (-346384.6 6632279.1 100, -346384.6 6632279.1 107)',
                                   'LineStringZ (-346577 6632367.4 86.8, -346577 6632367.4 93.8)',
                                   'LineStringZ (-347062.8 6632554.4 84.2, -347062.8 6632554.4 91.2)'])
            self.assertAlmostEqual(results.zRange().lower(), 81.3588, 2)
            self.assertAlmostEqual(results.zRange().upper(), 107.009, 2)
        else:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  ['LineStringZ (-346549.8 6632363.9 87.4, -346549.8 6632363.9 94.4)',
                                   'LineStringZ (-346501.4 6632357.8 94.5, -346501.4 6632357.8 101.5)',
                                   'LineStringZ (-346434.5 6632349.2 98, -346434.5 6632349.2 105)',
                                   'LineStringZ (-346384.6 6632279.1 94, -346384.6 6632279.1 101)',
                                   'LineStringZ (-346577 6632367.4 90.5, -346577 6632367.4 97.5)',
                                   'LineStringZ (-347062.8 6632554.4 80.5, -347062.8 6632554.4 87.5)'])
            self.assertAlmostEqual(results.zRange().lower(), 80.45645, 2)
            self.assertAlmostEqual(results.zRange().upper(), 104.9811499, 2)

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
            'LineString (-347701.59207547508412972 6632766.96282589063048363, -346577.00878971704514697 6632369.7371364813297987, -346449.93654899462126195 6632331.81857067719101906, -346383.52035177784273401 6632216.85897350125014782)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        generator = vl.createProfileGenerator(req)
        self.assertIsNotNone(generator)
        # the request did not have the crs of the linestring set, so the whole linestring falls outside the vector extent
        self.assertFalse(generator.generateProfile())

        # set correct crs for linestring and re-try
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {1041.8: 15.0, 1042.4: 15.0, 1049.5: 15.0, 1070.2: 15.0, 1073.1: 15.0, 1074.8: 15.0,
                          1078.9: 17.5, 1083.9: 17.5, 1091.1: 17.5, 1186.8: 20.0, 1189.8: 20.0, 1192.7: 20.0,
                          1199.2: 20.0, 1450.0: 22.5, 1455.6: 22.5, 1458.1: 22.5})

        self.assertAlmostEqual(results.zRange().lower(), 15.0, 2)
        self.assertAlmostEqual(results.zRange().upper(), 22.5000, 2)

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
            'LineString (-347701.59207547508412972 6632766.96282589063048363, -346577.00878971704514697 6632369.7371364813297987, -346449.93654899462126195 6632331.81857067719101906, -346383.52035177784273401 6632216.85897350125014782)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

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

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.8: 55.3, 1042.4: 55.2, 1049.5: 55.2, 1070.2: 55.2, 1073.1: 55.2, 1074.8: 55.3,
                              1078.9: 54.5, 1083.9: 54.5, 1091.1: 54.5, 1186.8: 49.3, 1189.8: 49.2, 1192.7: 49.2,
                              1199.2: 49.2, 1450.0: 53.0, 1455.6: 53.0, 1458.1: 53.0})
            self.assertAlmostEqual(results.zRange().lower(), 49.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 55.250, 2)
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.8: 48.5, 1042.4: 48.5, 1049.5: 48.5, 1070.2: 48.5, 1073.1: 48.5, 1074.8: 48.5,
                              1078.9: 48.5, 1083.9: 48.5, 1091.1: 48.5, 1186.8: 52.3, 1189.8: 52.2, 1192.7: 52.2,
                              1199.2: 52.2, 1450.0: 54.5, 1455.6: 54.5, 1458.1: 54.5})
            self.assertAlmostEqual(results.zRange().lower(), 48.5, 2)
            self.assertAlmostEqual(results.zRange().upper(), 54.500000, 2)

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
            'LineString (-347701.59207547508412972 6632766.96282589063048363, -346577.00878971704514697 6632369.7371364813297987, -346449.93654899462126195 6632331.81857067719101906, -346383.52035177784273401 6632216.85897350125014782)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

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

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.8: 60.3, 1042.4: 60.2, 1049.5: 60.2, 1070.2: 60.2, 1073.1: 60.2, 1074.8: 60.3,
                              1078.9: 62.0, 1083.9: 62.0, 1091.1: 62.0, 1186.8: 59.3, 1189.8: 59.2, 1192.7: 59.2,
                              1199.2: 59.2, 1450.0: 65.5, 1455.6: 65.5, 1458.1: 65.5})
            self.assertAlmostEqual(results.zRange().lower(), 59.2499, 2)
            self.assertAlmostEqual(results.zRange().upper(), 65.5000, 2)

        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.8: 53.5, 1042.4: 53.5, 1049.5: 53.5, 1070.2: 53.5, 1073.1: 53.5, 1074.8: 53.5,
                              1078.9: 56.0, 1083.9: 56.0, 1091.1: 56.0, 1186.8: 62.3, 1189.8: 62.3, 1192.7: 62.3,
                              1199.2: 62.2, 1450.0: 67.0, 1455.6: 67.0, 1458.1: 67.0})
            self.assertAlmostEqual(results.zRange().lower(), 53.5, 2)
            self.assertAlmostEqual(results.zRange().upper(), 67.000, 2)

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  [
                                      'MultiLineStringZ ((-346718.7 6632419.8 60.3, -346712 6632417.4 60.3),(-346719.3 6632420 60.3, -346718.7 6632419.8 60.2),(-346689.7 6632409.5 60.3, -346688.2 6632409 60.3),(-346692.5 6632410.5 60.3, -346689.7 6632409.5 60.3))',
                                      'MultiLineStringZ ((-346684.3 6632407.6 62, -346679.6 6632406 62),(-346679.6 6632406 62, -346672.8 6632403.6 62))',
                                      'MultiLineStringZ ((-346582.6 6632371.7 59.3, -346579.7 6632370.7 59.3),(-346579.7 6632370.7 59.3, -346577 6632369.7 59.2, -346570.8 6632367.9 59.3))',
                                      'MultiLineStringZ ((-346387.6 6632223.9 65.5, -346384.8 6632219 65.5),(-346384.8 6632219 65.5, -346383.5 6632216.9 65.5))'])
        else:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  [
                                      'MultiLineStringZ ((-346684.3 6632407.6 56, -346679.6 6632406 56),(-346679.6 6632406 56, -346672.8 6632403.6 56))',
                                      'MultiLineStringZ ((-346718.7 6632419.8 53.5, -346712 6632417.4 53.5),(-346719.3 6632420 53.5, -346718.7 6632419.8 53.5),(-346689.7 6632409.5 53.5, -346688.2 6632409 53.5),(-346692.5 6632410.5 53.5, -346689.7 6632409.5 53.5))',
                                      'MultiLineStringZ ((-346387.6 6632223.9 67, -346384.8 6632219 67),(-346384.8 6632219 67, -346383.5 6632216.9 67))',
                                      'MultiLineStringZ ((-346582.6 6632371.7 62.3, -346579.7 6632370.7 62.3),(-346579.7 6632370.7 62.3, -346577 6632369.7 62.3, -346570.8 6632367.9 62.3))'])

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
            'LineString (-347701.59207547508412972 6632766.96282589063048363, -346577.00878971704514697 6632369.7371364813297987, -346449.93654899462126195 6632331.81857067719101906, -346383.52035177784273401 6632216.85897350125014782)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

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

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.8: 60.3, 1042.4: 60.2, 1049.5: 60.2, 1070.2: 60.2, 1073.1: 60.2, 1074.8: 60.3,
                              1078.9: 62.0, 1083.9: 62.0, 1091.1: 62.0, 1186.8: 59.3, 1189.8: 59.2, 1192.7: 59.2,
                              1199.2: 59.2, 1450.0: 65.5, 1455.6: 65.5, 1458.1: 65.5})
            self.assertAlmostEqual(results.zRange().lower(), 59.2499, 2)
            self.assertAlmostEqual(results.zRange().upper(), 72.50000, 2)
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.8: 53.5, 1042.4: 53.5, 1049.5: 53.5, 1070.2: 53.5, 1073.1: 53.5, 1074.8: 53.5,
                              1078.9: 56.0, 1083.9: 56.0, 1091.1: 56.0, 1186.8: 62.3, 1189.8: 62.3, 1192.7: 62.3,
                              1199.2: 62.2, 1450.0: 67.0, 1455.6: 67.0, 1458.1: 67.0})
            self.assertAlmostEqual(results.zRange().lower(), 53.5, 2)
            self.assertAlmostEqual(results.zRange().upper(), 74.00000, 2)

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  [
                                      'MultiPolygonZ (((-346718.7 6632419.8 60.3, -346712 6632417.4 60.3, -346712 6632417.4 67.3, -346718.7 6632419.8 67.3, -346718.7 6632419.8 60.3)),((-346719.3 6632420 60.3, -346718.7 6632419.8 60.2, -346718.7 6632419.8 67.3, -346719.3 6632420 67.3, -346719.3 6632420 60.3)),((-346689.7 6632409.5 60.3, -346688.2 6632409 60.3, -346688.2 6632409 67.3, -346689.7 6632409.5 67.3, -346689.7 6632409.5 60.3)),((-346692.5 6632410.5 60.3, -346689.7 6632409.5 60.3, -346689.7 6632409.5 67.3, -346692.5 6632410.5 67.3, -346692.5 6632410.5 60.3)))',
                                      'MultiPolygonZ (((-346684.3 6632407.6 62, -346679.6 6632406 62, -346679.6 6632406 69, -346684.3 6632407.6 69, -346684.3 6632407.6 62)),((-346679.6 6632406 62, -346672.8 6632403.6 62, -346672.8 6632403.6 69, -346679.6 6632406 69, -346679.6 6632406 62)))',
                                      'MultiPolygonZ (((-346582.6 6632371.7 59.3, -346579.7 6632370.7 59.3, -346579.7 6632370.7 66.3, -346582.6 6632371.7 66.3, -346582.6 6632371.7 59.3)),((-346579.7 6632370.7 59.3, -346577 6632369.7 59.2, -346570.8 6632367.9 59.3, -346570.8 6632367.9 66.3, -346577 6632369.7 66.3, -346579.7 6632370.7 66.3, -346579.7 6632370.7 59.3)))',
                                      'MultiPolygonZ (((-346387.6 6632223.9 65.5, -346384.8 6632219 65.5, -346384.8 6632219 72.5, -346387.6 6632223.9 72.5, -346387.6 6632223.9 65.5)),((-346384.8 6632219 65.5, -346383.5 6632216.9 65.5, -346383.5 6632216.9 72.5, -346384.8 6632219 72.5, -346384.8 6632219 65.5)))'])
        else:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  [
                                      'MultiPolygonZ (((-346684.3 6632407.6 56, -346679.6 6632406 56, -346679.6 6632406 63, -346684.3 6632407.6 63, -346684.3 6632407.6 56)),((-346679.6 6632406 56, -346672.8 6632403.6 56, -346672.8 6632403.6 63, -346679.6 6632406 63, -346679.6 6632406 56)))',
                                      'MultiPolygonZ (((-346718.7 6632419.8 53.5, -346712 6632417.4 53.5, -346712 6632417.4 60.5, -346718.7 6632419.8 60.5, -346718.7 6632419.8 53.5)),((-346719.3 6632420 53.5, -346718.7 6632419.8 53.5, -346718.7 6632419.8 60.5, -346719.3 6632420 60.5, -346719.3 6632420 53.5)),((-346689.7 6632409.5 53.5, -346688.2 6632409 53.5, -346688.2 6632409 60.5, -346689.7 6632409.5 60.5, -346689.7 6632409.5 53.5)),((-346692.5 6632410.5 53.5, -346689.7 6632409.5 53.5, -346689.7 6632409.5 60.5, -346692.5 6632410.5 60.5, -346692.5 6632410.5 53.5)))',
                                      'MultiPolygonZ (((-346387.6 6632223.9 67, -346384.8 6632219 67, -346384.8 6632219 74, -346387.6 6632223.9 74, -346387.6 6632223.9 67)),((-346384.8 6632219 67, -346383.5 6632216.9 67, -346383.5 6632216.9 74, -346384.8 6632219 74, -346384.8 6632219 67)))',
                                      'MultiPolygonZ (((-346582.6 6632371.7 62.3, -346579.7 6632370.7 62.3, -346579.7 6632370.7 69.3, -346582.6 6632371.7 69.3, -346582.6 6632371.7 62.3)),((-346579.7 6632370.7 62.3, -346577 6632369.7 62.3, -346570.8 6632367.9 62.3, -346570.8 6632367.9 69.3, -346577 6632369.7 69.3, -346579.7 6632370.7 69.3, -346579.7 6632370.7 62.3)))'])

    def testPolygonGenerationRelativeExtrusionTolerance(self):
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
            'LineString (-347701.59207547508412972 6632766.96282589063048363, -346577.00878971704514697 6632369.7371364813297987, -346449.93654899462126195 6632331.81857067719101906, -346383.52035177784273401 6632216.85897350125014782)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'), 'DTM')
        self.assertTrue(rl.isValid())
        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(rl)
        terrain_provider.setScale(0.3)
        terrain_provider.setOffset(-5)
        req.setTerrainProvider(terrain_provider)

        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        req.setTolerance(2.0)

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.0: 60.2, 1042.2: 60.2, 1042.9: 60.2, 1048.2: 60.2, 1050.8: 60.2, 1066.9: 60.2,
                              1073.4: 60.2, 1076.2: 60.2, 1077.9: 62.0, 1079.9: 62.0, 1089.9: 62.0, 1092.2: 62.0,
                              1185.4: 59.2, 1188.2: 59.2, 1192.6: 59.2, 1192.7: 59.2, 1197.9: 59.2, 1200.4: 59.2,
                              1449.3: 65.5, 1450.1: 65.5, 1451.1: 65.5, 1458.1: 65.5})
            self.assertAlmostEqual(results.zRange().lower(), 59.25, 2)
            self.assertAlmostEqual(results.zRange().upper(), 65.5, 2)
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.8: 53.5, 1042.4: 53.5, 1049.5: 53.5, 1070.2: 53.5, 1073.1: 53.5, 1074.8: 53.5,
                              1078.9: 56.0, 1083.9: 56.0, 1091.1: 56.0, 1186.8: 62.3, 1189.8: 62.3, 1192.7: 62.3,
                              1199.2: 62.2, 1450.0: 67.0, 1455.6: 67.0, 1458.1: 67.0})
            self.assertAlmostEqual(results.zRange().lower(), 53.5, 2)
            self.assertAlmostEqual(results.zRange().upper(), 74.00000, 2)

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  [
                                      'MultiLineStringZ ((-346696.3 6632409.8 60.2, -346688.9 6632411.2 60.2, -346687.5 6632406.6 60.2),(-346718.9 6632417.7 60.2, -346719.5 6632421.6 60.2, -346718.2 6632421.7 60.2, -346712.5 6632419.7 60.2, -346711.5 6632415.1 60.2))',
                                      'LineStringZ (-346684.1 6632405.4 62, -346684.6 6632409.8 62, -346673.3 6632405.9 62, -346672.4 6632401.3 62)',
                                      'LineStringZ (-346571.4 6632370.2 59.3, -346570.2 6632365.6 59.3, -346577.6 6632367.8 59.3, -346577.7 6632367.9 59.3, -346581.9 6632369.4 59.3, -346583.2 6632374 59.3, -346576.4 6632371.6 59.3)',
                                      'LineStringZ (-346381.8 6632217.9 65.5, -346385.3 6632215.9 65.5, -346388.7 6632221.9 65.5, -346387 6632224.9 65.5, -346385.8 6632224.7 65.5)'])
        else:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  [
                                      'MultiPolygonZ (((-346684.3 6632407.6 56, -346679.6 6632406 56, -346679.6 6632406 63, -346684.3 6632407.6 63, -346684.3 6632407.6 56)),((-346679.6 6632406 56, -346672.8 6632403.6 56, -346672.8 6632403.6 63, -346679.6 6632406 63, -346679.6 6632406 56)))',
                                      'MultiPolygonZ (((-346718.7 6632419.8 53.5, -346712 6632417.4 53.5, -346712 6632417.4 60.5, -346718.7 6632419.8 60.5, -346718.7 6632419.8 53.5)),((-346719.3 6632420 53.5, -346718.7 6632419.8 53.5, -346718.7 6632419.8 60.5, -346719.3 6632420 60.5, -346719.3 6632420 53.5)),((-346689.7 6632409.5 53.5, -346688.2 6632409 53.5, -346688.2 6632409 60.5, -346689.7 6632409.5 60.5, -346689.7 6632409.5 53.5)),((-346692.5 6632410.5 53.5, -346689.7 6632409.5 53.5, -346689.7 6632409.5 60.5, -346692.5 6632410.5 60.5, -346692.5 6632410.5 53.5)))',
                                      'MultiPolygonZ (((-346387.6 6632223.9 67, -346384.8 6632219 67, -346384.8 6632219 74, -346387.6 6632223.9 74, -346387.6 6632223.9 67)),((-346384.8 6632219 67, -346383.5 6632216.9 67, -346383.5 6632216.9 74, -346384.8 6632219 74, -346384.8 6632219 67)))',
                                      'MultiPolygonZ (((-346582.6 6632371.7 62.3, -346579.7 6632370.7 62.3, -346579.7 6632370.7 69.3, -346582.6 6632371.7 69.3, -346582.6 6632371.7 62.3)),((-346579.7 6632370.7 62.3, -346577 6632369.7 62.3, -346570.8 6632367.9 62.3, -346570.8 6632367.9 69.3, -346577 6632369.7 69.3, -346579.7 6632370.7 69.3, -346579.7 6632370.7 62.3)))'])

    def test25DPolygonGeneration(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:2056', 'lines', 'memory')
        self.assertTrue(vl.isValid())

        for line in [
                'MultiPolygonZ (((2607398.48000000044703484 1228694.19700000062584877 448.28800000000046566, 2607403.2760000005364418 1228696.4050000011920929 444.7440000000060536, 2607393.68600000068545341 1228691.98900000005960464 444.7440000000060536, 2607398.48000000044703484 1228694.19700000062584877 448.28800000000046566)))']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (2607400.97201532032340765 1228697.90654633427038789, 2607405.23384975455701351 1228690.52444026106968522)')
        req = QgsProfileRequest(curve)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:2056'))

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                         {2.3: 445.6})

        self.assertAlmostEqual(results.zRange().lower(), 444.744, 2)
        self.assertAlmostEqual(results.zRange().upper(), 445.583, 2)

    def testDataDefinedExtrusionOffset(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700&field=extrusion:int&field=offset:int', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for extrusion, offset, wkt in [
            (5, 10, 'PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))'),
            (1, 6, 'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))'),
            (2, 1, 'PolygonZ ((321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3))'),
            (3, 9, 'PolygonZ ((321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4))'),
                (7, 11, 'PolygonZ ((322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5))')]:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(wkt))
            f.setAttributes([extrusion, offset])
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(17)
        vl.elevationProperties().setZOffset(34)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        generator = vl.createProfileGenerator(req)

        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                              ['MultiPolygonZ (((321906.5 129918.5 36, 321907.7 129918.8 36, 321907.7 129918.8 53, 321906.5 129918.5 53, 321906.5 129918.5 36)),((321902.8 129917.9 36, 321906.5 129918.5 36, 321906.5 129918.5 53, 321902.8 129917.9 53, 321902.8 129917.9 36)),((321917.9 129920.6 36, 321921 129921.1 36, 321921 129921.1 53, 321917.9 129920.6 53, 321917.9 129920.6 36)),((321912.4 129919.6 36, 321917.9 129920.6 36, 321917.9 129920.6 53, 321912.4 129919.6 53, 321912.4 129919.6 36)))',
                               'MultiPolygonZ (((321922.9 129921.5 37, 321927.8 129922.4 37, 321927.8 129922.4 54, 321922.9 129921.5 54, 321922.9 129921.5 37)),((321927.8 129922.4 37, 321929.5 129922.7 37, 321929.5 129922.7 54, 321927.8 129922.4 54, 321927.8 129922.4 37)))'])

        # with data defined extrusion and offset
        vl.elevationProperties().dataDefinedProperties().setProperty(QgsMapLayerElevationProperties.Property.ExtrusionHeight, QgsProperty.fromField('extrusion'))
        vl.elevationProperties().dataDefinedProperties().setProperty(QgsMapLayerElevationProperties.Property.ZOffset,
                                                                     QgsProperty.fromField('offset'))

        generator = vl.createProfileGenerator(req)

        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                              ['MultiPolygonZ (((321922.9 129921.5 4, 321927.8 129922.4 4, 321927.8 129922.4 6, 321922.9 129921.5 6, 321922.9 129921.5 4)),((321927.8 129922.4 4, 321929.5 129922.7 4, 321929.5 129922.7 6, 321927.8 129922.4 6, 321927.8 129922.4 4)))',
                               'MultiPolygonZ (((321906.5 129918.5 8, 321907.7 129918.8 8, 321907.7 129918.8 9, 321906.5 129918.5 9, 321906.5 129918.5 8)),((321902.8 129917.9 8, 321906.5 129918.5 8, 321906.5 129918.5 9, 321902.8 129917.9 9, 321902.8 129917.9 8)),((321917.9 129920.6 8, 321921 129921.1 8, 321921 129921.1 9, 321917.9 129920.6 9, 321917.9 129920.6 8)),((321912.4 129919.6 8, 321917.9 129920.6 8, 321917.9 129920.6 9, 321912.4 129919.6 9, 321912.4 129919.6 8)))'])

    def testSnappingPoints(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())
        vl.setCrs(QgsCoordinateReferenceSystem())

        for line in ['LineStringZ(322006 129874 12, 322008 129910 13, 322038 129909 14, 322037 129868 15)',
                     'LineStringZ(322068 129900 16, 322128 129813 17)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)

        curve = QgsLineString()
        curve.fromWkt('LineStringZ (322021.96201738982927054 129896.83061585001996718 0, 322116.8371042063809 129880.94244341662852094 0)')
        req = QgsProfileRequest(curve)

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()

        # try snapping some points
        context = QgsProfileSnapContext()
        res = r.snapPoint(QgsProfilePoint(-10, -10), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 1
        context.maximumPointElevationDelta = 3
        context.maximumSurfaceDistanceDelta = 0
        context.maximumSurfaceElevationDelta = 0
        res = r.snapPoint(QgsProfilePoint(15, 14), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 15.89, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 14.36, 1)

        context.maximumPointDistanceDelta = 2
        context.maximumPointElevationDelta = 2
        res = r.snapPoint(QgsProfilePoint(55, 16), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 55.279, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 16.141, 1)

        context.maximumPointDistanceDelta = 0.1
        context.maximumPointElevationDelta = 0.1
        res = r.snapPoint(QgsProfilePoint(55, 16), context)
        self.assertFalse(res.isValid())

    def testSnappingVerticalLines(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())
        vl.setCrs(QgsCoordinateReferenceSystem())

        for line in ['LineStringZ(322006 129874 12, 322008 129910 13, 322038 129909 14, 322037 129868 15)',
                     'LineStringZ(322068 129900 16, 322128 129813 17)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(17)

        curve = QgsLineString()
        curve.fromWkt('LineStringZ (322021.96201738982927054 129896.83061585001996718 0, 322116.8371042063809 129880.94244341662852094 0)')
        req = QgsProfileRequest(curve)

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()

        # try snapping some points
        context = QgsProfileSnapContext()
        res = r.snapPoint(QgsProfilePoint(-10, -10), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 1
        context.maximumPointElevationDelta = 3
        context.maximumSurfaceElevationDelta = 0
        context.maximumSurfaceDistanceDelta = 0
        res = r.snapPoint(QgsProfilePoint(15, 14), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 15.89, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 14.36, 1)

        res = r.snapPoint(QgsProfilePoint(15, 31), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 15.89, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 31.36, 1)

        res = r.snapPoint(QgsProfilePoint(15, 35), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 2
        context.maximumPointElevationDelta = 2
        res = r.snapPoint(QgsProfilePoint(55, 16), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 55.279, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 16.141, 1)

        res = r.snapPoint(QgsProfilePoint(55, 33), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 55.279, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 33.1413, 1)

        res = r.snapPoint(QgsProfilePoint(55, 36), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 0.1
        context.maximumPointElevationDelta = 0.1
        res = r.snapPoint(QgsProfilePoint(55, 16), context)
        self.assertFalse(res.isValid())

    def testSnappingPolygons(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())
        vl.setCrs(QgsCoordinateReferenceSystem())

        for poly in ['PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
                     'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(poly))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)

        curve = QgsLineString()
        curve.fromWkt('LineStringZ (321944.79089414176996797 129899.10035476912162267 0, 321818.13946245843544602 129991.70570266660070047 0)')
        req = QgsProfileRequest(curve)

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()

        # try snapping some points
        context = QgsProfileSnapContext()
        res = r.snapPoint(QgsProfilePoint(-10, -10), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 1
        context.maximumSurfaceElevationDelta = 3
        context.maximumPointElevationDelta = 0
        context.maximumSurfaceDistanceDelta = 0
        res = r.snapPoint(QgsProfilePoint(27, 1.9), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 27.37797, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 2.0, 1)

        res = r.snapPoint(QgsProfilePoint(27, 7), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 3
        context.maximumSurfaceElevationDelta = 2
        res = r.snapPoint(QgsProfilePoint(42, 3), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 40.7058, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 2.000, 1)

        context.maximumPointDistanceDelta = 0.01
        context.maximumSurfaceElevationDelta = 2
        res = r.snapPoint(QgsProfilePoint(42, 3), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 0.1
        context.maximumSurfaceElevationDelta = 0.1
        res = r.snapPoint(QgsProfilePoint(55, 16), context)
        self.assertFalse(res.isValid())

    def testSnappingExtrudedPolygons(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())
        vl.setCrs(QgsCoordinateReferenceSystem())

        for poly in ['PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
                     'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(poly))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(17)

        curve = QgsLineString()
        curve.fromWkt('LineStringZ (321944.79089414176996797 129899.10035476912162267 0, 321818.13946245843544602 129991.70570266660070047 0)')
        req = QgsProfileRequest(curve)

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()

        # try snapping some points
        context = QgsProfileSnapContext()
        res = r.snapPoint(QgsProfilePoint(-10, -10), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 1
        context.maximumSurfaceElevationDelta = 3
        context.maximumSurfaceDistanceDelta = 0
        context.maximumPointElevationDelta = 0
        res = r.snapPoint(QgsProfilePoint(27, 1.9), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 27.37797, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 2.0, 1)

        res = r.snapPoint(QgsProfilePoint(27, 18.9), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 27.37797, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 19.0, 1)

        res = r.snapPoint(QgsProfilePoint(27, 22.9), context)
        self.assertFalse(res.isValid())

        res = r.snapPoint(QgsProfilePoint(27, 7), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 3
        context.maximumSurfaceElevationDelta = 2
        res = r.snapPoint(QgsProfilePoint(42, 3), context)
        self.assertTrue(res.isValid())
        self.assertAlmostEqual(res.snappedPoint.distance(), 40.7058, 1)
        self.assertAlmostEqual(res.snappedPoint.elevation(), 2.000, 1)

        context.maximumPointDistanceDelta = 0.01
        context.maximumSurfaceElevationDelta = 2
        res = r.snapPoint(QgsProfilePoint(42, 3), context)
        self.assertFalse(res.isValid())

        context.maximumPointDistanceDelta = 0.1
        context.maximumSurfaceElevationDelta = 0.1
        res = r.snapPoint(QgsProfilePoint(55, 16), context)
        self.assertFalse(res.isValid())

    def testIdentifyPoints(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())
        vl.setCrs(QgsCoordinateReferenceSystem())

        for line in ['LineStringZ(322006 129874 12, 322008 129910 13, 322038 129909 14, 322037 129868 15)',
                     'LineStringZ(322068 129900 16, 322128 129813 17)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)

        curve = QgsLineString()
        curve.fromWkt('LineStringZ (322021.96201738982927054 129896.83061585001996718 0, 322116.8371042063809 129880.94244341662852094 0)')
        req = QgsProfileRequest(curve)

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()

        # try identifying some points
        context = QgsProfileIdentifyContext()
        res = r.identify(QgsProfilePoint(-10, -10), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 1
        context.maximumPointElevationDelta = 3
        context.maximumSurfaceDistanceDelta = 0
        context.maximumSurfaceElevationDelta = 0
        res = r.identify(QgsProfilePoint(15, 14), context)
        self.assertTrue(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.9654154289752465, 'distance': 15.895441865142377, 'elevation': 14.360847359216885, 'id': 1}])

        context.maximumPointDistanceDelta = 2
        context.maximumPointElevationDelta = 2
        res = r.identify(QgsProfilePoint(55, 16), context)
        self.assertTrue(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.3133775679752489, 'distance': 55.279676009112876, 'elevation': 16.14137478571788, 'id': 2}])

        context.maximumPointDistanceDelta = 0.1
        context.maximumPointElevationDelta = 0.1
        res = r.identify(QgsProfilePoint(55, 16), context)
        self.assertFalse(res)

        res = r.identify(QgsDoubleRange(15, 56), QgsDoubleRange(13, 17), context)
        self.assertTrue(len(res), 2)
        self.assertEqual(res[0].layer(), vl)
        self.assertCountEqual(res[0].results(), [{'id': 2}, {'id': 1}])

        res = r.identify(QgsDoubleRange(15, 36), QgsDoubleRange(13, 17), context)
        self.assertTrue(len(res), 2)
        self.assertEqual(res[0].layer(), vl)
        self.assertCountEqual(res[0].results(), [{'id': 1}])

        res = r.identify(QgsDoubleRange(25, 36), QgsDoubleRange(13, 17), context)
        self.assertFalse(res)

    def testIdentifyVerticalLines(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())
        vl.setCrs(QgsCoordinateReferenceSystem())

        for line in ['LineStringZ(322006 129874 12, 322008 129910 13, 322038 129909 14, 322037 129868 15)',
                     'LineStringZ(322068 129900 16, 322128 129813 17)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(17)

        curve = QgsLineString()
        curve.fromWkt('LineStringZ (322021.96201738982927054 129896.83061585001996718 0, 322116.8371042063809 129880.94244341662852094 0)')
        req = QgsProfileRequest(curve)

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()

        # try identifying some points
        context = QgsProfileIdentifyContext()
        res = r.identify(QgsProfilePoint(-10, -10), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 1
        context.maximumPointElevationDelta = 3
        context.maximumSurfaceElevationDelta = 0
        context.maximumSurfaceDistanceDelta = 0
        res = r.identify(QgsProfilePoint(15, 14), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.9654154289752465, 'distance': 15.895441865142377, 'elevation': 14.360847359216885, 'id': 1}])

        res = r.identify(QgsProfilePoint(15, 31), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.8954418651423772, 'distance': 15.895441865142377, 'elevation': 31.0, 'id': 1}])

        res = r.identify(QgsProfilePoint(15, 35), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 2
        context.maximumPointElevationDelta = 2
        res = r.identify(QgsProfilePoint(55, 16), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.3133775679752489, 'distance': 55.279676009112876, 'elevation': 16.14137478571788, 'id': 2}])

        res = r.identify(QgsProfilePoint(55, 33), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.27967600911287605, 'distance': 55.279676009112876, 'elevation': 33.0, 'id': 2}])

        res = r.identify(QgsProfilePoint(55, 36), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 0.1
        context.maximumPointElevationDelta = 0.1
        res = r.identify(QgsProfilePoint(55, 16), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 1
        res = r.identify(QgsProfilePoint(55, 22), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.27967600911287605, 'distance': 55.279676009112876, 'elevation': 22.0, 'id': 2}])

    def testIdentifyPolygons(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())
        vl.setCrs(QgsCoordinateReferenceSystem())

        for poly in ['PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
                     'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(poly))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)

        curve = QgsLineString()
        curve.fromWkt('LineStringZ (321944.79089414176996797 129899.10035476912162267 0, 321818.13946245843544602 129991.70570266660070047 0)')
        req = QgsProfileRequest(curve)

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()

        # try identifying some points
        context = QgsProfileIdentifyContext()
        res = r.identify(QgsProfilePoint(-10, -10), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 1
        context.maximumSurfaceElevationDelta = 3
        context.maximumPointElevationDelta = 0
        context.maximumSurfaceDistanceDelta = 0
        res = r.identify(QgsProfilePoint(27, 1.9), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.3909814462196946, 'distance': 27.377976839618572, 'elevation': 2.0, 'id': 2}])

        res = r.identify(QgsProfilePoint(27, 7), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 3
        context.maximumSurfaceElevationDelta = 2
        res = r.identify(QgsProfilePoint(42, 3), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 1.635491751097172, 'distance': 40.70584650527578, 'elevation': 2.0000000000000093, 'id': 2}])

        context.maximumPointDistanceDelta = 0.01
        context.maximumSurfaceElevationDelta = 2
        res = r.identify(QgsProfilePoint(35, 3), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.9999999999968199, 'distance': 35.0, 'elevation': 2.00000000000318, 'id': 2}])

        context.maximumPointDistanceDelta = 0.01
        context.maximumSurfaceElevationDelta = 2
        res = r.identify(QgsProfilePoint(42, 3), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 0.1
        context.maximumSurfaceElevationDelta = 0.1
        res = r.identify(QgsProfilePoint(55, 16), context)
        self.assertFalse(res)

    def testIdentifyExtrudedPolygons(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        self.assertTrue(vl.isValid())
        vl.setCrs(QgsCoordinateReferenceSystem())

        for poly in ['PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))',
                     'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(poly))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(17)

        curve = QgsLineString()
        curve.fromWkt('LineStringZ (321944.79089414176996797 129899.10035476912162267 0, 321818.13946245843544602 129991.70570266660070047 0)')
        req = QgsProfileRequest(curve)

        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())

        r = generator.takeResults()

        # try identifying some points
        context = QgsProfileIdentifyContext()
        res = r.identify(QgsProfilePoint(-10, -10), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 1
        context.maximumSurfaceElevationDelta = 3
        context.maximumSurfaceDistanceDelta = 0
        context.maximumPointElevationDelta = 0
        res = r.identify(QgsProfilePoint(27, 1.9), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.3909814462196946, 'distance': 27.377976839618572, 'elevation': 2.0, 'id': 2}])

        res = r.identify(QgsProfilePoint(27, 18.9), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.39098144621969494, 'distance': 27.377976839618572, 'elevation': 19.0, 'id': 2}])

        res = r.identify(QgsProfilePoint(27, 22.9), context)
        self.assertFalse(res)

        res = r.identify(QgsProfilePoint(27, 7), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 3
        context.maximumSurfaceElevationDelta = 2
        res = r.identify(QgsProfilePoint(42, 3), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 1.635491751097172, 'distance': 40.70584650527578, 'elevation': 2.0000000000000093, 'id': 2}])

        context.maximumPointDistanceDelta = 0
        context.maximumSurfaceElevationDelta = 0
        res = r.identify(QgsProfilePoint(35, 16), context)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].layer(), vl)
        self.assertEqual(res[0].results(), [{'delta': 0.0, 'distance': 35.0, 'elevation': 16.0, 'id': 2}])

        context.maximumPointDistanceDelta = 0.01
        context.maximumSurfaceElevationDelta = 2
        res = r.identify(QgsProfilePoint(42, 3), context)
        self.assertFalse(res)

        context.maximumPointDistanceDelta = 0.1
        context.maximumSurfaceElevationDelta = 0.1
        res = r.identify(QgsProfilePoint(55, 16), context)
        self.assertFalse(res)

    def testRenderProfile(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
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
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(7)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_polygon_no_layer_symbology', 'vector_polygon_no_layer_symbology', res))

    def testRenderProfileDataDefinedProperties(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700&field=color:string', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for color, line in [
            ('red', 'PolygonZ ((321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1))'),
            ('green', 'PolygonZ ((321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2))'),
            ('blue', 'PolygonZ ((321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3))'),
            ('purple', 'PolygonZ ((321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4))'),
                ('orange', 'PolygonZ ((322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5))')]:
            f = QgsFeature()
            f.setAttributes([color])
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(7)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        fill_symbol[0].dataDefinedProperties().setProperty(QgsSymbolLayer.Property.PropertyFillColor, QgsProperty.fromField('color'))

        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_polygon_data_defined_symbology', 'vector_polygon_data_defined_symbology', res))

    def testRenderProfileAsSurfaceLines(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setType(Qgis.VectorProfileType.ContinuousSurface)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        vl.elevationProperties().setProfileLineSymbol(line_symbol)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_lines_as_surface', 'vector_lines_as_surface', res))

    def testRenderProfileAsSurfaceLinesWithMarkers(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setType(Qgis.VectorProfileType.ContinuousSurface)
        marker_symbol = QgsMarkerSymbol.createSimple({'name': 'square', 'size': 4, 'color': '#00ff00', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileMarkerSymbol(marker_symbol)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        vl.elevationProperties().setProfileLineSymbol(line_symbol)
        vl.elevationProperties().setShowMarkerSymbolInSurfacePlots(True)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_lines_as_surface_with_markers', 'vector_lines_as_surface_with_markers', res))

    def testRenderProfileAsLineWithHoledDtm(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm_with_holes.tif'), 'DTM')
        self.assertTrue(rl.isValid())

        rl.elevationProperties().setEnabled(True)
        rl.elevationProperties().setProfileSymbology(Qgis.ProfileSurfaceSymbology.Line)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        rl.elevationProperties().setProfileLineSymbol(line_symbol)

        curve = QgsLineString()
        curve.fromWkt('LineString (320900 129000, 322900 129000)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([rl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(1600, 800, 0, curve.length(), 0, 90)
        self.assertTrue(self.image_check('vector_lines_as_line_with_holed_dtm', 'vector_lines_as_line_with_holed_dtm', res))

    def testRenderProfileAsSurfaceFillBelowWithHoledDtm(self):
        rl = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm_with_holes.tif'), 'DTM')
        self.assertTrue(rl.isValid())

        rl.elevationProperties().setEnabled(True)
        rl.elevationProperties().setProfileSymbology(Qgis.ProfileSurfaceSymbology.FillBelow)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        rl.elevationProperties().setProfileFillSymbol(fill_symbol)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        rl.elevationProperties().setProfileLineSymbol(line_symbol)

        curve = QgsLineString()
        curve.fromWkt('LineString (320900 129000, 322900 129000)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([rl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(1600, 800, 0, curve.length(), 0, 90)
        self.assertTrue(self.image_check('vector_lines_as_fill_below_surface_with_holed_dtm', 'vector_lines_as_fill_below_surface_with_holed_dtm', res))

    def testRenderProfileAsSurfaceFillBelow(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setType(Qgis.VectorProfileType.ContinuousSurface)
        vl.elevationProperties().setProfileSymbology(Qgis.ProfileSurfaceSymbology.FillBelow)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        vl.elevationProperties().setProfileLineSymbol(line_symbol)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_lines_as_fill_below_surface', 'vector_lines_as_fill_below_surface', res))

    def testRenderProfileAsSurfaceFillBelowLimit(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setType(Qgis.VectorProfileType.ContinuousSurface)
        vl.elevationProperties().setProfileSymbology(Qgis.ProfileSurfaceSymbology.FillBelow)
        vl.elevationProperties().setElevationLimit(
            1)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        vl.elevationProperties().setProfileLineSymbol(line_symbol)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_lines_as_fill_below_surface_limit', 'vector_lines_as_fill_below_surface_limit', res))

    def testRenderProfileAsSurfaceFillAbove(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setType(Qgis.VectorProfileType.ContinuousSurface)
        vl.elevationProperties().setProfileSymbology(Qgis.ProfileSurfaceSymbology.FillAbove)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        vl.elevationProperties().setProfileLineSymbol(line_symbol)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_lines_as_fill_above_surface', 'vector_lines_as_fill_above_surface', res))

    def testRenderProfileAsSurfaceFillAboveLimit(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setType(Qgis.VectorProfileType.ContinuousSurface)
        vl.elevationProperties().setProfileSymbology(Qgis.ProfileSurfaceSymbology.FillAbove)
        vl.elevationProperties().setElevationLimit(
            10)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        vl.elevationProperties().setProfileLineSymbol(line_symbol)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_lines_as_fill_above_surface_limit', 'vector_lines_as_fill_above_surface_limit', res))

    def testRenderProfileAsSurfaceFillAboveLimitTolerance(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setType(Qgis.VectorProfileType.ContinuousSurface)
        vl.elevationProperties().setProfileSymbology(Qgis.ProfileSurfaceSymbology.FillAbove)
        vl.elevationProperties().setElevationLimit(
            10)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        vl.elevationProperties().setProfileLineSymbol(line_symbol)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())
        req.setTolerance(20)

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_lines_as_fill_above_surface_limit_tolerance', 'vector_lines_as_fill_above_surface_limit_tolerance', res))

    def testRenderProfileSymbolWithMapUnits(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        marker_symbol = QgsMarkerSymbol.createSimple({'name': 'square', 'size': 4, 'color': '#00ff00', 'outline_style': 'no'})
        marker_symbol.setSizeUnit(Qgis.RenderUnit.MapUnits)
        vl.elevationProperties().setRespectLayerSymbology(False)
        vl.elevationProperties().setProfileMarkerSymbol(marker_symbol)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_profile_map_units', 'vector_profile_map_units', res))

    def testRenderProfileSymbolWithMapUnitsTolerance(self):
        vl = QgsVectorLayer('LineStringZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(vl.isValid())

        for line in [
            'LineStringZ (321829.48893365426920354 129991.38697145861806348 1, 321847.89668515208177269 129996.63588572069420479 1, 321848.97131609614007175 129979.22330882755341008 1, 321830.31725845142500475 129978.07136809575604275 1, 321829.48893365426920354 129991.38697145861806348 1)',
            'LineStringZ (321920.00953056826256216 129924.58260190498549491 2, 321924.65299345907988027 129908.43546159457764588 2, 321904.78543491888558492 129903.99811821122420952 2, 321900.80605239619035274 129931.39860145389684476 2, 321904.84799937985371798 129931.71552911199978553 2, 321908.93646715773502365 129912.90030360443051904 2, 321914.20495146053144708 129913.67693978428724222 2, 321911.30165811872575432 129923.01272751353099011 2, 321920.00953056826256216 129924.58260190498549491 2)',
            'LineStringZ (321923.10517279652412981 129919.61521573827485554 3, 321922.23537852568551898 129928.3598982143739704 3, 321928.60423935484141111 129934.22530528216157109 3, 321929.39881197665818036 129923.29054521876969375 3, 321930.55804549407912418 129916.53248518184409477 3, 321923.10517279652412981 129919.61521573827485554 3)',
            'LineStringZ (321990.47451346553862095 129909.63588680300745182 4, 321995.04325810901354998 129891.84052284323843196 4, 321989.66826330573530868 129890.5092018858413212 4, 321990.78512359503656626 129886.49917887404444627 4, 321987.37291929306229576 129885.64982962771318853 4, 321985.2254804756375961 129893.81317058412241749 4, 321987.63158903241856024 129894.41078495365218259 4, 321984.34022761805681512 129907.57450046355370432 4, 321990.47451346553862095 129909.63588680300745182 4)',
                'LineStringZ (322103.03910495212767273 129795.91051736124791205 5, 322108.25568856322206557 129804.76113295342656784 5, 322113.29666162584908307 129803.9285887333098799 5, 322117.78645010641776025 129794.48194090687320568 5, 322103.03910495212767273 129795.91051736124791205 5)']:
            f = QgsFeature()
            f.setGeometry(QgsGeometry.fromWkt(line))
            self.assertTrue(vl.dataProvider().addFeature(f))

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setRespectLayerSymbology(False)
        line_symbol = QgsLineSymbol.createSimple({'color': '#ff00ff', 'width': '0.8'})
        line_symbol.setWidthUnit(Qgis.RenderUnit.MapUnits)
        vl.elevationProperties().setProfileLineSymbol(line_symbol)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())
        req.setTolerance(10)

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_profile_map_units_tolerance', 'vector_profile_map_units_tolerance', res))

    def testRenderLayerSymbology(self):
        vl = QgsVectorLayer('PolygonZ?crs=EPSG:27700', 'lines', 'memory')
        vl.setCrs(QgsCoordinateReferenceSystem())
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
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(7)
        fill_symbol = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_style': 'no'})
        vl.elevationProperties().setRespectLayerSymbology(True)
        vl.elevationProperties().setProfileFillSymbol(fill_symbol)

        renderer = QgsCategorizedSymbolRenderer('$id', [
            QgsRendererCategory(1, QgsFillSymbol.createSimple({'color': '#0000ff', 'outline_style': 'no'}), '1'),
            QgsRendererCategory(2, QgsFillSymbol.createSimple({'color': '#00ffff', 'outline_style': 'no'}), '2'),
            QgsRendererCategory(3, QgsFillSymbol.createSimple({'color': '#3388ff', 'outline_style': 'no'}), '3'),
            QgsRendererCategory(4, QgsFillSymbol.createSimple({'color': '#883388', 'outline_style': 'no'}), '4'),
        ])
        vl.setRenderer(renderer)

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (321897.18831187387695536 129916.86947759155009408, 321942.11597351566888392 129924.94403429214435164)')
        req = QgsProfileRequest(curve)
        req.setTransformContext(self.create_transform_context())

        req.setCrs(QgsCoordinateReferenceSystem())

        plot_renderer = QgsProfilePlotRenderer([vl], req)
        plot_renderer.startGeneration()
        plot_renderer.waitForFinished()

        res = plot_renderer.renderToImage(400, 400, 0, curve.length(), 0, 14)
        self.assertTrue(self.image_check('vector_polygon_layer_symbology', 'vector_polygon_layer_symbology', res))

    def doCheckPoint(self, request: QgsProfileRequest, tolerance: float, layer: QgsVectorLayer, expectedFeatures):
        request.setTolerance(tolerance)

        profGen = layer.createProfileGenerator(request)
        self.assertIsNotNone(profGen)
        self.assertTrue(profGen.generateProfile())
        results = profGen.takeResults()
        features = results.asFeatures(Qgis.ProfileExportType.Features3D)
        self.assertFalse(len(features) == 0)

        expected = sorted(expectedFeatures.copy())
        actual = [f.attributes['id'] for _, f in enumerate(features)]
        actualUniqSorted = sorted(list(set(actual)))

        self.assertEqual(actualUniqSorted, expected)

        for k, feat in enumerate(features):
            hasValidZ = False
            if QgsWkbTypes.hasZ(feat.geometry.wkbType()):
                for v in feat.geometry.vertices():
                    if not math.isnan(v.z()):
                        hasValidZ = True
                        break
                self.assertTrue(hasValidZ, "All vertice are on the ground!")
            else:
                self.assertTrue(hasValidZ, "Geometry should have z coordinates!")

        return results

    def doCheckLine(self, request: QgsProfileRequest, tolerance: float, layer: QgsVectorLayer, expectedFeatures, nbSubGeomPerFeature, geomType):
        results = self.doCheckPoint(request, tolerance, layer, expectedFeatures)
        features = results.asFeatures(Qgis.ProfileExportType.Features3D)

        actual = [f.attributes['id'] for _, f in enumerate(features)]
        actualUniqSorted = sorted(list(set(actual)))
        for idx, fid in enumerate(actualUniqSorted):
            actual = [1 for _, f in enumerate(features) if f.attributes['id'] == fid]
            self.assertEqual(len(actual), nbSubGeomPerFeature[idx])

        for k, feat in enumerate(features):
            self.assertEqual(feat.geometry.type(), geomType)

        for _, height in enumerate(results.distanceToHeightMap()):
            self.assertTrue(math.isnan(height) or height > 0.0)

        return results

    def testPointGenerationFeature(self):
        vl = QgsVectorLayer(os.path.join(unitTestDataPath(), '3d', 'points_with_z.shp'))
        self.assertTrue(vl.isValid())

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Terrain)
        vl.elevationProperties().setBinding(Qgis.AltitudeBinding.Vertex)

        dtmLayer = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'))
        self.assertTrue(dtmLayer.isValid())

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-346120 6631840, -346550 6632030, -346440 6632140, -347830 6632930)')
        req = QgsProfileRequest(curve)

        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(dtmLayer)

        req.setTerrainProvider(terrain_provider)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        if Qgis.geosVersionMajor() == 3 and Qgis.geosVersionMinor() <= 10:
            self.doCheckPoint(req, 15, vl, [5, 11, 12, 13, 14, 15, 18, 45, 46])
        elif Qgis.geosVersionMajor() == 3 and Qgis.geosVersionMinor() == 11:
            self.doCheckPoint(req, 16, vl, [5, 11, 12, 13, 14, 15, 18, 45, 46])
        elif Qgis.geosVersionMajor() == 3 and Qgis.geosVersionMinor() >= 12:
            self.doCheckPoint(req, 15, vl, [5, 11, 12, 13, 14, 15, 18, 45])

        self.doCheckPoint(req, 70, vl, [0, 3, 4, 5, 6, 7, 10, 11, 12, 13, 14, 15, 16, 17, 18, 38, 45, 46, 48])

    def testLineGenerationFeature(self):
        vl = QgsVectorLayer(os.path.join(unitTestDataPath(), '3d', 'lines.shp'))
        self.assertTrue(vl.isValid())

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Terrain)
        vl.elevationProperties().setBinding(Qgis.AltitudeBinding.Vertex)
        vl.elevationProperties().setExtrusionEnabled(False)

        dtmLayer = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'))
        self.assertTrue(dtmLayer.isValid())

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-346120 6631840, -346550 6632030, -346440 6632140, -347830 6632930)')
        req = QgsProfileRequest(curve)

        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(dtmLayer)

        req.setTerrainProvider(terrain_provider)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        # check no tolerance
        self.doCheckLine(req, 0, vl, [0, 2], [1, 5], Qgis.GeometryType.Point)

        # check increased tolerance, terrain, no extrusion
        self.doCheckLine(req, 1, vl, [0, 2], [1, 5], Qgis.GeometryType.Line)

        # check increased tolerance, terrain, no extrusion
        self.doCheckLine(req, 20, vl, [0, 2], [1, 3], Qgis.GeometryType.Line)

        # check increased tolerance, terrain, no extrusion
        self.doCheckLine(req, 50, vl, [1, 0, 2], [1, 1, 1], Qgis.GeometryType.Line)

        # check terrain + extrusion
        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Terrain)
        vl.elevationProperties().setExtrusionEnabled(True)
        vl.elevationProperties().setExtrusionHeight(17)
        self.doCheckLine(req, 50, vl, [1, 0, 2], [1, 1, 1], Qgis.GeometryType.Polygon)

        # check no terrain + no extrusion
        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Absolute)
        vl.elevationProperties().setExtrusionEnabled(False)
        vl.elevationProperties().setZOffset(5.0)
        self.doCheckLine(req, 50, vl, [1, 0, 2], [1, 1, 1], Qgis.GeometryType.Line)

    def testPolygonGenerationFeature(self):
        vl = QgsVectorLayer(os.path.join(unitTestDataPath(), '3d', 'buildings.shp'))
        self.assertTrue(vl.isValid())

        vl.elevationProperties().setClamping(Qgis.AltitudeClamping.Terrain)
        vl.elevationProperties().setBinding(Qgis.AltitudeBinding.Vertex)
        vl.elevationProperties().setExtrusionEnabled(False)

        dtmLayer = QgsRasterLayer(os.path.join(unitTestDataPath(), '3d', 'dtm.tif'))
        self.assertTrue(dtmLayer.isValid())

        curve = QgsLineString()
        curve.fromWkt(
            'LineString (-346120 6631840, -346550 6632030, -346440 6632140, -347830 6632930)')
        req = QgsProfileRequest(curve)

        terrain_provider = QgsRasterDemTerrainProvider()
        terrain_provider.setLayer(dtmLayer)

        req.setTerrainProvider(terrain_provider)
        req.setCrs(QgsCoordinateReferenceSystem('EPSG:3857'))

        self.doCheckLine(req, 1, vl, [168, 206, 210, 284, 306, 321], [1, 1, 1, 1, 1, 1], Qgis.GeometryType.Line)

        if Qgis.geosVersionMajor() == 3 and Qgis.geosVersionMinor() <= 10:
            self.doCheckLine(req, 10, vl, [168, 172, 206, 210, 231, 267, 275, 282, 284, 306, 307, 319, 321], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], Qgis.GeometryType.Line)
            self.doCheckLine(req, 11, vl, [168, 172, 206, 210, 231, 255, 267, 275, 282, 283, 284, 306, 307, 319, 321], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], Qgis.GeometryType.Line)
        elif Qgis.geosVersionMajor() == 3 and Qgis.geosVersionMinor() == 11:
            self.doCheckLine(req, 9, vl, [168, 172, 206, 210, 231, 267, 275, 282, 284, 306, 307, 319, 321], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], Qgis.GeometryType.Line)
            self.doCheckLine(req, 10, vl, [168, 172, 206, 210, 231, 267, 275, 282, 283, 284, 306, 307, 319, 321], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], Qgis.GeometryType.Line)
        elif Qgis.geosVersionMajor() == 3 and Qgis.geosVersionMinor() >= 12:
            self.doCheckLine(req, 10, vl, [168, 172, 206, 210, 231, 267, 275, 282, 283, 284, 306, 307, 319, 321], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], Qgis.GeometryType.Line)
            self.doCheckLine(req, 11, vl, [168, 172, 206, 210, 231, 237, 255, 267, 275, 282, 283, 284, 306, 307, 319, 321], [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1], Qgis.GeometryType.Line)


if __name__ == '__main__':
    unittest.main()
