# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPointCloudRgbRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import QDir, QSize
from qgis.PyQt.QtGui import QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsProviderRegistry,
    QgsPointCloudLayer,
    QgsPointCloudRgbRenderer,
    QgsReadWriteContext,
    QgsRenderContext,
    QgsPointCloudRenderContext,
    QgsVector3D,
    QgsMultiRenderChecker,
    QgsMapSettings,
    QgsRectangle,
    QgsContrastEnhancement,
    QgsUnitTypes,
    QgsMapUnitScale,
    QgsCoordinateReferenceSystem,
    QgsDoubleRange,
    QgsPointCloudRenderer,
    QgsMapClippingRegion,
    QgsGeometry
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudRgbRenderer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsPointCloudRgbRenderer Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testSetLayer(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        # test that a point cloud with RGB attributes is automatically assigned the RGB renderer by default
        self.assertIsInstance(layer.renderer(), QgsPointCloudRgbRenderer)

        # for this point cloud, we should default to 0-255 ranges (ie. no contrast enhancement)
        self.assertIsNone(layer.renderer().redContrastEnhancement())
        self.assertIsNone(layer.renderer().greenContrastEnhancement())
        self.assertIsNone(layer.renderer().blueContrastEnhancement())

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testSetLayer16(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb16/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        # test that a point cloud with RGB attributes is automatically assigned the RGB renderer by default
        self.assertIsInstance(layer.renderer(), QgsPointCloudRgbRenderer)

        # for this point cloud, we should default to 0-65024 ranges with contrast enhancement
        self.assertEqual(layer.renderer().redContrastEnhancement().minimumValue(), 0)
        self.assertEqual(layer.renderer().redContrastEnhancement().maximumValue(), 65535.0)
        self.assertEqual(layer.renderer().redContrastEnhancement().contrastEnhancementAlgorithm(),
                         QgsContrastEnhancement.StretchToMinimumMaximum)
        self.assertEqual(layer.renderer().greenContrastEnhancement().minimumValue(), 0)
        self.assertEqual(layer.renderer().greenContrastEnhancement().maximumValue(), 65535.0)
        self.assertEqual(layer.renderer().greenContrastEnhancement().contrastEnhancementAlgorithm(),
                         QgsContrastEnhancement.StretchToMinimumMaximum)
        self.assertEqual(layer.renderer().blueContrastEnhancement().minimumValue(), 0)
        self.assertEqual(layer.renderer().blueContrastEnhancement().maximumValue(), 65535.0)
        self.assertEqual(layer.renderer().blueContrastEnhancement().contrastEnhancementAlgorithm(),
                         QgsContrastEnhancement.StretchToMinimumMaximum)

    def testBasic(self):
        renderer = QgsPointCloudRgbRenderer()
        renderer.setBlueAttribute('b')
        self.assertEqual(renderer.blueAttribute(), 'b')
        renderer.setGreenAttribute('g')
        self.assertEqual(renderer.greenAttribute(), 'g')
        renderer.setRedAttribute('r')
        self.assertEqual(renderer.redAttribute(), 'r')

        redce = QgsContrastEnhancement()
        redce.setMinimumValue(100)
        redce.setMaximumValue(120)
        redce.setContrastEnhancementAlgorithm(QgsContrastEnhancement.StretchAndClipToMinimumMaximum)
        renderer.setRedContrastEnhancement(redce)

        greence = QgsContrastEnhancement()
        greence.setMinimumValue(130)
        greence.setMaximumValue(150)
        greence.setContrastEnhancementAlgorithm(QgsContrastEnhancement.StretchToMinimumMaximum)
        renderer.setGreenContrastEnhancement(greence)

        bluece = QgsContrastEnhancement()
        bluece.setMinimumValue(170)
        bluece.setMaximumValue(190)
        bluece.setContrastEnhancementAlgorithm(QgsContrastEnhancement.ClipToMinimumMaximum)
        renderer.setBlueContrastEnhancement(bluece)

        renderer.setMaximumScreenError(18)
        renderer.setMaximumScreenErrorUnit(QgsUnitTypes.RenderInches)
        renderer.setPointSize(13)
        renderer.setPointSizeUnit(QgsUnitTypes.RenderPoints)
        renderer.setPointSizeMapUnitScale(QgsMapUnitScale(1000, 2000))

        rr = renderer.clone()
        self.assertEqual(rr.maximumScreenError(), 18)
        self.assertEqual(rr.maximumScreenErrorUnit(), QgsUnitTypes.RenderInches)
        self.assertEqual(rr.pointSize(), 13)
        self.assertEqual(rr.pointSizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(rr.pointSizeMapUnitScale().minScale, 1000)
        self.assertEqual(rr.pointSizeMapUnitScale().maxScale, 2000)

        self.assertEqual(rr.blueAttribute(), 'b')
        self.assertEqual(rr.greenAttribute(), 'g')
        self.assertEqual(rr.redAttribute(), 'r')
        self.assertEqual(rr.redContrastEnhancement().minimumValue(), 100)
        self.assertEqual(rr.redContrastEnhancement().maximumValue(), 120)
        self.assertEqual(rr.redContrastEnhancement().contrastEnhancementAlgorithm(),
                         QgsContrastEnhancement.StretchAndClipToMinimumMaximum)
        self.assertEqual(rr.greenContrastEnhancement().minimumValue(), 130)
        self.assertEqual(rr.greenContrastEnhancement().maximumValue(), 150)
        self.assertEqual(rr.greenContrastEnhancement().contrastEnhancementAlgorithm(),
                         QgsContrastEnhancement.StretchToMinimumMaximum)
        self.assertEqual(rr.blueContrastEnhancement().minimumValue(), 170)
        self.assertEqual(rr.blueContrastEnhancement().maximumValue(), 190)
        self.assertEqual(rr.blueContrastEnhancement().contrastEnhancementAlgorithm(),
                         QgsContrastEnhancement.ClipToMinimumMaximum)

        doc = QDomDocument("testdoc")
        elem = renderer.save(doc, QgsReadWriteContext())

        r2 = QgsPointCloudRgbRenderer.create(elem, QgsReadWriteContext())
        self.assertEqual(r2.maximumScreenError(), 18)
        self.assertEqual(r2.maximumScreenErrorUnit(), QgsUnitTypes.RenderInches)
        self.assertEqual(r2.pointSize(), 13)
        self.assertEqual(r2.pointSizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(r2.pointSizeMapUnitScale().minScale, 1000)
        self.assertEqual(r2.pointSizeMapUnitScale().maxScale, 2000)

        self.assertEqual(r2.blueAttribute(), 'b')
        self.assertEqual(r2.greenAttribute(), 'g')
        self.assertEqual(r2.redAttribute(), 'r')
        self.assertEqual(r2.redContrastEnhancement().minimumValue(), 100)
        self.assertEqual(r2.redContrastEnhancement().maximumValue(), 120)
        self.assertEqual(r2.redContrastEnhancement().contrastEnhancementAlgorithm(),
                         QgsContrastEnhancement.StretchAndClipToMinimumMaximum)
        self.assertEqual(r2.greenContrastEnhancement().minimumValue(), 130)
        self.assertEqual(r2.greenContrastEnhancement().maximumValue(), 150)
        self.assertEqual(r2.greenContrastEnhancement().contrastEnhancementAlgorithm(),
                         QgsContrastEnhancement.StretchToMinimumMaximum)
        self.assertEqual(r2.blueContrastEnhancement().minimumValue(), 170)
        self.assertEqual(r2.blueContrastEnhancement().maximumValue(), 190)
        self.assertEqual(r2.blueContrastEnhancement().contrastEnhancementAlgorithm(),
                         QgsContrastEnhancement.ClipToMinimumMaximum)

    def testUsedAttributes(self):
        renderer = QgsPointCloudRgbRenderer()
        renderer.setBlueAttribute('b')
        renderer.setGreenAttribute('g')
        renderer.setRedAttribute('r')

        rc = QgsRenderContext()
        prc = QgsPointCloudRenderContext(rc, QgsVector3D(), QgsVector3D(), 1, 0)

        self.assertEqual(renderer.usedAttributes(prc), {'r', 'g', 'b'})

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRender(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_rgb_render')
        result = renderchecker.runTest('expected_rgb_render')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderCircles(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(3)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)
        layer.renderer().setPointSymbol(QgsPointCloudRenderer.Circle)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_rgb_circle_render')
        result = renderchecker.runTest('expected_rgb_circle_render')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderCrsTransform(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        mapsettings.setExtent(QgsRectangle(152.977434544, -26.663017454, 152.977424882, -26.663009624))
        mapsettings.setLayers([layer])
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_rgb_render_crs_transform')
        result = renderchecker.runTest('expected_rgb_render_crs_transform')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderWithContrast(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        redce = QgsContrastEnhancement()
        redce.setMinimumValue(100)
        redce.setMaximumValue(120)
        redce.setContrastEnhancementAlgorithm(QgsContrastEnhancement.StretchToMinimumMaximum)
        layer.renderer().setRedContrastEnhancement(redce)

        greence = QgsContrastEnhancement()
        greence.setMinimumValue(130)
        greence.setMaximumValue(150)
        greence.setContrastEnhancementAlgorithm(QgsContrastEnhancement.StretchToMinimumMaximum)
        layer.renderer().setGreenContrastEnhancement(greence)

        bluece = QgsContrastEnhancement()
        bluece.setMinimumValue(170)
        bluece.setMaximumValue(190)
        bluece.setContrastEnhancementAlgorithm(QgsContrastEnhancement.StretchToMinimumMaximum)
        layer.renderer().setBlueContrastEnhancement(bluece)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_rgb_contrast')
        result = renderchecker.runTest('expected_rgb_contrast')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderOpacity(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        layer.setOpacity(0.5)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_opacity')
        result = renderchecker.runTest('expected_opacity')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderBlendMode(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        layer.setBlendMode(QPainter.CompositionMode_ColorBurn)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_blendmode')
        result = renderchecker.runTest('expected_blendmode')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderPointSize(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(0.05)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMapUnits)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_pointsize')
        result = renderchecker.runTest('expected_pointsize')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderZRange(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])
        mapsettings.setZRange(QgsDoubleRange(1.1, 1.2))

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_zfilter')
        result = renderchecker.runTest('expected_zfilter')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderClipRegion(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        mapsettings.setExtent(QgsRectangle(152.977434544, -26.663017454, 152.977424882, -26.663009624))
        mapsettings.setLayers([layer])

        region = QgsMapClippingRegion(QgsGeometry.fromWkt(
            'Polygon ((152.97742833685992991 -26.66301088198133584, 152.97742694456141521 -26.66301085776744983, 152.97742676295726483 -26.66301358182974468, 152.97742895431403554 -26.66301349708113833, 152.97742833685992991 -26.66301088198133584))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt(
            'Polygon ((152.97743215054714483 -26.66301111201326535, 152.97742715037946937 -26.66301116044103736, 152.97742754990858316 -26.66301436878107367, 152.97743264693181686 -26.66301491359353193, 152.97743215054714483 -26.66301111201326535))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_clip_region')
        result = renderchecker.runTest('expected_clip_region')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderOrderedTopToBottom(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(6)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)
        layer.renderer().setDrawOrder2d(QgsPointCloudRenderer.DrawOrder.TopToBottom)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_rgb_top_to_bottom')
        result = renderchecker.runTest('expected_rgb_top_to_bottom')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderOrderedBottomToTop(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/rgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        layer.renderer().setPointSize(6)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)
        layer.renderer().setDrawOrder2d(QgsPointCloudRenderer.DrawOrder.BottomToTop)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(497753.5, 7050887.5, 497754.6, 7050888.6))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_rgb_bottom_to_top')
        result = renderchecker.runTest('expected_rgb_bottom_to_top')
        TestQgsPointCloudRgbRenderer.report += renderchecker.report()
        self.assertTrue(result)


if __name__ == '__main__':
    unittest.main()
