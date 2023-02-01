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

__author__ = 'Nyall Dawson'
__date__ = 'November 2018'
__copyright__ = '(C) 2018, Nyall Dawson'

import qgis  # NOQA

import os

from utilities import unitTestDataPath

from qgis.PyQt.QtCore import QDir, Qt, QSize
from qgis.PyQt.QtGui import QImage, QColor, QPainter
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsGeometry,
                       QgsRectangle,
                       QgsFillSymbol,
                       QgsRenderContext,
                       QgsFeature,
                       QgsMapSettings,
                       QgsRenderChecker,
                       QgsReadWriteContext,
                       QgsSymbolLayerUtils,
                       QgsSimpleLineSymbolLayer,
                       QgsLineSymbolLayer,
                       QgsLineSymbol,
                       QgsUnitTypes,
                       QgsMapUnitScale,
                       QgsVectorLayer,
                       QgsSymbolLayer,
                       QgsMultiRenderChecker,
                       QgsProperty,
                       QgsSingleSymbolRenderer,
                       QgsSymbol
                       )

from qgis.testing import unittest, start_app

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSimpleLineSymbolLayer(unittest.TestCase):

    def setUp(self):
        self.report = "<h1>Python QgsSimpleLineSymbolLayer Tests</h1>\n"

    def tearDown(self):
        report_file_path = f"{QDir.tempPath()}/qgistest.html"
        with open(report_file_path, 'a') as report_file:
            report_file.write(self.report)

    def testDashPatternWithDataDefinedWidth(self):
        # rendering test
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})

        s[0].setUseCustomDashPattern(True)
        s[0].setPenCapStyle(Qt.FlatCap)
        s[0].setCustomDashVector([3, 4, 5, 6])

        s[0].dataDefinedProperties().setProperty(QgsSymbolLayer.PropertyStrokeWidth, QgsProperty.fromExpression('3'))

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simpleline_dashpattern_datadefined_width', 'simpleline_dashpattern_datadefined_width', rendered_image)

    def testTrimDistance(self):
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '0.6'})

        s.symbolLayer(0).setTrimDistanceStart(1.2)
        s.symbolLayer(0).setTrimDistanceStartUnit(QgsUnitTypes.RenderPoints)
        s.symbolLayer(0).setTrimDistanceStartMapUnitScale(QgsMapUnitScale(5, 10))
        s.symbolLayer(0).setTrimDistanceEnd(3.2)
        s.symbolLayer(0).setTrimDistanceEndUnit(QgsUnitTypes.RenderPercentage)
        s.symbolLayer(0).setTrimDistanceEndMapUnitScale(QgsMapUnitScale(15, 20))

        s2 = s.clone()
        self.assertEqual(s2.symbolLayer(0).trimDistanceStart(), 1.2)
        self.assertEqual(s2.symbolLayer(0).trimDistanceStartUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s2.symbolLayer(0).trimDistanceStartMapUnitScale().minScale, 5)
        self.assertEqual(s2.symbolLayer(0).trimDistanceStartMapUnitScale().maxScale, 10)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEnd(), 3.2)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEndUnit(), QgsUnitTypes.RenderPercentage)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEndMapUnitScale().minScale, 15)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEndMapUnitScale().maxScale, 20)

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(s2.symbolLayer(0).trimDistanceStart(), 1.2)
        self.assertEqual(s2.symbolLayer(0).trimDistanceStartUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s2.symbolLayer(0).trimDistanceStartMapUnitScale().minScale, 5)
        self.assertEqual(s2.symbolLayer(0).trimDistanceStartMapUnitScale().maxScale, 10)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEnd(), 3.2)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEndUnit(), QgsUnitTypes.RenderPercentage)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEndMapUnitScale().minScale, 15)
        self.assertEqual(s2.symbolLayer(0).trimDistanceEndMapUnitScale().maxScale, 20)

    def testTrimDistanceRender(self):
        """
        Rendering test of trim distances
        """
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})

        s.symbolLayer(0).setTrimDistanceStart(150)
        s.symbolLayer(0).setTrimDistanceStartUnit(QgsUnitTypes.RenderPoints)
        s.symbolLayer(0).setTrimDistanceEnd(9)
        s.symbolLayer(0).setTrimDistanceEndUnit(QgsUnitTypes.RenderMillimeters)

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simpleline_trim_distance_units', 'simpleline_trim_distance_units', rendered_image)

    def testTrimDistanceRenderPercentage(self):
        """
        Rendering test of trim distances using percentage
        """
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})

        s.symbolLayer(0).setTrimDistanceStart(10)
        s.symbolLayer(0).setTrimDistanceStartUnit(QgsUnitTypes.RenderPercentage)
        s.symbolLayer(0).setTrimDistanceEnd(50)
        s.symbolLayer(0).setTrimDistanceEndUnit(QgsUnitTypes.RenderPercentage)

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simpleline_trim_distance_percentage', 'simpleline_trim_distance_percentage', rendered_image)

    def testTrimDistanceRenderDataDefined(self):
        """
        Rendering test of trim distances using data defined lengths
        """
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})

        s.symbolLayer(0).setTrimDistanceStart(1)
        s.symbolLayer(0).setTrimDistanceStartUnit(QgsUnitTypes.RenderPercentage)
        s.symbolLayer(0).setTrimDistanceEnd(5)
        s.symbolLayer(0).setTrimDistanceEndUnit(QgsUnitTypes.RenderPercentage)

        s.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertyTrimStart, QgsProperty.fromExpression('5*2'))
        s.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertyTrimEnd, QgsProperty.fromExpression('60-10'))

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simpleline_trim_distance_percentage', 'simpleline_trim_distance_percentage', rendered_image)

    def testDashPatternOffset(self):

        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '0.6'})

        s.symbolLayer(0).setDashPatternOffset(1.2)
        s.symbolLayer(0).setDashPatternOffsetUnit(QgsUnitTypes.RenderPoints)
        s.symbolLayer(0).setDashPatternOffsetMapUnitScale(QgsMapUnitScale(5, 10))

        s2 = s.clone()
        self.assertEqual(s2.symbolLayer(0).dashPatternOffset(), 1.2)
        self.assertEqual(s2.symbolLayer(0).dashPatternOffsetUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s2.symbolLayer(0).dashPatternOffsetMapUnitScale().minScale, 5)
        self.assertEqual(s2.symbolLayer(0).dashPatternOffsetMapUnitScale().maxScale, 10)

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(s2.symbolLayer(0).dashPatternOffset(), 1.2)
        self.assertEqual(s2.symbolLayer(0).dashPatternOffsetUnit(), QgsUnitTypes.RenderPoints)
        self.assertEqual(s2.symbolLayer(0).dashPatternOffsetMapUnitScale().minScale, 5)
        self.assertEqual(s2.symbolLayer(0).dashPatternOffsetMapUnitScale().maxScale, 10)

    def testDashPatternOffsetRender(self):
        # rendering test
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})

        s.symbolLayer(0).setPenStyle(Qt.DashDotDotLine)
        s.symbolLayer(0).setDashPatternOffset(10)
        s.symbolLayer(0).setDashPatternOffsetUnit(QgsUnitTypes.RenderPoints)

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simpleline_dashpattern_offset', 'simpleline_dashpattern_offset', rendered_image)

    def testDashPatternOffsetRenderNegative(self):
        # rendering test
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})

        s.symbolLayer(0).setPenStyle(Qt.DashDotDotLine)
        s.symbolLayer(0).setDashPatternOffset(-10)
        s.symbolLayer(0).setDashPatternOffsetUnit(QgsUnitTypes.RenderPoints)

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simpleline_dashpattern_offset_negative', 'simpleline_dashpattern_offset_negative', rendered_image)

    def testDashPatternOffsetRenderCustomPattern(self):
        # rendering test
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})

        s.symbolLayer(0).setUseCustomDashPattern(True)
        s.symbolLayer(0).setPenCapStyle(Qt.FlatCap)
        s.symbolLayer(0).setCustomDashVector([3, 4, 5, 6])
        s.symbolLayer(0).setDashPatternOffset(10)

        g = QgsGeometry.fromWkt('LineString(0 0, 10 0, 10 10, 0 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simpleline_dashpattern_offset_custom', 'simpleline_dashpattern_offset_custom', rendered_image)

    def testDashTweaks(self):
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '0.6'})

        self.assertFalse(s.symbolLayer(0).alignDashPattern())
        self.assertFalse(s.symbolLayer(0).tweakDashPatternOnCorners())

        s.symbolLayer(0).setAlignDashPattern(True)
        s.symbolLayer(0).setTweakDashPatternOnCorners(True)

        s2 = s.clone()
        self.assertTrue(s2.symbolLayer(0).alignDashPattern())
        self.assertTrue(s2.symbolLayer(0).tweakDashPatternOnCorners())

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertTrue(s2.symbolLayer(0).alignDashPattern())
        self.assertTrue(s2.symbolLayer(0).tweakDashPatternOnCorners())

    def testAlignDashRender(self):
        # rendering test
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})

        s.symbolLayer(0).setPenStyle(Qt.DashDotDotLine)
        s.symbolLayer(0).setAlignDashPattern(True)

        g = QgsGeometry.fromWkt('LineString(0 0, 9.2 0, 9.2 10, 1.3 10)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simpleline_aligndashpattern', 'simpleline_aligndashpattern', rendered_image)

    def testDashCornerTweakDashRender(self):
        # rendering test
        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})

        s.symbolLayer(0).setPenStyle(Qt.DashDotDotLine)
        s.symbolLayer(0).setAlignDashPattern(True)
        s.symbolLayer(0).setTweakDashPatternOnCorners(True)
        s.symbolLayer(0).setPenJoinStyle(Qt.RoundJoin)

        g = QgsGeometry.fromWkt('LineString(0 0, 2 1, 3 1, 10 0, 10 10, 5 5)')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simpleline_dashcornertweak', 'simpleline_dashcornertweak', rendered_image)

    def testRingNumberVariable(self):
        # test test geometry_ring_num variable
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(
            QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=2))
        s3.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertyStrokeColor,
                                                 QgsProperty.fromExpression('case when @geometry_ring_num=0 then \'green\' when @geometry_ring_num=1 then \'blue\' when @geometry_ring_num=2 then \'red\' end'))

        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('simpleline_ring_num', 'simpleline_ring_num', rendered_image)

    def testRingFilter(self):
        # test filtering rings during rendering

        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)
        s.appendSymbolLayer(
            QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=2))
        self.assertEqual(s.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.AllRings)
        s.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.ExteriorRingOnly)
        self.assertEqual(s.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.ExteriorRingOnly)

        s2 = s.clone()
        self.assertEqual(s2.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.ExteriorRingOnly)

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(s2.symbolLayer(0).ringFilter(), QgsLineSymbolLayer.ExteriorRingOnly)

        # rendering test
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(
            QgsSimpleLineSymbolLayer(color=QColor(255, 0, 0), width=2))
        s3.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.ExteriorRingOnly)

        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('simpleline_exterioronly', 'simpleline_exterioronly', rendered_image)

        s3.symbolLayer(0).setRingFilter(QgsLineSymbolLayer.InteriorRingsOnly)
        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        assert self.imageCheck('simpleline_interioronly', 'simpleline_interioronly', rendered_image)

    def testOpacityWithDataDefinedColor(self):
        line_shp = os.path.join(TEST_DATA_DIR, 'lines.shp')
        line_layer = QgsVectorLayer(line_shp, 'Lines', 'ogr')
        self.assertTrue(line_layer.isValid())

        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})
        s.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertyStrokeColor, QgsProperty.fromExpression(
            "if(Name='Arterial', 'red', 'green')"))

        s.setOpacity(0.5)

        line_layer.setRenderer(QgsSingleSymbolRenderer(s))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([line_layer])

        # Test rendering
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(ms)
        renderchecker.setControlPathPrefix('symbol_simpleline')
        renderchecker.setControlName('expected_simpleline_opacityddcolor')
        res = renderchecker.runTest('expected_simpleline_opacityddcolor')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def testDataDefinedOpacity(self):
        line_shp = os.path.join(TEST_DATA_DIR, 'lines.shp')
        line_layer = QgsVectorLayer(line_shp, 'Lines', 'ogr')
        self.assertTrue(line_layer.isValid())

        s = QgsLineSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2'})
        s.symbolLayer(0).setDataDefinedProperty(QgsSymbolLayer.PropertyStrokeColor, QgsProperty.fromExpression(
            "if(Name='Arterial', 'red', 'green')"))

        s.setDataDefinedProperty(QgsSymbol.PropertyOpacity, QgsProperty.fromExpression("if(\"Value\" = 1, 25, 50)"))

        line_layer.setRenderer(QgsSingleSymbolRenderer(s))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([line_layer])

        # Test rendering
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(ms)
        renderchecker.setControlPathPrefix('symbol_simpleline')
        renderchecker.setControlName('expected_simpleline_ddopacity')
        res = renderchecker.runTest('expected_simpleline_ddopacity')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def renderGeometry(self, symbol, geom):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

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

    def imageCheck(self, name, reference_image, image):
        self.report += f"<h2>Render {name}</h2>\n"
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol_simpleline")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        self.report += checker.report()
        print(self.report)
        return result


if __name__ == '__main__':
    unittest.main()
