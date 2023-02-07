"""QGIS Unit tests for QgsVectorLayerRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2020-06'
__copyright__ = 'Copyright 2020, The QGIS Project'

import os

import qgis  # NOQA
from qgis.PyQt.QtCore import QSize, QDir
from qgis.core import (QgsVectorLayer,
                       QgsMapClippingRegion,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsGeometry,
                       QgsSingleSymbolRenderer,
                       QgsMapSettings,
                       QgsFillSymbol,
                       QgsCoordinateReferenceSystem,
                       QgsRuleBasedRenderer,
                       QgsFeatureRendererGenerator,
                       QgsCategorizedSymbolRenderer,
                       QgsRendererCategory,
                       QgsCentroidFillSymbolLayer,
                       QgsMarkerSymbol,
                       QgsLineSymbol
                       )
from qgis.testing import start_app, unittest

from utilities import (unitTestDataPath)

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsVectorLayerRenderer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsVectorLayerRenderer Tests</h1>\n"

    def tearDown(self):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testRenderWithIntersectsRegions(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys.shp'))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_color': '#000000', 'outline_width': '1'})
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        mapsettings.setExtent(QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5))
        mapsettings.setLayers([poly_layer])

        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon ((-11725957 5368254, -12222900 4807501, -12246014 3834025, -12014878 3496059, -11259833 3518307, -10751333 3621153, -10574129 4516741, -10847640 5194995, -11105742 5325957, -11725957 5368254))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon ((-11032549 5421399, -11533344 4693167, -11086481 4229112, -11167378 3742984, -10616504 3553984, -10161936 3925771, -9618766 4668482, -9472380 5620753, -10115709 5965063, -11032549 5421399))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_intersects_region')
        result = renderchecker.runTest('expected_intersects_region')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_intersects_region')
        result = renderchecker.runTest('expected_intersects_region')
        self.report += renderchecker.report()
        self.assertTrue(result)

    def testRenderWithIntersectionRegions(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys.shp'))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_color': '#000000', 'outline_width': '1'})
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        mapsettings.setExtent(QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5))
        mapsettings.setLayers([poly_layer])

        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon ((-11725957 5368254, -12222900 4807501, -12246014 3834025, -12014878 3496059, -11259833 3518307, -10751333 3621153, -10574129 4516741, -10847640 5194995, -11105742 5325957, -11725957 5368254))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon ((-11032549 5421399, -11533344 4693167, -11086481 4229112, -11167378 3742984, -10616504 3553984, -10161936 3925771, -9618766 4668482, -9472380 5620753, -10115709 5965063, -11032549 5421399))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_intersection_region')
        result = renderchecker.runTest('expected_intersection_region')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_intersection_region')
        result = renderchecker.runTest('expected_intersection_region')
        self.report += renderchecker.report()
        self.assertTrue(result)

    def testIntersectionRuleBased(self):
        """
        Test that rule based renderer using intersection clip paths correctly uses original feature area for rule
        evaluation, not clipped area
        """
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys.shp'))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_color': '#000000', 'outline_width': '1'})
        sym2 = QgsFillSymbol.createSimple({'color': '#00ffff', 'outline_color': '#000000', 'outline_width': '1'})

        r1 = QgsRuleBasedRenderer.Rule(sym1, 0, 0, 'area($geometry)>25')
        r2 = QgsRuleBasedRenderer.Rule(sym2, 0, 0, 'ELSE')

        rootrule = QgsRuleBasedRenderer.Rule(None)
        rootrule.appendChild(r1)
        rootrule.appendChild(r2)
        renderer = QgsRuleBasedRenderer(rootrule)
        poly_layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        mapsettings.setExtent(QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5))
        mapsettings.setLayers([poly_layer])
        mapsettings.setEllipsoid('')

        region = QgsMapClippingRegion(QgsGeometry.fromWkt(
            'Polygon ((-11725957 5368254, -12222900 4807501, -12246014 3834025, -12014878 3496059, -11259833 3518307, -10751333 3621153, -10574129 4516741, -10847640 5194995, -11105742 5325957, -11725957 5368254))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt(
            'Polygon ((-11032549 5421399, -11533344 4693167, -11086481 4229112, -11167378 3742984, -10616504 3553984, -10161936 3925771, -9618766 4668482, -9472380 5620753, -10115709 5965063, -11032549 5421399))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipToIntersection)
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_intersection_rule_based')
        result = renderchecker.runTest('expected_intersection_rule_based')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_intersection_rule_based')
        result = renderchecker.runTest('expected_intersection_rule_based')
        self.report += renderchecker.report()
        self.assertTrue(result)

    def testRenderWithPainterClipRegions(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys.shp'))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_color': '#000000', 'outline_width': '1'})
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        mapsettings.setExtent(QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5))
        mapsettings.setLayers([poly_layer])

        region = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon ((-11725957 5368254, -12222900 4807501, -12246014 3834025, -12014878 3496059, -11259833 3518307, -10751333 3621153, -10574129 4516741, -10847640 5194995, -11105742 5325957, -11725957 5368254))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        region2 = QgsMapClippingRegion(QgsGeometry.fromWkt('Polygon ((-11032549 5421399, -11533344 4693167, -11086481 4229112, -11167378 3742984, -10616504 3553984, -10161936 3925771, -9618766 4668482, -9472380 5620753, -10115709 5965063, -11032549 5421399))'))
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_painterclip_region')
        result = renderchecker.runTest('expected_painterclip_region')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_painterclip_region')
        result = renderchecker.runTest('expected_painterclip_region')
        self.report += renderchecker.report()
        self.assertTrue(result)

    def testRenderWithPainterClipRegionsMultiPolygon(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys.shp'))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_color': '#000000', 'outline_width': '1'})
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        mapsettings.setExtent(QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5))
        mapsettings.setLayers([poly_layer])

        region = QgsMapClippingRegion(QgsGeometry.fromWkt(
            'MultiSurface (Polygon ((-10856627.66351187042891979 5625411.45629768911749125, -11083997.96136780828237534 4995770.63146586995571852, -10887235.20360786281526089 4357384.79517805296927691, -9684796.12840820662677288 4851477.9424419105052948, -10069576.63247209787368774 5428648.69853774644434452, -10856627.66351187042891979 5625411.45629768911749125)),Polygon ((-12045949.22152753174304962 5533588.83600971661508083, -12758667.65519132651388645 4868967.96535390708595514, -12478827.28859940730035305 4296169.71498607192188501, -11783598.87784760631620884 4077544.42858613422140479, -11223918.14466376602649689 4715930.26487395167350769, -11127723.01864779368042946 5673509.01930567622184753, -11359465.8222317285835743 5809056.69687363691627979, -12045949.22152753174304962 5533588.83600971661508083),(-11341975.79931973293423653 4790262.86224992945790291, -11722383.7976556234061718 4318032.24362606555223465, -12019714.18715953826904297 4606617.62167398259043694, -11757363.84347961470484734 4908320.51690589636564255, -11341975.79931973293423653 4790262.86224992945790291)))'))
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        mapsettings.addClippingRegion(region)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_painterclip_region_multi')
        result = renderchecker.runTest('expected_painterclip_region_multi')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_painterclip_region_multi')
        result = renderchecker.runTest('expected_painterclip_region_multi')
        self.report += renderchecker.report()
        self.assertTrue(result)

    def testRenderMultipleRenderersBelow(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys.shp'))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple({'color': '#ffaaff', 'outline_color': '#000000', 'outline_width': '1'})
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        # add secondary renderer, for rendering below
        class Gen1(QgsFeatureRendererGenerator):

            def id(self):
                return 'Gen1'

            def level(self):
                return -2

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute('Name')

                sym1 = QgsFillSymbol.createSimple(
                    {'color': '#ffaaff', 'outline_color': '#33aa33', 'outline_width': '3'})
                sym2 = QgsFillSymbol.createSimple(
                    {'color': '#ffaaff', 'outline_color': '#aa33aa', 'outline_width': '3'})

                renderer.addCategory(QgsRendererCategory('Dam', sym1, 'Dam'))
                renderer.addCategory(QgsRendererCategory('Lake', sym2, 'Lake'))
                return renderer

        # add secondary renderer, for rendering below
        class Gen2(QgsFeatureRendererGenerator):

            def id(self):
                return 'Gen2'

            def level(self):
                return -3

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute('Value < 12')

                sym1 = QgsFillSymbol.createSimple(
                    {'color': '#ffaaff', 'outline_color': '#aa1111', 'outline_width': '5'})
                sym2 = QgsFillSymbol.createSimple(
                    {'color': '#ffaaff', 'outline_color': '#1111dd', 'outline_width': '5'})

                renderer.addCategory(QgsRendererCategory('1', sym1, '1'))
                renderer.addCategory(QgsRendererCategory('0', sym2, '0'))
                return renderer

        poly_layer.addFeatureRendererGenerator(Gen1())
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], ['Gen1'])
        poly_layer.addFeatureRendererGenerator(Gen2())
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], ['Gen1', 'Gen2'])

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        mapsettings.setExtent(QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5))
        mapsettings.setLayers([poly_layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_multiple_renderers_below')
        result = renderchecker.runTest('expected_multiple_renderers_below')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_multiple_renderers_below')
        result = renderchecker.runTest('expected_multiple_renderers_below')
        self.report += renderchecker.report()
        self.assertTrue(result)

        poly_layer.removeFeatureRendererGenerator('Gen3')
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], ['Gen1', 'Gen2'])
        poly_layer.removeFeatureRendererGenerator('Gen1')
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], ['Gen2'])
        poly_layer.removeFeatureRendererGenerator('Gen1')
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], ['Gen2'])
        poly_layer.removeFeatureRendererGenerator('Gen2')
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], [])
        poly_layer.removeFeatureRendererGenerator('Gen1')
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], [])

    def testRenderMultipleRenderersAbove(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys.shp'))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple({'color': '#ffaaff', 'outline_color': '#000000', 'outline_width': '1'})
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        # add secondary renderer, for rendering below
        class Gen1(QgsFeatureRendererGenerator):

            def id(self):
                return 'Gen1'

            def level(self):
                return 3

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute('Name')

                cf1 = QgsCentroidFillSymbolLayer()
                cf1.setSubSymbol(QgsMarkerSymbol.createSimple(
                    {'color': '#33aa33', 'outline_style': 'no', 'size': '5'}))
                sym1 = QgsFillSymbol([cf1])

                cf2 = QgsCentroidFillSymbolLayer()
                cf2.setSubSymbol(QgsMarkerSymbol.createSimple(
                    {'color': '#aa33aa', 'outline_style': 'no', 'size': '5'}))
                sym2 = QgsFillSymbol([cf2])

                renderer.addCategory(QgsRendererCategory('Dam', sym1, 'Dam'))
                renderer.addCategory(QgsRendererCategory('Lake', sym2, 'Lake'))
                return renderer

        # add secondary renderer, for rendering below
        class Gen2(QgsFeatureRendererGenerator):

            def id(self):
                return 'Gen2'

            def level(self):
                return 2

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute('Value < 12')

                cf1 = QgsCentroidFillSymbolLayer()
                cf1.setSubSymbol(QgsMarkerSymbol.createSimple(
                    {'color': '#aa1111', 'outline_style': 'no', 'size': '8'}))
                sym1 = QgsFillSymbol([cf1])

                cf2 = QgsCentroidFillSymbolLayer()
                cf2.setSubSymbol(QgsMarkerSymbol.createSimple(
                    {'color': '#1111dd', 'outline_style': 'no', 'size': '8'}))
                sym2 = QgsFillSymbol([cf2])

                renderer.addCategory(QgsRendererCategory('1', sym1, '1'))
                renderer.addCategory(QgsRendererCategory('0', sym2, '0'))
                return renderer

        poly_layer.addFeatureRendererGenerator(Gen1())
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], ['Gen1'])
        poly_layer.addFeatureRendererGenerator(Gen2())
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], ['Gen1', 'Gen2'])

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        mapsettings.setExtent(QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5))
        mapsettings.setLayers([poly_layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_multiple_renderers_above')
        result = renderchecker.runTest('expected_multiple_renderers_above')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_multiple_renderers_above')
        result = renderchecker.runTest('expected_multiple_renderers_above')
        self.report += renderchecker.report()
        self.assertTrue(result)

    def testRenderMultipleRenderersAboveAndBelow(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys.shp'))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple({'color': '#ffaaff', 'outline_color': '#000000', 'outline_width': '1'})
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        # add secondary renderer, for rendering below
        class Gen1(QgsFeatureRendererGenerator):

            def id(self):
                return 'Gen1'

            def level(self):
                return 3

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute('Name')

                cf1 = QgsCentroidFillSymbolLayer()
                cf1.setSubSymbol(QgsMarkerSymbol.createSimple(
                    {'color': '#33aa33', 'outline_style': 'no', 'size': '5'}))
                sym1 = QgsFillSymbol([cf1])

                cf2 = QgsCentroidFillSymbolLayer()
                cf2.setSubSymbol(QgsMarkerSymbol.createSimple(
                    {'color': '#aa33aa', 'outline_style': 'no', 'size': '5'}))
                sym2 = QgsFillSymbol([cf2])

                renderer.addCategory(QgsRendererCategory('Dam', sym1, 'Dam'))
                renderer.addCategory(QgsRendererCategory('Lake', sym2, 'Lake'))
                return renderer

        # add secondary renderer, for rendering below
        class Gen2(QgsFeatureRendererGenerator):

            def id(self):
                return 'Gen2'

            def level(self):
                return 2

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute('Value < 12')

                cf1 = QgsCentroidFillSymbolLayer()
                cf1.setSubSymbol(QgsMarkerSymbol.createSimple(
                    {'color': '#aa1111', 'outline_style': 'no', 'size': '8'}))
                sym1 = QgsFillSymbol([cf1])

                cf2 = QgsCentroidFillSymbolLayer()
                cf2.setSubSymbol(QgsMarkerSymbol.createSimple(
                    {'color': '#1111dd', 'outline_style': 'no', 'size': '8'}))
                sym2 = QgsFillSymbol([cf2])

                renderer.addCategory(QgsRendererCategory('1', sym1, '1'))
                renderer.addCategory(QgsRendererCategory('0', sym2, '0'))
                return renderer

        # add secondary renderer, for rendering below

        class Gen1b(QgsFeatureRendererGenerator):

            def id(self):
                return 'Gen1b'

            def level(self):
                return -2

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute('Name')

                sym1 = QgsFillSymbol.createSimple(
                    {'color': '#ffaaff', 'outline_color': '#33aa33', 'outline_width': '3'})
                sym2 = QgsFillSymbol.createSimple(
                    {'color': '#ffaaff', 'outline_color': '#aa33aa', 'outline_width': '3'})

                renderer.addCategory(QgsRendererCategory('Dam', sym1, 'Dam'))
                renderer.addCategory(QgsRendererCategory('Lake', sym2, 'Lake'))
                return renderer

        # add secondary renderer, for rendering below
        class Gen2b(QgsFeatureRendererGenerator):

            def id(self):
                return 'Gen2b'

            def level(self):
                return -3

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute('Value < 12')

                sym1 = QgsFillSymbol.createSimple(
                    {'color': '#ffaaff', 'outline_color': '#aa1111', 'outline_width': '5'})
                sym2 = QgsFillSymbol.createSimple(
                    {'color': '#ffaaff', 'outline_color': '#1111dd', 'outline_width': '5'})

                renderer.addCategory(QgsRendererCategory('1', sym1, '1'))
                renderer.addCategory(QgsRendererCategory('0', sym2, '0'))
                return renderer

        poly_layer.addFeatureRendererGenerator(Gen1())
        poly_layer.addFeatureRendererGenerator(Gen2())
        poly_layer.addFeatureRendererGenerator(Gen1b())
        poly_layer.addFeatureRendererGenerator(Gen2b())

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        mapsettings.setExtent(QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5))
        mapsettings.setLayers([poly_layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_multiple_renderers_both_above_below')
        result = renderchecker.runTest('expected_multiple_renderers_both_above_below')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_multiple_renderers_both_above_below')
        result = renderchecker.runTest('expected_multiple_renderers_both_above_below')
        self.report += renderchecker.report()
        self.assertTrue(result)

    def testRenderMultipleRenderersSelection(self):
        """
        Test that selection colors only apply to main renderer
        :return:
        """
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'polys.shp'))
        self.assertTrue(poly_layer.isValid())
        poly_layer.selectAll()

        sym1 = QgsFillSymbol.createSimple({'color': '#ffaaff', 'outline_color': '#000000', 'outline_style': 'no'})
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        # add secondary renderer, for rendering below
        class Gen1(QgsFeatureRendererGenerator):

            def id(self):
                return 'Gen1'

            def level(self):
                return -2

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute('Name')

                sym1 = QgsFillSymbol.createSimple(
                    {'color': '#ffaaff', 'outline_color': '#33aa33', 'outline_width': '3'})
                sym2 = QgsFillSymbol.createSimple(
                    {'color': '#ffaaff', 'outline_color': '#aa33aa', 'outline_width': '3'})

                renderer.addCategory(QgsRendererCategory('Dam', sym1, 'Dam'))
                renderer.addCategory(QgsRendererCategory('Lake', sym2, 'Lake'))
                return renderer

        poly_layer.addFeatureRendererGenerator(Gen1())

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem('EPSG:3857'))
        mapsettings.setExtent(QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5))
        mapsettings.setLayers([poly_layer])

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_multiple_renderers_selection')
        result = renderchecker.runTest('expected_multiple_renderers_selection')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_multiple_renderers_selection')
        result = renderchecker.runTest('expected_multiple_renderers_selection')
        self.report += renderchecker.report()
        self.assertTrue(result)

    def test_reference_scale(self):
        """
        Test rendering a layer with a reference scale set
        """
        layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'lines.shp'), 'Lines', 'ogr')
        self.assertTrue(layer.isValid())

        sym1 = QgsLineSymbol.createSimple({'line_color': '#4dbf6f', 'line_width': 4, 'line_width_unit': "points"})

        renderer = QgsSingleSymbolRenderer(sym1)
        layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setExtent(layer.extent())
        mapsettings.setLayers([layer])
        self.assertAlmostEqual(mapsettings.scale(), 22738556, -5)

        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_reference_scale_not_set')
        result = renderchecker.runTest('expected_reference_scale_not_set')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # Set the reference scale as half the map scale -- the lines should be double as wide
        # as their preset width
        renderer.setReferenceScale(22738556 * 2)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_reference_scale_double')
        result = renderchecker.runTest('expected_reference_scale_double')
        self.report += renderchecker.report()
        self.assertTrue(result)

        # Set the reference scale as double the map scale -- the lines should be half as wide
        # as their preset width
        renderer.setReferenceScale(22738556 / 2)
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(mapsettings)
        renderchecker.setControlPathPrefix('vectorlayerrenderer')
        renderchecker.setControlName('expected_reference_scale_half')
        result = renderchecker.runTest('expected_reference_scale_half')
        self.report += renderchecker.report()
        self.assertTrue(result)


if __name__ == '__main__':
    unittest.main()
