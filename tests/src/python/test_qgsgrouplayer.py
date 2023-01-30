# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsGroupLayer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '22/11/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import qgis  # NOQA

import tempfile
import os
import gc
from qgis.PyQt.QtCore import (
    QCoreApplication,
    QEvent,
    QSize,
    QDir
)
from qgis.PyQt.QtGui import (
    QPainter,
    QColor
)
from tempfile import TemporaryDirectory

from qgis.core import (
    QgsFeature,
    QgsVectorLayer,
    QgsRasterLayer,
    QgsProject,
    QgsCoordinateTransformContext,
    QgsGroupLayer,
    QgsGeometry,
    QgsPointXY,
    QgsMapSettings,
    QgsMultiRenderChecker,
    QgsDropShadowEffect,
    QgsEffectStack,
    QgsDrawSourceEffect,
    QgsColorEffect,
    QgsImageOperation
)
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsGroupLayer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsGroupLayer Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def test_children(self):
        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('test', options)
        self.assertTrue(group_layer.isValid())

        self.assertFalse(group_layer.childLayers())

        # add some child layers
        layer1 = QgsVectorLayer('Point?crs=epsg:3111', 'Point', 'memory')
        layer2 = QgsVectorLayer('Point?crs=epsg:4326', 'Point', 'memory')

        group_layer.setChildLayers([layer1, layer2])
        self.assertEqual(group_layer.childLayers(), [layer1, layer2])

        # force deletion of a layer
        layer1.deleteLater()
        layer1 = None
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
        # should be automatically cleaned
        self.assertEqual(group_layer.childLayers(), [layer2])

    def test_clone(self):
        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('test', options)
        self.assertTrue(group_layer.isValid())
        layer1 = QgsVectorLayer('Point?crs=epsg:3111', 'Point', 'memory')
        layer2 = QgsVectorLayer('Point?crs=epsg:4326', 'Point', 'memory')
        group_layer.setChildLayers([layer1, layer2])

        group_cloned = group_layer.clone()

        self.assertEqual(group_cloned.childLayers(), [layer1, layer2])

    def test_crs(self):
        # group layer should inherit first child layer crs
        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('test', options)
        self.assertTrue(group_layer.isValid())
        layer1 = QgsVectorLayer('Point?crs=epsg:3111', 'Point', 'memory')
        layer2 = QgsVectorLayer('Point?crs=epsg:4326', 'Point', 'memory')

        self.assertFalse(group_layer.crs().isValid())

        group_layer.setChildLayers([layer1, layer2])
        self.assertEqual(group_layer.crs().authid(), 'EPSG:3111')

        group_layer.setChildLayers([layer2, layer1])
        self.assertEqual(group_layer.crs().authid(), 'EPSG:4326')

    def test_extent(self):
        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('test', options)
        self.assertTrue(group_layer.isValid())
        layer1 = QgsVectorLayer('Point?crs=epsg:3111', 'Point', 'memory')
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(2478778, 2487236)))
        layer1.startEditing()
        layer1.addFeature(f)
        layer1.commitChanges()

        group_layer.setChildLayers([layer1])
        extent = group_layer.extent()
        self.assertAlmostEqual(extent.xMinimum(), 2478778, -2)
        self.assertAlmostEqual(extent.xMaximum(), 2478778, -2)
        self.assertAlmostEqual(extent.yMinimum(), 2487236, -2)
        self.assertAlmostEqual(extent.yMaximum(), 2487236, -2)

        layer2 = QgsVectorLayer('Point?crs=epsg:4326', 'Point', 'memory')
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(142.178, -35.943)))
        layer2.startEditing()
        layer2.addFeature(f)
        layer2.commitChanges()

        group_layer.setChildLayers([layer1, layer2])
        extent = group_layer.extent()
        self.assertAlmostEqual(extent.xMinimum(), 2245407, -2)
        self.assertEqual(extent.xMaximum(), 2478778)
        self.assertEqual(extent.yMinimum(), 2487236)
        self.assertAlmostEqual(extent.yMaximum(), 2613508, -2)

    def test_save_restore(self):
        """
        Test saving/restoring a project with a group layer
        """

        p = QgsProject()
        layer1 = QgsVectorLayer('Point?crs=epsg:3111', 'L1', 'memory')
        layer2 = QgsVectorLayer('Point?crs=epsg:4326', 'L2', 'memory')
        layer3 = QgsVectorLayer('Point?crs=epsg:3111', 'L3', 'memory')

        p.addMapLayer(layer1)
        p.addMapLayer(layer2)
        p.addMapLayer(layer3)

        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer1 = QgsGroupLayer('group 1', options)
        group_layer1.setChildLayers([layer1, layer2, layer3])
        group_layer2 = QgsGroupLayer('group 2', options)
        group_layer2.setChildLayers([layer3, layer1])

        drop_shadow = QgsDropShadowEffect()
        group_layer2.setPaintEffect(drop_shadow)

        p.addMapLayer(group_layer1, False)
        p.addMapLayer(group_layer2, False)

        with TemporaryDirectory() as d:
            path = os.path.join(d, 'group_layers.qgs')

            p.setFileName(path)
            p.write()

            p2 = QgsProject()
            p2.read(path)

            restored_layer1 = p2.mapLayersByName('L1')[0]
            restored_layer2 = p2.mapLayersByName('L2')[0]
            restored_layer3 = p2.mapLayersByName('L3')[0]
            restored_group_1 = p2.mapLayersByName('group 1')[0]
            restored_group_2 = p2.mapLayersByName('group 2')[0]

            self.assertEqual(restored_group_1.childLayers(), [restored_layer1, restored_layer2, restored_layer3])
            self.assertEqual(restored_group_2.childLayers(), [restored_layer3, restored_layer1])

            self.assertIsInstance(restored_group_2.paintEffect(), QgsDropShadowEffect)

    def test_render_group_opacity(self):
        """
        Test rendering layers as a group with opacity
        """
        vl1 = QgsVectorLayer(TEST_DATA_DIR + '/lines.shp')
        self.assertTrue(vl1.isValid())
        vl2 = QgsVectorLayer(TEST_DATA_DIR + '/points.shp')
        self.assertTrue(vl2.isValid())
        vl3 = QgsVectorLayer(TEST_DATA_DIR + '/polys.shp')
        self.assertTrue(vl3.isValid())

        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('group', options)
        group_layer.setChildLayers([vl1, vl2, vl3])
        # render group with 50% opacity
        group_layer.setOpacity(0.5)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(600, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(group_layer.crs())
        mapsettings.setExtent(group_layer.extent())
        mapsettings.setLayers([group_layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('group_layer')
        renderchecker.setControlName('expected_group_opacity')
        result = renderchecker.runTest('expected_group_opacity')
        TestQgsGroupLayer.report += renderchecker.report()
        self.assertTrue(result)

    def test_render_group_blend_mode(self):
        """
        Test rendering layers as a group limits child layer blend mode scope
        """
        vl1 = QgsVectorLayer(TEST_DATA_DIR + '/lines.shp')
        self.assertTrue(vl1.isValid())
        vl2 = QgsVectorLayer(TEST_DATA_DIR + '/points.shp')
        self.assertTrue(vl2.isValid())

        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('group', options)
        group_layer.setChildLayers([vl2, vl1])
        vl1.setBlendMode(QPainter.CompositionMode_DestinationIn)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(600, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(group_layer.crs())
        mapsettings.setExtent(group_layer.extent())
        mapsettings.setLayers([group_layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('group_layer')
        renderchecker.setControlName('expected_group_child_blend_mode')
        result = renderchecker.runTest('expected_group_child_blend_mode')
        TestQgsGroupLayer.report += renderchecker.report()
        self.assertTrue(result)

    def test_render_paint_effect(self):
        """
        Test rendering layers as a group with paint effect
        """
        vl1 = QgsVectorLayer(TEST_DATA_DIR + '/lines.shp')
        self.assertTrue(vl1.isValid())
        vl2 = QgsVectorLayer(TEST_DATA_DIR + '/points.shp')
        self.assertTrue(vl2.isValid())

        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('group', options)
        group_layer.setChildLayers([vl2, vl1])

        drop_shadow = QgsDropShadowEffect()
        drop_shadow.setBlurLevel(0)
        drop_shadow.setOpacity(1)
        drop_shadow.setColor(QColor(255, 0, 255))
        drop_shadow.setOffsetDistance(3)

        effect_stack = QgsEffectStack()
        effect_stack.appendEffect(drop_shadow)
        effect_stack.appendEffect(QgsDrawSourceEffect())
        group_layer.setPaintEffect(effect_stack)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(600, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(group_layer.crs())
        mapsettings.setExtent(group_layer.extent())
        mapsettings.setLayers([group_layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('group_layer')
        renderchecker.setControlName('expected_group_paint_effect')
        result = renderchecker.runTest('expected_group_paint_effect')
        TestQgsGroupLayer.report += renderchecker.report()
        self.assertTrue(result)

    def test_render_magnification(self):
        """
        Test rendering layers with magnification
        """
        vl1 = QgsVectorLayer(TEST_DATA_DIR + '/points.shp')
        self.assertTrue(vl1.isValid())
        rl1 = QgsRasterLayer(TEST_DATA_DIR + '/raster_layer.tiff')
        self.assertTrue(rl1.isValid())

        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('group', options)
        group_layer.setChildLayers([rl1, vl1])

        color_effect = QgsColorEffect()
        color_effect.setGrayscaleMode(QgsImageOperation.GrayscaleAverage)

        effect_stack = QgsEffectStack()
        effect_stack.appendEffect(color_effect)
        group_layer.setPaintEffect(effect_stack)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(600, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDpiTarget(192)
        mapsettings.setMagnificationFactor(2)
        mapsettings.setDestinationCrs(group_layer.crs())
        mapsettings.setExtent(group_layer.extent())
        mapsettings.setLayers([group_layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('group_layer')
        renderchecker.setControlName('expected_group_magnification')
        result = renderchecker.runTest('expected_group_magnification')
        TestQgsGroupLayer.report += renderchecker.report()
        self.assertTrue(result)


if __name__ == '__main__':
    unittest.main()
