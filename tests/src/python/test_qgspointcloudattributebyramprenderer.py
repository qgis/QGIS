# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPointCloudAttributeByRampRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    QgsProviderRegistry,
    QgsPointCloudLayer,
    QgsPointCloudAttributeByRampRenderer,
    QgsPointCloudRenderer,
    QgsReadWriteContext,
    QgsRenderContext,
    QgsPointCloudRenderContext,
    QgsVector3D,
    QgsMultiRenderChecker,
    QgsMapSettings,
    QgsRectangle,
    QgsUnitTypes,
    QgsMapUnitScale,
    QgsCoordinateReferenceSystem,
    QgsDoubleRange,
    QgsColorRampShader,
    QgsStyle,
    QgsLayerTreeLayer,
    QgsColorRampLegendNode,
    QgsSimpleLegendNode
)

from qgis.PyQt.QtCore import QDir, QSize, Qt
from qgis.PyQt.QtGui import QPainter
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudAttributeByRampRenderer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsPointCloudAttributeByRampRenderer Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testSetLayer(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/norgb/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        # test that a point cloud with no RGB attributes is automatically assigned the ramp renderer
        self.assertIsInstance(layer.renderer(), QgsPointCloudAttributeByRampRenderer)

        # check default range
        self.assertAlmostEqual(layer.renderer().minimum(), -1.98, 6)
        self.assertAlmostEqual(layer.renderer().maximum(), -1.92, 6)

    def testBasic(self):
        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('attr')
        self.assertEqual(renderer.attribute(), 'attr')
        renderer.setMinimum(5)
        self.assertEqual(renderer.minimum(), 5)
        renderer.setMaximum(15)
        self.assertEqual(renderer.maximum(), 15)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(20, 30, ramp)
        renderer.setColorRampShader(shader)
        self.assertEqual(renderer.colorRampShader().minimumValue(), 20)
        self.assertEqual(renderer.colorRampShader().maximumValue(), 30)

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

        self.assertEqual(rr.attribute(), 'attr')
        self.assertEqual(rr.minimum(), 5)
        self.assertEqual(rr.maximum(), 15)
        self.assertEqual(rr.colorRampShader().minimumValue(), 20)
        self.assertEqual(rr.colorRampShader().maximumValue(), 30)
        self.assertEqual(rr.colorRampShader().sourceColorRamp().color1().name(),
                         renderer.colorRampShader().sourceColorRamp().color1().name())
        self.assertEqual(rr.colorRampShader().sourceColorRamp().color2().name(),
                         renderer.colorRampShader().sourceColorRamp().color2().name())

        doc = QDomDocument("testdoc")
        elem = renderer.save(doc, QgsReadWriteContext())

        r2 = QgsPointCloudAttributeByRampRenderer.create(elem, QgsReadWriteContext())
        self.assertEqual(r2.maximumScreenError(), 18)
        self.assertEqual(r2.maximumScreenErrorUnit(), QgsUnitTypes.RenderInches)
        self.assertEqual(r2.pointSize(), 13)
        self.assertEqual(r2.pointSizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(r2.pointSizeMapUnitScale().minScale, 1000)
        self.assertEqual(r2.pointSizeMapUnitScale().maxScale, 2000)
        self.assertEqual(r2.attribute(), 'attr')
        self.assertEqual(r2.minimum(), 5)
        self.assertEqual(r2.maximum(), 15)
        self.assertEqual(r2.colorRampShader().minimumValue(), 20)
        self.assertEqual(r2.colorRampShader().maximumValue(), 30)
        self.assertEqual(r2.colorRampShader().sourceColorRamp().color1().name(),
                         renderer.colorRampShader().sourceColorRamp().color1().name())
        self.assertEqual(r2.colorRampShader().sourceColorRamp().color2().name(),
                         renderer.colorRampShader().sourceColorRamp().color2().name())

    def testUsedAttributes(self):
        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('attr')

        rc = QgsRenderContext()
        prc = QgsPointCloudRenderContext(rc, QgsVector3D(), QgsVector3D(), 1, 0)

        self.assertEqual(renderer.usedAttributes(prc), {'attr'})

    def testLegend(self):
        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('Intensity')
        renderer.setMinimum(200)
        renderer.setMaximum(800)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(200, 800, ramp.clone())
        shader.setClassificationMode(QgsColorRampShader.EqualInterval)
        shader.classifyColorRamp(classes=4)
        renderer.setColorRampShader(shader)

        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        layer_tree_layer = QgsLayerTreeLayer(layer)
        nodes = renderer.createLegendNodes(layer_tree_layer)
        self.assertEqual(len(nodes), 2)
        self.assertIsInstance(nodes[0], QgsSimpleLegendNode)
        self.assertEqual(nodes[0].data(Qt.DisplayRole), 'Intensity')
        self.assertIsInstance(nodes[1], QgsColorRampLegendNode)
        self.assertEqual(nodes[1].ramp().color1().name(), '#440154')
        self.assertEqual(nodes[1].ramp().color2().name(), '#fde725')

        shader = QgsColorRampShader(200, 600, ramp.clone())
        shader.setClassificationMode(QgsColorRampShader.EqualInterval)
        shader.setColorRampType(QgsColorRampShader.Exact)
        shader.classifyColorRamp(classes=2)
        renderer.setColorRampShader(shader)
        nodes = renderer.createLegendNodes(layer_tree_layer)
        self.assertEqual(len(nodes), 3)
        self.assertEqual(nodes[0].data(Qt.DisplayRole), 'Intensity')
        self.assertEqual(nodes[1].data(Qt.DisplayRole), '200')
        self.assertEqual(nodes[2].data(Qt.DisplayRole), '600')

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRender(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('Intensity')
        renderer.setMinimum(200)
        renderer.setMaximum(1000)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(200, 1000, ramp)
        shader.classifyColorRamp()
        renderer.setColorRampShader(shader)

        layer.setRenderer(renderer)

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_ramp_render')
        result = renderchecker.runTest('expected_ramp_render')
        TestQgsPointCloudAttributeByRampRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderX(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('X')
        renderer.setMinimum(498062.00000)
        renderer.setMaximum(498067.39000)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(498062.00000, 498067.39000, ramp)
        shader.classifyColorRamp()
        renderer.setColorRampShader(shader)

        layer.setRenderer(renderer)

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_ramp_xrender')
        result = renderchecker.runTest('expected_ramp_xrender')
        TestQgsPointCloudAttributeByRampRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderY(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('Y')
        renderer.setMinimum(7050992.84000)
        renderer.setMaximum(7050997.04000)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(7050992.84000, 7050997.04000, ramp)
        shader.classifyColorRamp()
        renderer.setColorRampShader(shader)

        layer.setRenderer(renderer)

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_ramp_yrender')
        result = renderchecker.runTest('expected_ramp_yrender')
        TestQgsPointCloudAttributeByRampRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderZ(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('Z')
        renderer.setMinimum(74.34000)
        renderer.setMaximum(75)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(74.34000, 75, ramp)
        shader.classifyColorRamp()
        renderer.setColorRampShader(shader)

        layer.setRenderer(renderer)

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_ramp_zrender')
        result = renderchecker.runTest('expected_ramp_zrender')
        TestQgsPointCloudAttributeByRampRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderCrsTransform(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('Intensity')
        renderer.setMinimum(200)
        renderer.setMaximum(1000)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(200, 1000, ramp)
        shader.classifyColorRamp()
        renderer.setColorRampShader(shader)

        layer.setRenderer(renderer)
        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        mapsettings.setExtent(QgsRectangle(152.980508492, -26.662023491, 152.980586020, -26.662071137))
        mapsettings.setLayers([layer])
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_ramp_render_crs_transform')
        result = renderchecker.runTest('expected_ramp_render_crs_transform')
        TestQgsPointCloudAttributeByRampRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderPointSize(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('Intensity')
        renderer.setMinimum(200)
        renderer.setMaximum(1000)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(200, 1000, ramp)
        shader.classifyColorRamp()
        renderer.setColorRampShader(shader)

        layer.setRenderer(renderer)
        layer.renderer().setPointSize(.15)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMapUnits)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_ramp_pointsize')
        result = renderchecker.runTest('expected_ramp_pointsize')
        TestQgsPointCloudAttributeByRampRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderZRange(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('Intensity')
        renderer.setMinimum(200)
        renderer.setMaximum(1000)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(200, 1000, ramp)
        shader.classifyColorRamp()
        renderer.setColorRampShader(shader)

        layer.setRenderer(renderer)
        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])
        mapsettings.setZRange(QgsDoubleRange(74.7, 75))

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_ramp_zfilter')
        result = renderchecker.runTest('expected_ramp_zfilter')
        TestQgsPointCloudAttributeByRampRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderTopToBottom(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('Intensity')
        renderer.setMinimum(200)
        renderer.setMaximum(1000)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(200, 1000, ramp)
        shader.classifyColorRamp()
        renderer.setColorRampShader(shader)

        layer.setRenderer(renderer)

        layer.renderer().setPointSize(6)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)
        layer.renderer().setDrawOrder2d(QgsPointCloudRenderer.DrawOrder.TopToBottom)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_ramp_top_to_bottom')
        result = renderchecker.runTest('expected_ramp_top_to_bottom')
        TestQgsPointCloudAttributeByRampRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderBottomToTop(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudAttributeByRampRenderer()
        renderer.setAttribute('Intensity')
        renderer.setMinimum(200)
        renderer.setMaximum(1000)
        ramp = QgsStyle.defaultStyle().colorRamp("Viridis")
        shader = QgsColorRampShader(200, 1000, ramp)
        shader.classifyColorRamp()
        renderer.setColorRampShader(shader)

        layer.setRenderer(renderer)

        layer.renderer().setPointSize(6)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)
        layer.renderer().setDrawOrder2d(QgsPointCloudRenderer.DrawOrder.BottomToTop)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_ramp_bottom_to_top')
        result = renderchecker.runTest('expected_ramp_bottom_to_top')
        TestQgsPointCloudAttributeByRampRenderer.report += renderchecker.report()
        self.assertTrue(result)


if __name__ == '__main__':
    unittest.main()
