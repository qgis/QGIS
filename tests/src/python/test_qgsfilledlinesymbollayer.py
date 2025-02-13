"""
***************************************************************************
    test_qgsfilledlinesymbollayer.py
    ---------------------
    Date                 : November 2023
    Copyright            : (C) 2023 by Nyall Dawson
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
__date__ = "November 2023"
__copyright__ = "(C) 2023, Nyall Dawson"

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    QgsFeature,
    QgsGeometry,
    QgsLineSymbol,
    QgsMapSettings,
    QgsRenderContext,
    QgsFilledLineSymbolLayer,
    QgsFillSymbol,
    QgsSimpleFillSymbolLayer,
    QgsMarkerSymbol,
    QgsSimpleMarkerSymbolLayer,
    QgsCentroidFillSymbolLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsFilledLineSymbolLayer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_filledline"

    def testRender(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsFilledLineSymbolLayer()
        # color should be passed to subsymbol
        line.setColor(QColor(255, 0, 0))
        line.setWidth(8)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check("render", "render", rendered_image))

    def testRenderFlatCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsFilledLineSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setWidth(8)
        line.setPenCapStyle(Qt.PenCapStyle.FlatCap)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("renderflatcap", "renderflatcap", rendered_image)
        )

    def testRenderMiterJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsFilledLineSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setWidth(8)
        line.setPenJoinStyle(Qt.PenJoinStyle.MiterJoin)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(0 15, 0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("render_miter", "render_miter", rendered_image)
        )

    def testRenderBevelJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsFilledLineSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setWidth(8)
        line.setPenJoinStyle(Qt.PenJoinStyle.BevelJoin)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(2 2, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("render_bevel", "render_bevel", rendered_image)
        )

    def testLineOffset(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsFilledLineSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setWidth(5)
        line.setOffset(5)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(2 2, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("render_offset", "render_offset", rendered_image)
        )

    def testLineOffsetPolygon(self):
        s = QgsFillSymbol()
        s.deleteSymbolLayer(0)

        line = QgsFilledLineSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setWidth(5)
        line.setOffset(5)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("Polygon((2 2, 10 10, 10 0, 2 2))")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "render_offset_polygon", "render_offset_polygon", rendered_image
            )
        )

    def testRenderLooped(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsFilledLineSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setWidth(8)
        line.setPenCapStyle(Qt.PenCapStyle.FlatCap)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt(
            "LineString (13.07373737373737299 8.51161616161616053, 0.69292929292929273 5.93585858585858439, 11.32222222222222285 2.15808080808080582, 0 0, 10 10, 10 0)"
        )
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(self.image_check("loop", "loop", rendered_image))

    def testCentroidFillSubSymbol(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsFilledLineSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setWidth(5)

        sub_symbol = QgsFillSymbol()
        simple_fill = QgsSimpleFillSymbolLayer()
        simple_fill.setColor(QColor(0, 255, 0, 100))
        simple_fill.setStrokeStyle(Qt.NoPen)
        centroid_fill = QgsCentroidFillSymbolLayer()
        simple_marker = QgsSimpleMarkerSymbolLayer()
        simple_marker.setFillColor(QColor(255, 0, 0))
        simple_marker.setStrokeStyle(Qt.NoPen)
        centroid_fill_marker = QgsMarkerSymbol()
        centroid_fill_marker.changeSymbolLayer(0, simple_marker)
        centroid_fill.setPointOnSurface(True)

        centroid_fill.setSubSymbol(centroid_fill_marker)
        sub_symbol.changeSymbolLayer(0, centroid_fill)
        sub_symbol.appendSymbolLayer(simple_fill)
        line.setSubSymbol(sub_symbol)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(2 2, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "render_centroid_fill", "render_centroid_fill", rendered_image
            )
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
