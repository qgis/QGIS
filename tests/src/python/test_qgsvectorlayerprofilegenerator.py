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
    QgsGeometry,
    QgsCoordinateTransformContext,
    QgsProjUtils
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()


class TestQgsVectorLayerProfileGenerator(unittest.TestCase):

    @staticmethod
    def round_dict(val, places):
        return {round(k, places): round(val[k], places) for k in sorted(val.keys())}

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

        req.setTolerance(15)
        generator = vl.createProfileGenerator(req)
        self.assertTrue(generator.generateProfile())
        results = generator.takeResults()

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {175.6: 69.5, 31.2: 69.5, 1242.5: 55.2})
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {31.2: 67.2, 175.6: 65.8, 1242.5: 52.2})

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
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {31.2: 331.2, 175.6: 329.8, 1242.5: 264.0})

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
        else:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  ['LineStringZ (-347395 6632649.6 329.8, -347395 6632649.6 336.8)',
                                   'LineStringZ (-347533.4 6632692.2 331.3, -347533.4 6632692.2 338.3)',
                                   'LineStringZ (-346399.2 6632265.6 264, -346399.2 6632265.6 271)'])

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
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {675.2: 62.7, 1195.7: 53.0, 1223.1: 56.0, 1272.0: 58.2, 1339.4: 57.5, 1444.4: 52.2})

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
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {675.2: 80.5, 1195.7: 90.5, 1223.1: 87.4, 1272.0: 94.5, 1339.4: 98.0, 1444.4: 94.0})

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
        else:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  ['LineStringZ (-346549.8 6632363.9 87.4, -346549.8 6632363.9 94.4)',
                                   'LineStringZ (-346501.4 6632357.8 94.5, -346501.4 6632357.8 101.5)',
                                   'LineStringZ (-346434.5 6632349.2 98, -346434.5 6632349.2 105)',
                                   'LineStringZ (-346384.6 6632279.1 94, -346384.6 6632279.1 101)',
                                   'LineStringZ (-346577 6632367.4 90.5, -346577 6632367.4 97.5)',
                                   'LineStringZ (-347062.8 6632554.4 80.5, -347062.8 6632554.4 87.5)'])

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
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.8: 48.5, 1042.4: 48.5, 1049.5: 48.5, 1070.2: 48.5, 1073.1: 48.5, 1074.8: 48.5,
                              1078.9: 48.5, 1083.9: 48.5, 1091.1: 48.5, 1186.8: 52.3, 1189.8: 52.2, 1192.7: 52.2,
                              1199.2: 52.2, 1450.0: 54.5, 1455.6: 54.5, 1458.1: 54.5})

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
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.8: 53.5, 1042.4: 53.5, 1049.5: 53.5, 1070.2: 53.5, 1073.1: 53.5, 1074.8: 53.5,
                              1078.9: 56.0, 1083.9: 56.0, 1091.1: 56.0, 1186.8: 62.3, 1189.8: 62.3, 1192.7: 62.3,
                              1199.2: 62.2, 1450.0: 67.0, 1455.6: 67.0, 1458.1: 67.0})

        if QgsProjUtils.projVersionMajor() >= 8:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  ['LineStringZ (-346718.7 6632419.8 60.3, -346712 6632417.4 60.3)',
                                   'LineStringZ (-346719.3 6632420 60.3, -346718.7 6632419.8 60.2)',
                                   'LineStringZ (-346689.7 6632409.5 60.3, -346688.2 6632409 60.3)',
                                   'LineStringZ (-346692.5 6632410.5 60.3, -346689.7 6632409.5 60.3)',
                                   'LineStringZ (-346684.3 6632407.6 62, -346679.6 6632406 62)',
                                   'LineStringZ (-346679.6 6632406 62, -346672.8 6632403.6 62)',
                                   'LineStringZ (-346582.6 6632371.7 59.3, -346579.7 6632370.7 59.3)',
                                   'LineStringZ (-346579.7 6632370.7 59.3, -346577 6632369.7 59.2, -346570.8 6632367.9 59.3)',
                                   'LineStringZ (-346387.6 6632223.9 65.5, -346384.8 6632219 65.5)',
                                   'LineStringZ (-346384.8 6632219 65.5, -346383.5 6632216.9 65.5)'])
        else:
            self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                                  ['LineStringZ (-346718.7 6632419.8 53.5, -346712 6632417.4 53.5)',
                                   'LineStringZ (-346719.3 6632420 53.5, -346718.7 6632419.8 53.5)',
                                   'LineStringZ (-346689.7 6632409 53.5, -346688.2 6632409 53.5)',
                                   'LineStringZ (-346692.5 6632410.5 53.5, -346689.7 6632409.5 53.5)',
                                   'LineStringZ (-346684.3 6632407.6 56, -346679.6 6632406 56)',
                                   'LineStringZ (-346679.6 6632406 56, -346672.8 6632403.6 56)',
                                   'LineStringZ (-346582.6 6632371.7 62.3, -346579.7 6632370.7 62.3)',
                                   'LineStringZ (-346579.7 6632370.7 62.3, -346577 6632369.7 62.3, -346570.8 6632367.9 62.3)',
                                   'LineStringZ (-346387.6 6632223.9 67, -346384.8 6632219 67)',
                                   'LineStringZ (-346384.8 6632219 67, -346383.5 6632216.9 67)'])

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
        else:
            self.assertEqual(self.round_dict(results.distanceToHeightMap(), 1),
                             {1041.8: 53.5, 1042.4: 53.5, 1049.5: 53.5, 1070.2: 53.5, 1073.1: 53.5, 1074.8: 53.5,
                              1078.9: 56.0, 1083.9: 56.0, 1091.1: 56.0, 1186.8: 62.3, 1189.8: 62.3, 1192.7: 62.3,
                              1199.2: 62.2, 1450.0: 67.0, 1455.6: 67.0, 1458.1: 67.0})

        print([g.asWkt(1) for g in results.asGeometries()])
        self.assertCountEqual([g.asWkt(1) for g in results.asGeometries()],
                              [
                                  'PolygonZ ((-346718.7 6632419.8 60.3, -346712 6632417.4 60.3, -346712 6632417.4 67.3, -346718.7 6632419.8 67.3, -346718.7 6632419.8 60.3))',
                                  'PolygonZ ((-346719.3 6632420 60.3, -346718.7 6632419.8 60.2, -346718.7 6632419.8 67.3, -346719.3 6632420 67.3, -346719.3 6632420 60.3))',
                                  'PolygonZ ((-346689.7 6632409.5 60.3, -346688.2 6632409 60.3, -346688.2 6632409 67.3, -346689.7 6632409.5 67.3, -346689.7 6632409.5 60.3))',
                                  'PolygonZ ((-346692.5 6632410.5 60.3, -346689.7 6632409.5 60.3, -346689.7 6632409.5 67.3, -346692.5 6632410.5 67.3, -346692.5 6632410.5 60.3))',
                                  'PolygonZ ((-346684.3 6632407.6 62, -346679.6 6632406 62, -346679.6 6632406 69, -346684.3 6632407.6 69, -346684.3 6632407.6 62))',
                                  'PolygonZ ((-346679.6 6632406 62, -346672.8 6632403.6 62, -346672.8 6632403.6 69, -346679.6 6632406 69, -346679.6 6632406 62))',
                                  'PolygonZ ((-346582.6 6632371.7 59.3, -346579.7 6632370.7 59.3, -346579.7 6632370.7 66.3, -346582.6 6632371.7 66.3, -346582.6 6632371.7 59.3))',
                                  'PolygonZ ((-346579.7 6632370.7 59.3, -346577 6632369.7 59.2, -346570.8 6632367.9 59.3, -346570.8 6632367.9 66.3, -346577 6632369.7 66.3, -346579.7 6632370.7 66.3, -346579.7 6632370.7 59.3))',
                                  'PolygonZ ((-346387.6 6632223.9 65.5, -346384.8 6632219 65.5, -346384.8 6632219 72.5, -346387.6 6632223.9 72.5, -346387.6 6632223.9 65.5))',
                                  'PolygonZ ((-346384.8 6632219 65.5, -346383.5 6632216.9 65.5, -346383.5 6632216.9 72.5, -346384.8 6632219 72.5, -346384.8 6632219 65.5))'])


if __name__ == '__main__':
    unittest.main()
