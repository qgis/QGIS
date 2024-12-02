"""
***************************************************************************
    test_qgssimplelinesymbollayer.py
    ---------------------
    Date                 : November 2018
    Copyright            : (C) 2018 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "November 2018"
__copyright__ = "(C) 2018, Nyall Dawson"

import os

from qgis.PyQt.QtCore import QSize, Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsFeature,
    QgsFillSymbol,
    QgsGeometry,
    QgsLineSymbol,
    QgsLineSymbolLayer,
    QgsMapSettings,
    QgsMapUnitScale,
    QgsProperty,
    QgsReadWriteContext,
    QgsRectangle,
    QgsRenderContext,
    QgsSimpleLineSymbolLayer,
    QgsSingleSymbolRenderer,
    QgsSymbol,
    QgsSymbolLayer,
    QgsSymbolLayerUtils,
    QgsUnitTypes,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSimpleLineSymbolLayer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_simpleline"

    def testDashPatternWithDataDefinedWidth(self):
        # rendering test
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        s[0].setUseCustomDashPattern(True)
        s[0].setPenCapStyle(Qt.PenCapStyle.FlatCap)
        s[0].setCustomDashVector([3, 4, 5, 6])

        s[0].dataDefinedProperties().setProperty(
            QgsSymbolLayer.Property.PropertyStrokeWidth, QgsProperty.fromExpression("3")
        )

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "simpleline_dashpattern_datadefined_width",
                "simpleline_dashpattern_datadefined_width",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testTrimDistance(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "0.6"}
        )

        s.symbolLayer(0).setTrimDistanceStart(1.2)
        s.symbolLayer(0).setTrimDistanceStartUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        s.symbolLayer(0).setTrimDistanceStartMapUnitScale(QgsMapUnitScale(5, 10))
        s.symbolLayer(0).setTrimDistanceEnd(3.2)
        s.symbolLayer(0).setTrimDistanceEndUnit(
            QgsUnitTypes.RenderUnit.RenderPercentage
        )
        s.symbolLayer(0).setTrimDistanceEndMapUnitScale(QgsMapUnitScale(15, 20))

        s2 = s.clone()
        self.assertEqual(s2.symbolLayer(0).trimDistanceStart(), 1.2)
        self.assertEqual(
            s2.symbolLayer(0).trimDistanceStartUnit(),
            QgsUnitTypes.RenderUnit.RenderPoints,
        )
        self.assertEqual(s2.symbolLayer(0).trimDistanceStartMapUnitScale().minScale, 5)
        self.assertEqual(s2.symbolLayer(0).trimDistanceStartMapUnitScale().maxScale, 10)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEnd(), 3.2)
        self.assertEqual(
            s2.symbolLayer(0).trimDistanceEndUnit(),
            QgsUnitTypes.RenderUnit.RenderPercentage,
        )
        self.assertEqual(s2.symbolLayer(0).trimDistanceEndMapUnitScale().minScale, 15)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEndMapUnitScale().maxScale, 20)

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol("test", s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(s2.symbolLayer(0).trimDistanceStart(), 1.2)
        self.assertEqual(
            s2.symbolLayer(0).trimDistanceStartUnit(),
            QgsUnitTypes.RenderUnit.RenderPoints,
        )
        self.assertEqual(s2.symbolLayer(0).trimDistanceStartMapUnitScale().minScale, 5)
        self.assertEqual(s2.symbolLayer(0).trimDistanceStartMapUnitScale().maxScale, 10)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEnd(), 3.2)
        self.assertEqual(
            s2.symbolLayer(0).trimDistanceEndUnit(),
            QgsUnitTypes.RenderUnit.RenderPercentage,
        )
        self.assertEqual(s2.symbolLayer(0).trimDistanceEndMapUnitScale().minScale, 15)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEndMapUnitScale().maxScale, 20)

    def testTrimDistanceRender(self):
        """
        Rendering test of trim distances
        """
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        s.symbolLayer(0).setTrimDistanceStart(150)
        s.symbolLayer(0).setTrimDistanceStartUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        s.symbolLayer(0).setTrimDistanceEnd(9)
        s.symbolLayer(0).setTrimDistanceEndUnit(
            QgsUnitTypes.RenderUnit.RenderMillimeters
        )

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "simpleline_trim_distance_units",
                "simpleline_trim_distance_units",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testTrimDistanceRenderPercentage(self):
        """
        Rendering test of trim distances using percentage
        """
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        s.symbolLayer(0).setTrimDistanceStart(10)
        s.symbolLayer(0).setTrimDistanceStartUnit(
            QgsUnitTypes.RenderUnit.RenderPercentage
        )
        s.symbolLayer(0).setTrimDistanceEnd(50)
        s.symbolLayer(0).setTrimDistanceEndUnit(
            QgsUnitTypes.RenderUnit.RenderPercentage
        )

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "simpleline_trim_distance_percentage",
                "simpleline_trim_distance_percentage",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testTrimDistanceRenderDataDefined(self):
        """
        Rendering test of trim distances using data defined lengths
        """
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        s.symbolLayer(0).setTrimDistanceStart(1)
        s.symbolLayer(0).setTrimDistanceStartUnit(
            QgsUnitTypes.RenderUnit.RenderPercentage
        )
        s.symbolLayer(0).setTrimDistanceEnd(5)
        s.symbolLayer(0).setTrimDistanceEndUnit(
            QgsUnitTypes.RenderUnit.RenderPercentage
        )

        s.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyTrimStart, QgsProperty.fromExpression("5*2")
        )
        s.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyTrimEnd, QgsProperty.fromExpression("60-10")
        )

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "simpleline_trim_distance_percentage",
                "simpleline_trim_distance_percentage",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testDashPatternOffset(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "0.6"}
        )

        s.symbolLayer(0).setDashPatternOffset(1.2)
        s.symbolLayer(0).setDashPatternOffsetUnit(QgsUnitTypes.RenderUnit.RenderPoints)
        s.symbolLayer(0).setDashPatternOffsetMapUnitScale(QgsMapUnitScale(5, 10))

        s2 = s.clone()
        self.assertEqual(s2.symbolLayer(0).dashPatternOffset(), 1.2)
        self.assertEqual(
            s2.symbolLayer(0).dashPatternOffsetUnit(),
            QgsUnitTypes.RenderUnit.RenderPoints,
        )
        self.assertEqual(s2.symbolLayer(0).dashPatternOffsetMapUnitScale().minScale, 5)
        self.assertEqual(s2.symbolLayer(0).dashPatternOffsetMapUnitScale().maxScale, 10)

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol("test", s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(s2.symbolLayer(0).dashPatternOffset(), 1.2)
        self.assertEqual(
            s2.symbolLayer(0).dashPatternOffsetUnit(),
            QgsUnitTypes.RenderUnit.RenderPoints,
        )
        self.assertEqual(s2.symbolLayer(0).dashPatternOffsetMapUnitScale().minScale, 5)
        self.assertEqual(s2.symbolLayer(0).dashPatternOffsetMapUnitScale().maxScale, 10)

    def testDashPatternOffsetRender(self):
        # rendering test
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        s.symbolLayer(0).setPenStyle(Qt.PenStyle.DashDotDotLine)
        s.symbolLayer(0).setDashPatternOffset(10)
        s.symbolLayer(0).setDashPatternOffsetUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "simpleline_dashpattern_offset",
                "simpleline_dashpattern_offset",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testDashPatternOffsetRenderNegative(self):
        # rendering test
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        s.symbolLayer(0).setPenStyle(Qt.PenStyle.DashDotDotLine)
        s.symbolLayer(0).setDashPatternOffset(-10)
        s.symbolLayer(0).setDashPatternOffsetUnit(QgsUnitTypes.RenderUnit.RenderPoints)

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "simpleline_dashpattern_offset_negative",
                "simpleline_dashpattern_offset_negative",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testDashPatternOffsetRenderCustomPattern(self):
        # rendering test
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        s.symbolLayer(0).setUseCustomDashPattern(True)
        s.symbolLayer(0).setPenCapStyle(Qt.PenCapStyle.FlatCap)
        s.symbolLayer(0).setCustomDashVector([3, 4, 5, 6])
        s.symbolLayer(0).setDashPatternOffset(10)

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "simpleline_dashpattern_offset_custom",
                "simpleline_dashpattern_offset_custom",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testDashTweaks(self):
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "0.6"}
        )

        self.assertFalse(s.symbolLayer(0).alignDashPattern())
        self.assertFalse(s.symbolLayer(0).tweakDashPatternOnCorners())

        s.symbolLayer(0).setAlignDashPattern(True)
        s.symbolLayer(0).setTweakDashPatternOnCorners(True)

        s2 = s.clone()
        self.assertTrue(s2.symbolLayer(0).alignDashPattern())
        self.assertTrue(s2.symbolLayer(0).tweakDashPatternOnCorners())

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol("test", s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertTrue(s2.symbolLayer(0).alignDashPattern())
        self.assertTrue(s2.symbolLayer(0).tweakDashPatternOnCorners())

    def testAlignDashRender(self):
        # rendering test
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        s.symbolLayer(0).setPenStyle(Qt.PenStyle.DashDotDotLine)
        s.symbolLayer(0).setAlignDashPattern(True)

        g = QgsGeometry.fromWkt("LineString(0 0, 9.2 0, 9.2 10, 1.3 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "simpleline_aligndashpattern",
                "simpleline_aligndashpattern",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testDashCornerTweakDashRender(self):
        # rendering test
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )

        s.symbolLayer(0).setPenStyle(Qt.PenStyle.DashDotDotLine)
        s.symbolLayer(0).setAlignDashPattern(True)
        s.symbolLayer(0).setTweakDashPatternOnCorners(True)
        s.symbolLayer(0).setPenJoinStyle(Qt.PenJoinStyle.RoundJoin)

        g = QgsGeometry.fromWkt("LineString(0 0, 2 1, 3 1, 10 0, 10 10, 5 5)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "simpleline_dashcornertweak",
                "simpleline_dashcornertweak",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testAlignDashRenderSmallWidth(self):
        # rendering test
        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "0.1"}
        )

        s.symbolLayer(0).setPenStyle(Qt.PenStyle.DashDotDotLine)
        s.symbolLayer(0).setAlignDashPattern(True)

        g = QgsGeometry.fromWkt("LineString(0 0, 9.2 0, 9.2 10, 1.3 10)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "simpleline_aligndashpattern_small_width",
                "simpleline_aligndashpattern_small_width",
                rendered_image,
            )
        )

    def testRingNumberVariable(self):
        # test test geometry_ring_num variable
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=2))
        s3.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeColor,
            QgsProperty.fromExpression(
                "case when @geometry_ring_num=0 then 'green' when @geometry_ring_num=1 then 'blue' when @geometry_ring_num=2 then 'red' end"
            ),
        )

        g = QgsGeometry.fromWkt(
            "Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))"
        )
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(
            self.image_check(
                "simpleline_ring_num",
                "simpleline_ring_num",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testRingFilter(self):
        # test filtering rings during rendering

        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)
        s.appendSymbolLayer(QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=2))
        self.assertEqual(
            s.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.RenderRingFilter.AllRings
        )
        s.symbolLayer(0).setRingFilter(
            QgsLineSymbolLayer.RenderRingFilter.ExteriorRingOnly
        )
        self.assertEqual(
            s.symbolLayer(0).ringFilter(),
            QgsLineSymbolLayer.RenderRingFilter.ExteriorRingOnly,
        )

        s2 = s.clone()
        self.assertEqual(
            s2.symbolLayer(0).ringFilter(),
            QgsLineSymbolLayer.RenderRingFilter.ExteriorRingOnly,
        )

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol("test", s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(
            s2.symbolLayer(0).ringFilter(),
            QgsLineSymbolLayer.RenderRingFilter.ExteriorRingOnly,
        )

        # rendering test
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=2))
        s3.symbolLayer(0).setRingFilter(
            QgsLineSymbolLayer.RenderRingFilter.ExteriorRingOnly
        )

        g = QgsGeometry.fromWkt(
            "Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))"
        )
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(
            self.image_check(
                "simpleline_exterioronly",
                "simpleline_exterioronly",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

        s3.symbolLayer(0).setRingFilter(
            QgsLineSymbolLayer.RenderRingFilter.InteriorRingsOnly
        )
        g = QgsGeometry.fromWkt(
            "Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))"
        )
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(
            self.image_check(
                "simpleline_interioronly",
                "simpleline_interioronly",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testOpacityWithDataDefinedColor(self):
        line_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        line_layer = QgsVectorLayer(line_shp, "Lines", "ogr")
        self.assertTrue(line_layer.isValid())

        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )
        s.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeColor,
            QgsProperty.fromExpression("if(Name='Arterial', 'red', 'green')"),
        )

        s.setOpacity(0.5)

        line_layer.setRenderer(QgsSingleSymbolRenderer(s))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([line_layer])

        # Test rendering
        self.assertTrue(
            self.render_map_settings_check(
                "simpleline_opacityddcolor", "simpleline_opacityddcolor", ms
            )
        )

    def testDataDefinedOpacity(self):
        line_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        line_layer = QgsVectorLayer(line_shp, "Lines", "ogr")
        self.assertTrue(line_layer.isValid())

        s = QgsLineSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2"}
        )
        s.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeColor,
            QgsProperty.fromExpression("if(Name='Arterial', 'red', 'green')"),
        )

        s.setDataDefinedProperty(
            QgsSymbol.Property.PropertyOpacity,
            QgsProperty.fromExpression('if("Value" = 1, 25, 50)'),
        )

        line_layer.setRenderer(QgsSingleSymbolRenderer(s))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([line_layer])

        # Test rendering
        self.assertTrue(
            self.render_map_settings_check(
                "simpleline_ddopacity", "simpleline_ddopacity", ms
            )
        )

    def renderGeometry(self, symbol, geom):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = geom.get().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / 20.0)
        else:
            extent = extent.buffered(10)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            symbol.startRender(context)
            symbol.renderFeature(f, context)
            symbol.stopRender(context)
        finally:
            painter.end()

        return image


if __name__ == "__main__":
    unittest.main()
