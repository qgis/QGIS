# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapClippingUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2020-06'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.testing import unittest
from qgis.core import (
    QgsMapClippingRegion,
    QgsMapClippingUtils,
    QgsMapSettings,
    QgsRenderContext,
    QgsGeometry,
    QgsVectorLayer,
    QgsCoordinateTransform,
    QgsCoordinateReferenceSystem,
    QgsProject,
    QgsMapToPixel,
    QgsMapLayerType
)

from qgis.testing import start_app

start_app()


class TestQgsMapClippingUtils(unittest.TestCase):

    def testClippingRegionsForLayer(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                "addfeat", "memory")

        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 1 0, 1 1, 0 1, 0 0))'))
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 0.1 0, 0.1 2, 0 2, 0 0))'))
        region2.setRestrictedLayers([layer])
        region2.setRestrictToLayers(True)
        ms = QgsMapSettings()
        ms.addClippingRegion(region)
        ms.addClippingRegion(region2)
        rc = QgsRenderContext.fromMapSettings(ms)

        regions = QgsMapClippingUtils.collectClippingRegionsForLayer(rc, layer)
        self.assertEqual(len(regions), 2)
        self.assertEqual(regions[0].geometry().asWkt(1), 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')
        self.assertEqual(regions[1].geometry().asWkt(1), 'Polygon ((0 0, 0.1 0, 0.1 2, 0 2, 0 0))')

        regions = QgsMapClippingUtils.collectClippingRegionsForLayer(rc, layer2)
        self.assertEqual(len(regions), 1)
        self.assertEqual(regions[0].geometry().asWkt(1), 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')

    def testCalculateFeatureRequestGeometry(self):
        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 1 0, 1 1, 0 1, 0 0))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 0.1 0, 0.1 2, 0 2, 0 0))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.NoClipping)

        rc = QgsRenderContext()

        geom, should_clip = QgsMapClippingUtils.calculateFeatureRequestGeometry([], rc)
        self.assertFalse(should_clip)
        self.assertTrue(geom.isNull())

        geom, should_clip = QgsMapClippingUtils.calculateFeatureRequestGeometry([region], rc)
        geom.normalize()
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(1), 'Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))')

        geom, should_clip = QgsMapClippingUtils.calculateFeatureRequestGeometry([region, region2], rc)
        geom.normalize()
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(1), 'Polygon ((0 0, 0 1, 0.1 1, 0.1 0, 0 0))')

        rc.setCoordinateTransform(QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:3857'), QgsCoordinateReferenceSystem('EPSG:4326'), QgsProject.instance()))
        geom, should_clip = QgsMapClippingUtils.calculateFeatureRequestGeometry([region, region2], rc)
        geom.normalize()
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(0), 'Polygon ((0 0, 0 111325, 11132 111325, 11132 0, 0 0))')

    def testCalculateFeatureIntersectionGeometry(self):
        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 1 0, 1 1, 0 1, 0 0))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 0.1 0, 0.1 2, 0 2, 0 0))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        region3 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 0.1 0, 0.1 2, 0 2, 0 0))'))
        region3.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)

        rc = QgsRenderContext()

        geom, should_clip = QgsMapClippingUtils.calculateFeatureIntersectionGeometry([], rc)
        self.assertFalse(should_clip)
        self.assertTrue(geom.isNull())

        geom, should_clip = QgsMapClippingUtils.calculateFeatureIntersectionGeometry([region], rc)
        geom.normalize()
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(1), 'Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))')

        # region2 is a Intersects type clipping region, should not apply here
        geom, should_clip = QgsMapClippingUtils.calculateFeatureIntersectionGeometry([region2], rc)
        self.assertFalse(should_clip)
        self.assertTrue(geom.isNull())

        geom, should_clip = QgsMapClippingUtils.calculateFeatureIntersectionGeometry([region, region2], rc)
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(1), 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')

        geom, should_clip = QgsMapClippingUtils.calculateFeatureIntersectionGeometry([region, region2, region3], rc)
        geom.normalize()
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(1), 'Polygon ((0 0, 0 1, 0.1 1, 0.1 0, 0 0))')

        rc.setCoordinateTransform(QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:3857'), QgsCoordinateReferenceSystem('EPSG:4326'), QgsProject.instance()))
        geom, should_clip = QgsMapClippingUtils.calculateFeatureIntersectionGeometry([region, region3], rc)
        geom.normalize()
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(0), 'Polygon ((0 0, 0 111325, 11132 111325, 11132 0, 0 0))')

    def testPainterClipPath(self):
        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 1 0, 1 1, 0 1, 0 0))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 0.1 0, 0.1 2, 0 2, 0 0))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        region3 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 0.1 0, 0.1 2, 0 2, 0 0))'))
        region3.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)

        rc = QgsRenderContext()

        for t in [QgsMapLayerType.VectorLayer, QgsMapLayerType.RasterLayer, QgsMapLayerType.MeshLayer, QgsMapLayerType.VectorTileLayer]:
            path, should_clip = QgsMapClippingUtils.calculatePainterClipRegion([], rc, t)
            self.assertFalse(should_clip)
            self.assertEqual(path.elementCount(), 0)

        for t in [QgsMapLayerType.VectorLayer, QgsMapLayerType.RasterLayer, QgsMapLayerType.MeshLayer, QgsMapLayerType.VectorTileLayer]:
            path, should_clip = QgsMapClippingUtils.calculatePainterClipRegion([region], rc, t)
            self.assertTrue(should_clip)
            self.assertEqual(QgsGeometry.fromQPolygonF(path.toFillPolygon()).asWkt(1), 'Polygon ((0 1, 1 1, 1 0, 0 0, 0 1))')

        # region2 is a Intersects type clipping region, should not apply for vector layers
        path, should_clip = QgsMapClippingUtils.calculatePainterClipRegion([region2], rc, QgsMapLayerType.VectorLayer)
        self.assertFalse(should_clip)
        self.assertEqual(path.elementCount(), 0)

        for t in [QgsMapLayerType.RasterLayer, QgsMapLayerType.MeshLayer, QgsMapLayerType.VectorTileLayer]:
            path, should_clip = QgsMapClippingUtils.calculatePainterClipRegion([region2], rc, t)
            self.assertTrue(should_clip)
            self.assertEqual(QgsGeometry.fromQPolygonF(path.toFillPolygon()).asWkt(1), 'Polygon ((0 1, 0.1 1, 0.1 -1, 0 -1, 0 1))')

        for t in [QgsMapLayerType.VectorLayer, QgsMapLayerType.RasterLayer, QgsMapLayerType.MeshLayer, QgsMapLayerType.VectorTileLayer]:
            path, should_clip = QgsMapClippingUtils.calculatePainterClipRegion([region, region2, region3], rc, t)
            self.assertTrue(should_clip)
            geom = QgsGeometry.fromQPolygonF(path.toFillPolygon())
            geom.normalize()
            self.assertEqual(geom.asWkt(1), 'Polygon ((0 0, 0 1, 0.1 1, 0.1 0, 0 0))')

        rc.setMapToPixel(QgsMapToPixel(5, 10, 11, 200, 150, 0))
        for t in [QgsMapLayerType.VectorLayer, QgsMapLayerType.RasterLayer, QgsMapLayerType.MeshLayer, QgsMapLayerType.VectorTileLayer]:
            path, should_clip = QgsMapClippingUtils.calculatePainterClipRegion([region, region3], rc, t)
            self.assertTrue(should_clip)
            self.assertEqual(QgsGeometry.fromQPolygonF(path.toFillPolygon()).asWkt(0), 'Polygon ((98 77, 98 77, 98 77, 98 77, 98 77))')

    def testLabelIntersectionGeometry(self):
        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 1 0, 1 1, 0 1, 0 0))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 0.1 0, 0.1 2, 0 2, 0 0))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        region3 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon((0 0, 0.1 0, 0.1 2, 0 2, 0 0))'))
        region3.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)

        rc = QgsRenderContext()

        geom, should_clip = QgsMapClippingUtils.calculateLabelIntersectionGeometry([], rc)
        self.assertFalse(should_clip)
        self.assertTrue(geom.isNull())

        geom, should_clip = QgsMapClippingUtils.calculateLabelIntersectionGeometry([region], rc)
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(1), 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')

        # region2 is a Intersects type clipping region, should not apply here
        geom, should_clip = QgsMapClippingUtils.calculateLabelIntersectionGeometry([region2], rc)
        self.assertFalse(should_clip)
        self.assertTrue(geom.isNull())

        geom, should_clip = QgsMapClippingUtils.calculateLabelIntersectionGeometry([region, region2], rc)
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(1), 'Polygon ((0 0, 1 0, 1 1, 0 1, 0 0))')

        # region3 is a PainterClip type clipping region, MUST be applied for labels
        geom, should_clip = QgsMapClippingUtils.calculateLabelIntersectionGeometry([region, region2, region3], rc)
        geom.normalize()
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(1), 'Polygon ((0 0, 0 1, 0.1 1, 0.1 0, 0 0))')

        rc.setCoordinateTransform(
            QgsCoordinateTransform(QgsCoordinateReferenceSystem('EPSG:3857'), QgsCoordinateReferenceSystem('EPSG:4326'),
                                   QgsProject.instance()))
        geom, should_clip = QgsMapClippingUtils.calculateLabelIntersectionGeometry([region, region3], rc)
        geom.normalize()
        self.assertTrue(should_clip)
        self.assertEqual(geom.asWkt(0), 'Polygon ((0 0, 0 111325, 11132 111325, 11132 0, 0 0))')


if __name__ == '__main__':
    unittest.main()
