"""QGIS Unit tests for QgsVectorLayerRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2020-06"
__copyright__ = "Copyright 2020, The QGIS Project"

import os

from qgis.PyQt.QtCore import QDir, QSize
from qgis.PyQt.QtGui import QColor

from qgis.core import (
    edit,
    Qgis,
    QgsCategorizedSymbolRenderer,
    QgsCentroidFillSymbolLayer,
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsFeatureRendererGenerator,
    QgsFillSymbol,
    QgsGeometry,
    QgsGeometryGeneratorSymbolLayer,
    QgsLineSymbol,
    QgsMapClippingRegion,
    QgsMapSettings,
    QgsMarkerSymbol,
    QgsPointXY,
    QgsRectangle,
    QgsRendererCategory,
    QgsRuleBasedRenderer,
    QgsSingleSymbolRenderer,
    QgsSymbol,
    QgsRenderContext,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsVectorLayerRenderer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "vectorlayerrenderer"

    def testRenderWithIntersectsRegions(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ff00ff", "outline_color": "#000000", "outline_width": "1"}
        )
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])

        region = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "Polygon ((-11725957 5368254, -12222900 4807501, -12246014 3834025, -12014878 3496059, -11259833 3518307, -10751333 3621153, -10574129 4516741, -10847640 5194995, -11105742 5325957, -11725957 5368254))"
            )
        )
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        region2 = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "Polygon ((-11032549 5421399, -11533344 4693167, -11086481 4229112, -11167378 3742984, -10616504 3553984, -10161936 3925771, -9618766 4668482, -9472380 5620753, -10115709 5965063, -11032549 5421399))"
            )
        )
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.NoClipping)
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        self.assertTrue(
            self.render_map_settings_check(
                "intersects_region", "intersects_region", mapsettings
            )
        )

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        self.assertTrue(
            self.render_map_settings_check(
                "intersects_region", "intersects_region", mapsettings
            )
        )

    def testRenderWithIntersectionRegions(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ff00ff", "outline_color": "#000000", "outline_width": "1"}
        )
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])

        region = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "Polygon ((-11725957 5368254, -12222900 4807501, -12246014 3834025, -12014878 3496059, -11259833 3518307, -10751333 3621153, -10574129 4516741, -10847640 5194995, -11105742 5325957, -11725957 5368254))"
            )
        )
        region.setFeatureClip(
            QgsMapClippingRegion.FeatureClippingType.ClipToIntersection
        )
        region2 = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "Polygon ((-11032549 5421399, -11533344 4693167, -11086481 4229112, -11167378 3742984, -10616504 3553984, -10161936 3925771, -9618766 4668482, -9472380 5620753, -10115709 5965063, -11032549 5421399))"
            )
        )
        region2.setFeatureClip(
            QgsMapClippingRegion.FeatureClippingType.ClipToIntersection
        )
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        self.assertTrue(
            self.render_map_settings_check(
                "intersection_region", "intersection_region", mapsettings
            )
        )

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        self.assertTrue(
            self.render_map_settings_check(
                "intersection_region", "intersection_region", mapsettings
            )
        )

    def testIntersectionRuleBased(self):
        """
        Test that rule based renderer using intersection clip paths correctly uses original feature area for rule
        evaluation, not clipped area
        """
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ff00ff", "outline_color": "#000000", "outline_width": "1"}
        )
        sym2 = QgsFillSymbol.createSimple(
            {"color": "#00ffff", "outline_color": "#000000", "outline_width": "1"}
        )

        r1 = QgsRuleBasedRenderer.Rule(sym1, 0, 0, "area($geometry)>25")
        r2 = QgsRuleBasedRenderer.Rule(sym2, 0, 0, "ELSE")

        rootrule = QgsRuleBasedRenderer.Rule(None)
        rootrule.appendChild(r1)
        rootrule.appendChild(r2)
        renderer = QgsRuleBasedRenderer(rootrule)
        poly_layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])
        mapsettings.setEllipsoid("")

        region = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "Polygon ((-11725957 5368254, -12222900 4807501, -12246014 3834025, -12014878 3496059, -11259833 3518307, -10751333 3621153, -10574129 4516741, -10847640 5194995, -11105742 5325957, -11725957 5368254))"
            )
        )
        region.setFeatureClip(
            QgsMapClippingRegion.FeatureClippingType.ClipToIntersection
        )
        region2 = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "Polygon ((-11032549 5421399, -11533344 4693167, -11086481 4229112, -11167378 3742984, -10616504 3553984, -10161936 3925771, -9618766 4668482, -9472380 5620753, -10115709 5965063, -11032549 5421399))"
            )
        )
        region2.setFeatureClip(
            QgsMapClippingRegion.FeatureClippingType.ClipToIntersection
        )
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        self.assertTrue(
            self.render_map_settings_check(
                "intersection_rule_based", "intersection_rule_based", mapsettings
            )
        )

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        self.assertTrue(
            self.render_map_settings_check(
                "intersection_rule_based", "intersection_rule_based", mapsettings
            )
        )

    def testRenderWithPainterClipRegions(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ff00ff", "outline_color": "#000000", "outline_width": "1"}
        )
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])

        region = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "Polygon ((-11725957 5368254, -12222900 4807501, -12246014 3834025, -12014878 3496059, -11259833 3518307, -10751333 3621153, -10574129 4516741, -10847640 5194995, -11105742 5325957, -11725957 5368254))"
            )
        )
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        region2 = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "Polygon ((-11032549 5421399, -11533344 4693167, -11086481 4229112, -11167378 3742984, -10616504 3553984, -10161936 3925771, -9618766 4668482, -9472380 5620753, -10115709 5965063, -11032549 5421399))"
            )
        )
        region2.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        mapsettings.addClippingRegion(region)
        mapsettings.addClippingRegion(region2)

        self.assertTrue(
            self.render_map_settings_check(
                "painterclip_region", "painterclip_region", mapsettings
            )
        )

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        self.assertTrue(
            self.render_map_settings_check(
                "painterclip_region", "painterclip_region", mapsettings
            )
        )

    def testRenderWithPainterClipRegionsMultiPolygon(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ff00ff", "outline_color": "#000000", "outline_width": "1"}
        )
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])

        region = QgsMapClippingRegion(
            QgsGeometry.fromWkt(
                "MultiSurface (Polygon ((-10856627.66351187042891979 5625411.45629768911749125, -11083997.96136780828237534 4995770.63146586995571852, -10887235.20360786281526089 4357384.79517805296927691, -9684796.12840820662677288 4851477.9424419105052948, -10069576.63247209787368774 5428648.69853774644434452, -10856627.66351187042891979 5625411.45629768911749125)),Polygon ((-12045949.22152753174304962 5533588.83600971661508083, -12758667.65519132651388645 4868967.96535390708595514, -12478827.28859940730035305 4296169.71498607192188501, -11783598.87784760631620884 4077544.42858613422140479, -11223918.14466376602649689 4715930.26487395167350769, -11127723.01864779368042946 5673509.01930567622184753, -11359465.8222317285835743 5809056.69687363691627979, -12045949.22152753174304962 5533588.83600971661508083),(-11341975.79931973293423653 4790262.86224992945790291, -11722383.7976556234061718 4318032.24362606555223465, -12019714.18715953826904297 4606617.62167398259043694, -11757363.84347961470484734 4908320.51690589636564255, -11341975.79931973293423653 4790262.86224992945790291)))"
            )
        )
        region.setFeatureClip(QgsMapClippingRegion.FeatureClippingType.ClipPainterOnly)
        mapsettings.addClippingRegion(region)

        self.assertTrue(
            self.render_map_settings_check(
                "painterclip_region_multi", "painterclip_region_multi", mapsettings
            )
        )

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        self.assertTrue(
            self.render_map_settings_check(
                "painterclip_region_multi", "painterclip_region_multi", mapsettings
            )
        )

    def testRenderMultipleRenderersBelow(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ffaaff", "outline_color": "#000000", "outline_width": "1"}
        )
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        # add secondary renderer, for rendering below
        class Gen1(QgsFeatureRendererGenerator):

            def id(self):
                return "Gen1"

            def level(self):
                return -2

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute("Name")

                sym1 = QgsFillSymbol.createSimple(
                    {
                        "color": "#ffaaff",
                        "outline_color": "#33aa33",
                        "outline_width": "3",
                    }
                )
                sym2 = QgsFillSymbol.createSimple(
                    {
                        "color": "#ffaaff",
                        "outline_color": "#aa33aa",
                        "outline_width": "3",
                    }
                )

                renderer.addCategory(QgsRendererCategory("Dam", sym1, "Dam"))
                renderer.addCategory(QgsRendererCategory("Lake", sym2, "Lake"))
                return renderer

        # add secondary renderer, for rendering below
        class Gen2(QgsFeatureRendererGenerator):

            def id(self):
                return "Gen2"

            def level(self):
                return -3

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute("Value < 12")

                sym1 = QgsFillSymbol.createSimple(
                    {
                        "color": "#ffaaff",
                        "outline_color": "#aa1111",
                        "outline_width": "5",
                    }
                )
                sym2 = QgsFillSymbol.createSimple(
                    {
                        "color": "#ffaaff",
                        "outline_color": "#1111dd",
                        "outline_width": "5",
                    }
                )

                renderer.addCategory(QgsRendererCategory("1", sym1, "1"))
                renderer.addCategory(QgsRendererCategory("0", sym2, "0"))
                return renderer

        poly_layer.addFeatureRendererGenerator(Gen1())
        self.assertEqual(
            [g.id() for g in poly_layer.featureRendererGenerators()], ["Gen1"]
        )
        poly_layer.addFeatureRendererGenerator(Gen2())
        self.assertEqual(
            [g.id() for g in poly_layer.featureRendererGenerators()], ["Gen1", "Gen2"]
        )

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "multiple_renderers_below", "multiple_renderers_below", mapsettings
            )
        )

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        self.assertTrue(
            self.render_map_settings_check(
                "multiple_renderers_below", "multiple_renderers_below", mapsettings
            )
        )

        poly_layer.removeFeatureRendererGenerator("Gen3")
        self.assertEqual(
            [g.id() for g in poly_layer.featureRendererGenerators()], ["Gen1", "Gen2"]
        )
        poly_layer.removeFeatureRendererGenerator("Gen1")
        self.assertEqual(
            [g.id() for g in poly_layer.featureRendererGenerators()], ["Gen2"]
        )
        poly_layer.removeFeatureRendererGenerator("Gen1")
        self.assertEqual(
            [g.id() for g in poly_layer.featureRendererGenerators()], ["Gen2"]
        )
        poly_layer.removeFeatureRendererGenerator("Gen2")
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], [])
        poly_layer.removeFeatureRendererGenerator("Gen1")
        self.assertEqual([g.id() for g in poly_layer.featureRendererGenerators()], [])

    def testRenderMultipleRenderersAbove(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ffaaff", "outline_color": "#000000", "outline_width": "1"}
        )
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        # add secondary renderer, for rendering below
        class Gen1(QgsFeatureRendererGenerator):

            def id(self):
                return "Gen1"

            def level(self):
                return 3

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute("Name")

                cf1 = QgsCentroidFillSymbolLayer()
                cf1.setSubSymbol(
                    QgsMarkerSymbol.createSimple(
                        {"color": "#33aa33", "outline_style": "no", "size": "5"}
                    )
                )
                sym1 = QgsFillSymbol([cf1])

                cf2 = QgsCentroidFillSymbolLayer()
                cf2.setSubSymbol(
                    QgsMarkerSymbol.createSimple(
                        {"color": "#aa33aa", "outline_style": "no", "size": "5"}
                    )
                )
                sym2 = QgsFillSymbol([cf2])

                renderer.addCategory(QgsRendererCategory("Dam", sym1, "Dam"))
                renderer.addCategory(QgsRendererCategory("Lake", sym2, "Lake"))
                return renderer

        # add secondary renderer, for rendering below
        class Gen2(QgsFeatureRendererGenerator):

            def id(self):
                return "Gen2"

            def level(self):
                return 2

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute("Value < 12")

                cf1 = QgsCentroidFillSymbolLayer()
                cf1.setSubSymbol(
                    QgsMarkerSymbol.createSimple(
                        {"color": "#aa1111", "outline_style": "no", "size": "8"}
                    )
                )
                sym1 = QgsFillSymbol([cf1])

                cf2 = QgsCentroidFillSymbolLayer()
                cf2.setSubSymbol(
                    QgsMarkerSymbol.createSimple(
                        {"color": "#1111dd", "outline_style": "no", "size": "8"}
                    )
                )
                sym2 = QgsFillSymbol([cf2])

                renderer.addCategory(QgsRendererCategory("1", sym1, "1"))
                renderer.addCategory(QgsRendererCategory("0", sym2, "0"))
                return renderer

        poly_layer.addFeatureRendererGenerator(Gen1())
        self.assertEqual(
            [g.id() for g in poly_layer.featureRendererGenerators()], ["Gen1"]
        )
        poly_layer.addFeatureRendererGenerator(Gen2())
        self.assertEqual(
            [g.id() for g in poly_layer.featureRendererGenerators()], ["Gen1", "Gen2"]
        )

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "multiple_renderers_above", "multiple_renderers_above", mapsettings
            )
        )

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        self.assertTrue(
            self.render_map_settings_check(
                "multiple_renderers_above", "multiple_renderers_above", mapsettings
            )
        )

    def testRenderMultipleRenderersAboveAndBelow(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ffaaff", "outline_color": "#000000", "outline_width": "1"}
        )
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        # add secondary renderer, for rendering below
        class Gen1(QgsFeatureRendererGenerator):

            def id(self):
                return "Gen1"

            def level(self):
                return 3

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute("Name")

                cf1 = QgsCentroidFillSymbolLayer()
                cf1.setSubSymbol(
                    QgsMarkerSymbol.createSimple(
                        {"color": "#33aa33", "outline_style": "no", "size": "5"}
                    )
                )
                sym1 = QgsFillSymbol([cf1])

                cf2 = QgsCentroidFillSymbolLayer()
                cf2.setSubSymbol(
                    QgsMarkerSymbol.createSimple(
                        {"color": "#aa33aa", "outline_style": "no", "size": "5"}
                    )
                )
                sym2 = QgsFillSymbol([cf2])

                renderer.addCategory(QgsRendererCategory("Dam", sym1, "Dam"))
                renderer.addCategory(QgsRendererCategory("Lake", sym2, "Lake"))
                return renderer

        # add secondary renderer, for rendering below
        class Gen2(QgsFeatureRendererGenerator):

            def id(self):
                return "Gen2"

            def level(self):
                return 2

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute("Value < 12")

                cf1 = QgsCentroidFillSymbolLayer()
                cf1.setSubSymbol(
                    QgsMarkerSymbol.createSimple(
                        {"color": "#aa1111", "outline_style": "no", "size": "8"}
                    )
                )
                sym1 = QgsFillSymbol([cf1])

                cf2 = QgsCentroidFillSymbolLayer()
                cf2.setSubSymbol(
                    QgsMarkerSymbol.createSimple(
                        {"color": "#1111dd", "outline_style": "no", "size": "8"}
                    )
                )
                sym2 = QgsFillSymbol([cf2])

                renderer.addCategory(QgsRendererCategory("1", sym1, "1"))
                renderer.addCategory(QgsRendererCategory("0", sym2, "0"))
                return renderer

        # add secondary renderer, for rendering below

        class Gen1b(QgsFeatureRendererGenerator):

            def id(self):
                return "Gen1b"

            def level(self):
                return -2

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute("Name")

                sym1 = QgsFillSymbol.createSimple(
                    {
                        "color": "#ffaaff",
                        "outline_color": "#33aa33",
                        "outline_width": "3",
                    }
                )
                sym2 = QgsFillSymbol.createSimple(
                    {
                        "color": "#ffaaff",
                        "outline_color": "#aa33aa",
                        "outline_width": "3",
                    }
                )

                renderer.addCategory(QgsRendererCategory("Dam", sym1, "Dam"))
                renderer.addCategory(QgsRendererCategory("Lake", sym2, "Lake"))
                return renderer

        # add secondary renderer, for rendering below
        class Gen2b(QgsFeatureRendererGenerator):

            def id(self):
                return "Gen2b"

            def level(self):
                return -3

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute("Value < 12")

                sym1 = QgsFillSymbol.createSimple(
                    {
                        "color": "#ffaaff",
                        "outline_color": "#aa1111",
                        "outline_width": "5",
                    }
                )
                sym2 = QgsFillSymbol.createSimple(
                    {
                        "color": "#ffaaff",
                        "outline_color": "#1111dd",
                        "outline_width": "5",
                    }
                )

                renderer.addCategory(QgsRendererCategory("1", sym1, "1"))
                renderer.addCategory(QgsRendererCategory("0", sym2, "0"))
                return renderer

        poly_layer.addFeatureRendererGenerator(Gen1())
        poly_layer.addFeatureRendererGenerator(Gen2())
        poly_layer.addFeatureRendererGenerator(Gen1b())
        poly_layer.addFeatureRendererGenerator(Gen2b())

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "multiple_renderers_both_above_below",
                "multiple_renderers_both_above_below",
                mapsettings,
            )
        )

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        self.assertTrue(
            self.render_map_settings_check(
                "multiple_renderers_both_above_below",
                "multiple_renderers_both_above_below",
                mapsettings,
            )
        )

    def testRenderMultipleRenderersSelection(self):
        """
        Test that selection colors only apply to main renderer
        :return:
        """
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())
        poly_layer.selectAll()

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ffaaff", "outline_color": "#000000", "outline_style": "no"}
        )
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        # add secondary renderer, for rendering below
        class Gen1(QgsFeatureRendererGenerator):

            def id(self):
                return "Gen1"

            def level(self):
                return -2

            def createRenderer(self):
                renderer = QgsCategorizedSymbolRenderer()
                renderer.setClassAttribute("Name")

                sym1 = QgsFillSymbol.createSimple(
                    {
                        "color": "#ffaaff",
                        "outline_color": "#33aa33",
                        "outline_width": "3",
                    }
                )
                sym2 = QgsFillSymbol.createSimple(
                    {
                        "color": "#ffaaff",
                        "outline_color": "#aa33aa",
                        "outline_width": "3",
                    }
                )

                renderer.addCategory(QgsRendererCategory("Dam", sym1, "Dam"))
                renderer.addCategory(QgsRendererCategory("Lake", sym2, "Lake"))
                return renderer

        poly_layer.addFeatureRendererGenerator(Gen1())

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "multiple_renderers_selection",
                "multiple_renderers_selection",
                mapsettings,
            )
        )

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        self.assertTrue(
            self.render_map_settings_check(
                "multiple_renderers_selection",
                "multiple_renderers_selection",
                mapsettings,
            )
        )

    def test_reference_scale(self):
        """
        Test rendering a layer with a reference scale set
        """
        layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "lines.shp"), "Lines", "ogr")
        self.assertTrue(layer.isValid())

        sym1 = QgsLineSymbol.createSimple(
            {"line_color": "#4dbf6f", "line_width": 4, "line_width_unit": "points"}
        )

        renderer = QgsSingleSymbolRenderer(sym1)
        layer.setRenderer(renderer)

        mapsettings = QgsMapSettings()
        mapsettings.setDestinationCrs(layer.crs())
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setExtent(layer.extent())
        mapsettings.setLayers([layer])
        self.assertAlmostEqual(mapsettings.scale(), 22738556, -5)

        self.assertTrue(
            self.render_map_settings_check(
                "reference_scale_not_set", "reference_scale_not_set", mapsettings
            )
        )

        # Set the reference scale as half the map scale -- the lines should be double as wide
        # as their preset width
        renderer.setReferenceScale(22738556 * 2)
        self.assertTrue(
            self.render_map_settings_check(
                "reference_scale_double", "reference_scale_double", mapsettings
            )
        )

        # Set the reference scale as double the map scale -- the lines should be half as wide
        # as their preset width
        renderer.setReferenceScale(22738556 / 2)
        self.assertTrue(
            self.render_map_settings_check(
                "reference_scale_half", "reference_scale_half", mapsettings
            )
        )

    def testRenderWithSelectedFeatureColor(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ff00ff", "outline_color": "#000000", "outline_width": "1"}
        )
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        poly_layer.selectAll()
        poly_layer.selectionProperties().setSelectionColor(QColor(255, 0, 0))
        poly_layer.selectionProperties().setSelectionRenderingMode(
            Qgis.SelectionRenderingMode.CustomColor
        )

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "selection_color", "selection_color", mapsettings
            )
        )

    def testRenderWithSelectedFeatureSymbol(self):
        poly_layer = QgsVectorLayer(os.path.join(TEST_DATA_DIR, "polys.shp"))
        self.assertTrue(poly_layer.isValid())

        sym1 = QgsFillSymbol.createSimple(
            {"color": "#ff00ff", "outline_color": "#000000", "outline_width": "1"}
        )
        renderer = QgsSingleSymbolRenderer(sym1)
        poly_layer.setRenderer(renderer)

        poly_layer.selectAll()

        poly_layer.selectionProperties().setSelectionSymbol(
            QgsFillSymbol.createSimple(
                {"style": "no", "outline_color": "#6666ff", "outline_width": "3"}
            )
        )
        poly_layer.selectionProperties().setSelectionRenderingMode(
            Qgis.SelectionRenderingMode.CustomSymbol
        )

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(
            QgsRectangle(-13875783.2, 2266009.4, -8690110.7, 6673344.5)
        )
        mapsettings.setLayers([poly_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "selection_symbol", "selection_symbol", mapsettings
            )
        )

        # also try with symbol levels
        renderer.setUsingSymbolLevels(True)
        poly_layer.setRenderer(renderer)

        self.assertTrue(
            self.render_map_settings_check(
                "selection_symbol", "selection_symbol", mapsettings
            )
        )

    def testRenderWithExtentBuffer(self):
        def createFeature(x: float, y: float) -> QgsFeature:
            feat = QgsFeature()
            feat.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(x, y)))

            return feat

        def createSymbol() -> QgsMarkerSymbol:
            sym = QgsMarkerSymbol.createSimple(
                {"color": "#33aa33", "outline_style": "no", "size": "5"}
            )
            return sym

        def createGeometryGenerator() -> QgsGeometryGeneratorSymbolLayer:
            geomgen = QgsGeometryGeneratorSymbolLayer.create(
                {"geometryModifier": "make_point($x + if($x <= 0, 5, -5), $y)"}
            )
            geomgen.setSymbolType(QgsSymbol.SymbolType.Marker)
            geomgen.setSubSymbol(
                QgsMarkerSymbol.createSimple(
                    {"color": "#ff00ff", "outline_style": "no", "size": "6"}
                )
            )

            return geomgen

        point_layer = QgsVectorLayer("Point?crs=EPSG:3857", "point layer", "memory")

        with edit(point_layer):
            point_layer.addFeature(createFeature(-15.999, 8))
            point_layer.addFeature(createFeature(-16.001, 6))
            point_layer.addFeature(createFeature(-13, 4))
            point_layer.addFeature(createFeature(-10, 2))
            point_layer.addFeature(createFeature(17, 0))
            point_layer.addFeature(createFeature(4, -2))
            point_layer.addFeature(createFeature(15.999, -4))
            point_layer.addFeature(createFeature(17, -6))
            point_layer.addFeature(createFeature(15.999, -8))

        sym1 = createSymbol()
        sym1.appendSymbolLayer(createGeometryGenerator())

        renderer1 = QgsSingleSymbolRenderer(sym1)
        point_layer.setRenderer(renderer1)

        mapsettings = QgsMapSettings()
        mapsettings.setOutputSize(QSize(400, 400))
        mapsettings.setOutputDpi(96)
        mapsettings.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        mapsettings.setExtent(QgsRectangle(-10, -10, 10, 10))
        mapsettings.setLayers([point_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "buffer_extent_zero",
                "buffer_extent_zero",
                mapsettings,
            )
        )

        sym2 = createSymbol()
        sym2.appendSymbolLayer(createGeometryGenerator())
        sym2.setExtentBuffer(1)

        renderer2 = QgsSingleSymbolRenderer(sym2)
        point_layer.setRenderer(renderer2)

        self.assertTrue(
            self.render_map_settings_check(
                "buffer_extent",
                "buffer_extent",
                mapsettings,
            )
        )


if __name__ == "__main__":
    unittest.main()
