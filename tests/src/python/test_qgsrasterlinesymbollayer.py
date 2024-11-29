"""
***************************************************************************
    test_qgsrasterlinesymbollayer.py
    ---------------------
    Date                 : March 2019
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

__author__ = "Nyall Dawson"
__date__ = "March 2019"
__copyright__ = "(C) 2019, Nyall Dawson"


from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    QgsFeature,
    QgsGeometry,
    QgsLineSymbol,
    QgsMapSettings,
    QgsRasterLineSymbolLayer,
    QgsRenderContext,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRasterLineSymbolLayer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_rasterline"

    def testRender(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + "/raster_brush.png")
        raster_line.setWidth(8)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("rasterline_render", "rasterline_render", rendered_image)
        )

    def testRenderHairline(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + "/raster_brush.png")
        raster_line.setWidth(0)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "rasterline_hairline", "rasterline_hairline", rendered_image
            )
        )

    def testRenderClosedRing(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + "/raster_brush.png")
        raster_line.setWidth(8)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10, 0 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("rasterline_closed", "rasterline_closed", rendered_image)
        )

    def testRenderOpacity(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + "/raster_brush.png")
        raster_line.setWidth(8)
        raster_line.setOpacity(0.5)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("rasterline_opacity", "rasterline_opacity", rendered_image)
        )

    def testRenderFlatCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + "/raster_brush.png")
        raster_line.setWidth(15)
        raster_line.setPenCapStyle(Qt.PenCapStyle.FlatCap)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("rasterline_flatcap", "rasterline_flatcap", rendered_image)
        )

    def testRenderSquareCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + "/raster_brush.png")
        raster_line.setWidth(15)
        raster_line.setPenCapStyle(Qt.PenCapStyle.SquareCap)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt("LineString(2 2, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "rasterline_squarecap", "rasterline_squarecap", rendered_image
            )
        )

    def testRenderMiterJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + "/raster_brush.png")
        raster_line.setWidth(15)
        raster_line.setPenJoinStyle(Qt.PenJoinStyle.MiterJoin)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt("LineString(0 15, 0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "rasterline_miterjoin", "rasterline_miterjoin", rendered_image
            )
        )

    def testRenderBevelJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + "/raster_brush.png")
        raster_line.setWidth(15)
        raster_line.setPenJoinStyle(Qt.PenJoinStyle.BevelJoin)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt("LineString(2 2, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "rasterline_beveljoin", "rasterline_beveljoin", rendered_image
            )
        )

    def testLineOffset(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        raster_line = QgsRasterLineSymbolLayer()
        raster_line.setPath(TEST_DATA_DIR + "/raster_brush.png")
        raster_line.setWidth(5)
        raster_line.setOffset(5)

        s.appendSymbolLayer(raster_line.clone())

        g = QgsGeometry.fromWkt("LineString(2 2, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("rasterline_offset", "rasterline_offset", rendered_image)
        )

    def renderGeometry(self, symbol, geom, buffer=20):
        f = QgsFeature()
        f.setGeometry(geom)

        image = QImage(200, 200, QImage.Format.Format_RGB32)

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


if __name__ == "__main__":
    unittest.main()
