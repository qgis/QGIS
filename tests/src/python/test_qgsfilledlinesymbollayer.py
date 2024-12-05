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

from typing import Optional

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    QgsFeature,
    QgsGeometry,
    QgsLineSymbol,
    QgsMapSettings,
    QgsRenderContext,
    QgsFilledLineSymbolLayer,
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
