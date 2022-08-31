# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgsrandommarkersymbollayer.py
    ---------------------
    Date                 : October 2019
    Copyright            : (C) 2019 by Nyall Dawson
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
__date__ = 'October 2019'
__copyright__ = '(C) 2019, Nyall Dawson'

import qgis  # NOQA

import os
from utilities import unitTestDataPath

from qgis.PyQt.QtCore import QDir, Qt, QSize
from qgis.PyQt.QtGui import QImage, QColor, QPainter
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsGeometry,
                       QgsFillSymbol,
                       QgsRenderContext,
                       QgsFeature,
                       QgsMapSettings,
                       QgsMultiRenderChecker,
                       QgsReadWriteContext,
                       QgsSymbolLayerUtils,
                       QgsSimpleMarkerSymbolLayer,
                       QgsSimpleFillSymbolLayer,
                       QgsMarkerSymbol,
                       QgsRandomMarkerFillSymbolLayer,
                       QgsVectorLayer,
                       QgsSingleSymbolRenderer,
                       QgsProperty,
                       QgsSymbolLayer,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsSymbol
                       )

from qgis.testing import unittest, start_app

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRandomMarkerSymbolLayer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsRandomMarkerFillSymbolLayer Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def testSimple(self):
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        random_fill = QgsRandomMarkerFillSymbolLayer(10, seed=481523)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        random_fill.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(random_fill.clone())
        self.assertEqual(s.symbolLayer(0).pointCount(), 10)
        self.assertEqual(s.symbolLayer(0).seed(), 481523)
        s.symbolLayer(0).setPointCount(5)
        s.symbolLayer(0).setSeed(42)
        self.assertEqual(s.symbolLayer(0).pointCount(), 5)
        self.assertEqual(s.symbolLayer(0).seed(), 42)

        s2 = s.clone()
        self.assertEqual(s2.symbolLayer(0).pointCount(), 5)
        self.assertEqual(s2.symbolLayer(0).seed(), 42)

        doc = QDomDocument()
        context = QgsReadWriteContext()
        element = QgsSymbolLayerUtils.saveSymbol('test', s, doc, context)

        s2 = QgsSymbolLayerUtils.loadSymbol(element, context)
        self.assertEqual(s2.symbolLayer(0).pointCount(), 5)
        self.assertEqual(s2.symbolLayer(0).seed(), 42)

        # rendering test
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(random_fill.clone())

        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(self.imageCheck('randommarkerfill', 'randommarkerfill', rendered_image))

        s3.symbolLayer(0).setPointCount(3)
        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(self.imageCheck('randommarkerfill_3', 'randommarkerfill_3', rendered_image))

        s3.symbolLayer(0).setSeed(12783)
        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(self.imageCheck('randommarkerfill_seed', 'randommarkerfill_seed', rendered_image))

        # random seed
        s3.symbolLayer(0).setSeed(0)
        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        self.assertFalse(self.imageCheck('randommarkerfill_seed', 'randommarkerfill_seed', rendered_image, expect_fail=True))

        # density-based count
        s3.symbolLayer(0).setSeed(1)
        s3.symbolLayer(0).setCountMethod(QgsRandomMarkerFillSymbolLayer.DensityBasedCount)
        s3.symbolLayer(0).setPointCount(5)
        s3.symbolLayer(0).setDensityArea(250)  # 250 square millimeter
        g = QgsGeometry.fromWkt(
            'Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))')
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(
            self.imageCheck('randommarkerfill_densitybasedcount', 'randommarkerfill_densitybasedcount', rendered_image))

    def testCreate(self):
        random_fill = QgsRandomMarkerFillSymbolLayer(10, seed=5)
        self.assertEqual(random_fill.seed(), 5)
        fill2 = QgsRandomMarkerFillSymbolLayer.create(random_fill.properties())
        self.assertEqual(fill2.seed(), 5)

        random_fill = QgsRandomMarkerFillSymbolLayer(10)
        self.assertEqual(random_fill.seed(), 0)
        fill2 = QgsRandomMarkerFillSymbolLayer.create(random_fill.properties())
        self.assertEqual(fill2.seed(), 0)

        # a newly created random fill should default to a random seed, not 0
        random_fill = QgsRandomMarkerFillSymbolLayer.create({})
        self.assertNotEqual(random_fill.seed(), 0)

    def testMultipart(self):
        """
        Test rendering a multi-part random marker fill symbol
        """
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        simple_fill = QgsSimpleFillSymbolLayer(color=QColor(255, 255, 255))
        s.appendSymbolLayer(simple_fill.clone())

        random_fill = QgsRandomMarkerFillSymbolLayer(12, seed=481523)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        random_fill.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(random_fill.clone())

        g = QgsGeometry.fromWkt(
            'MultiPolygon(((0 0, 5 0, 5 10, 0 10, 0 0),(1 1, 1 9, 4 9, 4 1, 1 1)), ((6 0, 10 0, 10 5, 6 5, 6 0)), ((8 6, 10 6, 10 10, 8 10, 8 6)))')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('randommarkerfill_multipoly', 'randommarkerfill_multipoly', rendered_image))

    def testMultiLayer(self):
        """
        Test rendering a multi-layer symbol including a random marker fill symbol
        """
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        simple_fill = QgsSimpleFillSymbolLayer(color=QColor(255, 255, 255))
        s.appendSymbolLayer(simple_fill.clone())

        random_fill = QgsRandomMarkerFillSymbolLayer(12, seed=481523)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 4)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeStyle(Qt.NoPen)
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        random_fill.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(random_fill.clone())

        simple_fill = QgsSimpleFillSymbolLayer(color=QColor(100, 150, 200, 150))
        s.appendSymbolLayer(simple_fill.clone())

        g = QgsGeometry.fromWkt(
            'MultiPolygon(((0 0, 5 0, 5 10, 0 10, 0 0),(1 1, 1 9, 4 9, 4 1, 1 1)), ((6 0, 10 0, 10 5, 6 5, 6 0)), ((8 6, 10 6, 10 10, 8 10, 8 6)))')
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.imageCheck('randommarkerfill_multilayer', 'randommarkerfill_multilayer', rendered_image))

    def testOpacityWithDataDefinedColor(self):
        poly_shp = os.path.join(TEST_DATA_DIR, 'polys.shp')
        poly_layer = QgsVectorLayer(poly_shp, 'Polys', 'ogr')
        self.assertTrue(poly_layer.isValid())
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        random_fill = QgsRandomMarkerFillSymbolLayer(1, seed=481523)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Circle, 6)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeWidth(1)
        marker.setDataDefinedProperty(QgsSymbolLayer.PropertyFillColor, QgsProperty.fromExpression(
            "if(Name='Dam', 'red', 'green')"))
        marker.setDataDefinedProperty(QgsSymbolLayer.PropertyStrokeColor, QgsProperty.fromExpression(
            "if(Name='Dam', 'magenta', 'blue')"))
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_symbol.setOpacity(0.5)
        random_fill.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(random_fill.clone())

        s.setOpacity(0.5)

        poly_layer.setRenderer(QgsSingleSymbolRenderer(s))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([poly_layer])

        # Test rendering
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(ms)
        renderchecker.setControlPathPrefix('symbol_randommarkerfill')
        renderchecker.setControlName('expected_randommarker_opacityddcolor')
        res = renderchecker.runTest('expected_randommarker_opacityddcolor')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def testDataDefinedOpacity(self):
        poly_shp = os.path.join(TEST_DATA_DIR, 'polys.shp')
        poly_layer = QgsVectorLayer(poly_shp, 'Polys', 'ogr')
        self.assertTrue(poly_layer.isValid())
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        random_fill = QgsRandomMarkerFillSymbolLayer(1, seed=481523)
        marker = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Circle, 6)
        marker.setColor(QColor(255, 0, 0))
        marker.setStrokeWidth(1)
        marker.setDataDefinedProperty(QgsSymbolLayer.PropertyFillColor, QgsProperty.fromExpression(
            "if(Name='Dam', 'red', 'green')"))
        marker.setDataDefinedProperty(QgsSymbolLayer.PropertyStrokeColor, QgsProperty.fromExpression(
            "if(Name='Dam', 'magenta', 'blue')"))
        marker_symbol = QgsMarkerSymbol()
        marker_symbol.changeSymbolLayer(0, marker)
        marker_symbol.setOpacity(0.5)
        random_fill.setSubSymbol(marker_symbol)

        s.appendSymbolLayer(random_fill.clone())

        s.setDataDefinedProperty(QgsSymbol.PropertyOpacity, QgsProperty.fromExpression("if(\"Value\" >10, 25, 50)"))

        poly_layer.setRenderer(QgsSingleSymbolRenderer(s))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([poly_layer])

        # Test rendering
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(ms)
        renderchecker.setControlPathPrefix('symbol_randommarkerfill')
        renderchecker.setControlName('expected_randommarker_ddopacity')
        res = renderchecker.runTest('expected_randommarker_ddopacity')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def renderGeometry(self, symbol, geom, buffer=20):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format_RGB32)

        painter = QPainter()
        ms = QgsMapSettings()
        extent = geom.get().boundingBox()
        # buffer extent by 10%
        if extent.width() > 0:
            extent = extent.buffered((extent.height() + extent.width()) / buffer)
        else:
            extent = extent.buffered(buffer / 2)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI
        context.expressionContext().setFeature(f)

        painter.begin(image)
        try:
            image.fill(QColor(0, 0, 0))
            symbol.startRender(context)
            symbol.renderFeature(f, context)
            symbol.stopRender(context)
        finally:
            painter.end()

        return image

    def imageCheck(self, name, reference_image, image, expect_fail=False):
        self.report += "<h2>Render {}</h2>\n".format(name)
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsMultiRenderChecker()
        checker.setControlPathPrefix("symbol_randommarkerfill")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setExpectFail(expect_fail)
        checker.setColorTolerance(2)
        result = checker.runTest(name, 20)
        TestQgsRandomMarkerSymbolLayer.report += checker.report()
        return result


if __name__ == '__main__':
    unittest.main()
