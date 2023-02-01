"""
***************************************************************************
    test_qgssimplefillsymbollayer.py
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
__date__ = 'September 2020'
__copyright__ = '(C) 2020, Nyall Dawson'

import qgis  # NOQA
import os

from utilities import unitTestDataPath

from qgis.PyQt.QtCore import QDir, QPointF, Qt, QSize
from qgis.PyQt.QtGui import QImage, QColor, QPainter

from qgis.core import (QgsGeometry,
                       QgsFillSymbol,
                       QgsRenderContext,
                       QgsFeature,
                       QgsMapSettings,
                       QgsRenderChecker,
                       QgsVectorLayer,
                       QgsSimpleFillSymbolLayer,
                       QgsSymbolLayer,
                       QgsProperty,
                       QgsSingleSymbolRenderer,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsSymbol
                       )

from qgis.testing import unittest, start_app

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSimpleFillSymbolLayer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsSimpleFillSymbolLayer Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = f"{QDir.tempPath()}/qgistest.html"
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    def testRender(self):
        # rendering test
        s = QgsFillSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2', 'color': '#ff5588'})

        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 0))')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simplefill_render', 'simplefill_render', rendered_image)

    def testRenderWithOffset(self):
        # rendering test with offset
        s = QgsFillSymbol.createSimple({'outline_color': '#ff0000', 'outline_width': '2', 'color': '#ff5588'})
        s[0].setOffset(QPointF(5, 3))

        g = QgsGeometry.fromWkt('Polygon((0 0, 10 0, 10 10, 0 0))')
        rendered_image = self.renderGeometry(s, g)
        assert self.imageCheck('simplefill_offset', 'simplefill_offset', rendered_image)

    def testDataDefinedOffset(self):
        """ test that rendering a fill symbol with data defined offset works"""

        polys_shp = os.path.join(TEST_DATA_DIR, 'polys.shp')
        polys_layer = QgsVectorLayer(polys_shp, 'Polygons', 'ogr')

        # lets render two layers, to make comparison easier
        layer = QgsSimpleFillSymbolLayer()
        layer.setStrokeStyle(Qt.NoPen)
        layer.setColor(QColor(200, 250, 50))

        symbol = QgsFillSymbol()
        symbol.changeSymbolLayer(0, layer)

        layer = QgsSimpleFillSymbolLayer()
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyOffset, QgsProperty.fromExpression("array(-(x_min($geometry)+100)/5, (y_min($geometry)-35)/5)"))
        layer.setStrokeStyle(Qt.NoPen)
        layer.setColor(QColor(100, 150, 150))

        symbol.appendSymbolLayer(layer)

        polys_layer.setRenderer(QgsSingleSymbolRenderer(symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-133, 22, -70, 52))
        ms.setLayers([polys_layer])

        # Test rendering
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(ms)
        renderchecker.setControlPathPrefix('symbol_simplefill')
        renderchecker.setControlName('expected_simplefill_ddoffset')
        res = renderchecker.runTest('simplefill_ddoffset')
        TestQgsSimpleFillSymbolLayer.report += renderchecker.report()
        self.assertTrue(res)

    def testOpacityWithDataDefinedColor(self):
        poly_shp = os.path.join(TEST_DATA_DIR, 'polys.shp')
        poly_layer = QgsVectorLayer(poly_shp, 'Polys', 'ogr')
        self.assertTrue(poly_layer.isValid())

        layer = QgsSimpleFillSymbolLayer()
        layer.setStrokeStyle(Qt.NoPen)
        layer.setColor(QColor(200, 250, 50))
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyFillColor, QgsProperty.fromExpression(
            "if(Name='Dam', 'red', 'green')"))
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyStrokeColor, QgsProperty.fromExpression(
            "if(Name='Dam', 'magenta', 'blue')"))

        symbol = QgsFillSymbol()
        symbol.changeSymbolLayer(0, layer)

        symbol.setOpacity(0.5)

        poly_layer.setRenderer(QgsSingleSymbolRenderer(symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([poly_layer])

        # Test rendering
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(ms)
        renderchecker.setControlPathPrefix('symbol_simplefill')
        renderchecker.setControlName('expected_simplefill_opacityddcolor')
        res = renderchecker.runTest('expected_simplefill_opacityddcolor')
        self.report += renderchecker.report()
        self.assertTrue(res)

    def testDataDefinedOpacity(self):
        poly_shp = os.path.join(TEST_DATA_DIR, 'polys.shp')
        poly_layer = QgsVectorLayer(poly_shp, 'Polys', 'ogr')
        self.assertTrue(poly_layer.isValid())

        layer = QgsSimpleFillSymbolLayer()
        layer.setStrokeStyle(Qt.NoPen)
        layer.setColor(QColor(200, 250, 50))
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyFillColor, QgsProperty.fromExpression(
            "if(Name='Dam', 'red', 'green')"))
        layer.setDataDefinedProperty(QgsSymbolLayer.PropertyStrokeColor, QgsProperty.fromExpression(
            "if(Name='Dam', 'magenta', 'blue')"))

        symbol = QgsFillSymbol()
        symbol.changeSymbolLayer(0, layer)

        symbol.setDataDefinedProperty(QgsSymbol.PropertyOpacity, QgsProperty.fromExpression("if(\"Value\" >10, 25, 50)"))

        poly_layer.setRenderer(QgsSingleSymbolRenderer(symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([poly_layer])

        # Test rendering
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(ms)
        renderchecker.setControlPathPrefix('symbol_simplefill')
        renderchecker.setControlName('expected_simplefill_ddopacity')
        res = renderchecker.runTest('expected_simplefill_ddopacity')
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
        TestQgsSimpleFillSymbolLayer.report += f"<h2>Render {name}</h2>\n"
        temp_dir = QDir.tempPath() + '/'
        file_name = temp_dir + 'symbol_' + name + ".png"
        image.save(file_name, "PNG")
        checker = QgsRenderChecker()
        checker.setControlPathPrefix("symbol_simplefill")
        checker.setControlName("expected_" + reference_image)
        checker.setRenderedImage(file_name)
        checker.setColorTolerance(2)
        result = checker.compareImages(name, 20)
        TestQgsSimpleFillSymbolLayer.report += checker.report()
        print(TestQgsSimpleFillSymbolLayer.report)
        return result


if __name__ == '__main__':
    unittest.main()
