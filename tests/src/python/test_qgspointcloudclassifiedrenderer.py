# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPointCloudClassifiedRenderer

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
    QgsPointCloudClassifiedRenderer,
    QgsPointCloudCategory,
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
    QgsLayerTreeModelLegendNode
)

from qgis.PyQt.QtCore import QDir, QSize, Qt
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudClassifiedRenderer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsPointCloudClassifiedRenderer Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def testBasic(self):
        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute('attr')
        self.assertEqual(renderer.attribute(), 'attr')

        renderer.setCategories([QgsPointCloudCategory(3, QColor(255, 0, 0), 'cat 3'),
                                QgsPointCloudCategory(7, QColor(0, 255, 0), 'cat 7')])

        self.assertEqual(len(renderer.categories()), 2)
        self.assertEqual(renderer.categories()[0].label(), 'cat 3')
        self.assertEqual(renderer.categories()[1].label(), 'cat 7')

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
        self.assertEqual(len(rr.categories()), 2)
        self.assertEqual(rr.categories()[0].label(), 'cat 3')
        self.assertEqual(rr.categories()[1].label(), 'cat 7')

        doc = QDomDocument("testdoc")
        elem = renderer.save(doc, QgsReadWriteContext())

        r2 = QgsPointCloudClassifiedRenderer.create(elem, QgsReadWriteContext())
        self.assertEqual(r2.maximumScreenError(), 18)
        self.assertEqual(r2.maximumScreenErrorUnit(), QgsUnitTypes.RenderInches)
        self.assertEqual(r2.pointSize(), 13)
        self.assertEqual(r2.pointSizeUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(r2.pointSizeMapUnitScale().minScale, 1000)
        self.assertEqual(r2.pointSizeMapUnitScale().maxScale, 2000)
        self.assertEqual(r2.attribute(), 'attr')
        self.assertEqual(len(r2.categories()), 2)
        self.assertEqual(r2.categories()[0].label(), 'cat 3')
        self.assertEqual(r2.categories()[1].label(), 'cat 7')

    def testUsedAttributes(self):
        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute('attr')

        rc = QgsRenderContext()
        prc = QgsPointCloudRenderContext(rc, QgsVector3D(), QgsVector3D(), 1, 0)

        self.assertEqual(renderer.usedAttributes(prc), {'attr'})

    def testLegend(self):
        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute('Classification')
        renderer.setCategories([QgsPointCloudCategory(3, QColor(255, 0, 0), 'cat 3'),
                                QgsPointCloudCategory(7, QColor(0, 255, 0), 'cat 7')])

        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        layer_tree_layer = QgsLayerTreeLayer(layer)
        nodes = renderer.createLegendNodes(layer_tree_layer)
        self.assertEqual(len(nodes), 2)
        self.assertEqual(nodes[0].data(Qt.DisplayRole), 'cat 3')
        self.assertEqual(nodes[0].data(QgsLayerTreeModelLegendNode.RuleKeyRole), '3')
        self.assertEqual(nodes[1].data(Qt.DisplayRole), 'cat 7')
        self.assertEqual(nodes[1].data(QgsLayerTreeModelLegendNode.RuleKeyRole), '7')

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRender(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute('Classification')
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
        renderchecker.setControlName('expected_classified_render')
        result = renderchecker.runTest('expected_classified_render')
        TestQgsPointCloudClassifiedRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderCrsTransform(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute('Classification')
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
        renderchecker.setControlName('expected_classified_render_crs_transform')
        result = renderchecker.runTest('expected_classified_render_crs_transform')
        TestQgsPointCloudClassifiedRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderPointSize(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute('Classification')
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
        renderchecker.setControlName('expected_classified_pointsize')
        result = renderchecker.runTest('expected_classified_pointsize')
        TestQgsPointCloudClassifiedRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderZRange(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute('Classification')
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
        renderchecker.setControlName('expected_classified_zfilter')
        result = renderchecker.runTest('expected_classified_zfilter')
        TestQgsPointCloudClassifiedRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderOrderedTopToBottom(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute('Classification')
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
        renderchecker.setControlName('expected_classified_top_to_bottom')
        result = renderchecker.runTest('expected_classified_top_to_bottom')
        TestQgsPointCloudClassifiedRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderOrderedBottomToTop(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute('Classification')
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
        renderchecker.setControlName('expected_classified_bottom_to_top')
        result = renderchecker.runTest('expected_classified_bottom_to_top')
        TestQgsPointCloudClassifiedRenderer.report += renderchecker.report()
        self.assertTrue(result)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testRenderFiltered(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        renderer = QgsPointCloudClassifiedRenderer()
        renderer.setAttribute('Classification')
        layer.setRenderer(renderer)

        layer.renderer().setPointSize(2)
        layer.renderer().setPointSizeUnit(QgsUnitTypes.RenderMillimeters)
        layer.setSubsetString('NumberOfReturns > 1')

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setExtent(QgsRectangle(498061, 7050991, 498069, 7050999))
        mapsettings.setLayers([layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_classified_render_filtered')
        result = renderchecker.runTest('expected_classified_render_filtered')
        TestQgsPointCloudClassifiedRenderer.report += renderchecker.report()
        self.assertTrue(result)

        layer.setSubsetString('')
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('pointcloudrenderer')
        renderchecker.setControlName('expected_classified_render_unfiltered')
        result = renderchecker.runTest('expected_classified_render_unfiltered')
        TestQgsPointCloudClassifiedRenderer.report += renderchecker.report()
        self.assertTrue(result)


if __name__ == '__main__':
    unittest.main()
