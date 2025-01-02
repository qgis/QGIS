"""
***************************************************************************
    test_qgslineburstsymbollayer.py
    ---------------------
    Date                 : October 2021
    Copyright            : (C) 2021 by Nyall Dawson
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
__date__ = "October 2021"
__copyright__ = "(C) 2021, Nyall Dawson"

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    Qgis,
    QgsFeature,
    QgsGeometry,
    QgsGradientColorRamp,
    QgsGradientStop,
    QgsLineburstSymbolLayer,
    QgsLineSymbol,
    QgsMapSettings,
    QgsProperty,
    QgsRenderContext,
    QgsSymbolLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLineburstSymbolLayer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_lineburst"

    def testTwoColor(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "lineburst_two_color",
                "lineburst_two_color",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testDataDefinedColors(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)
        line.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeColor,
            QgsProperty.fromExpression("'orange'"),
        )
        line.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertySecondaryColor,
            QgsProperty.fromExpression("'purple'"),
        )

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "lineburst_datadefined_color",
                "lineburst_datadefined_color",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testColorRamp(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setGradientColorType(Qgis.GradientColorSource.ColorRamp)
        line.setColorRamp(
            QgsGradientColorRamp(
                QColor(200, 0, 0),
                QColor(0, 200, 0),
                False,
                [QgsGradientStop(0.5, QColor(0, 0, 200))],
            )
        )
        line.setWidth(8)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "lineburst_colorramp",
                "lineburst_colorramp",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testRenderClosedRing(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 0, 10 10, 0 10, 0 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "lineburst_closed",
                "lineburst_closed",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testRenderFlatCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)
        line.setPenCapStyle(Qt.PenCapStyle.FlatCap)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "lineburst_flatcap",
                "lineburst_flatcap",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testRenderSquareCap(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)
        line.setPenCapStyle(Qt.PenCapStyle.SquareCap)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(2 2, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "lineburst_squarecap",
                "lineburst_squarecap",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testRenderMiterJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)
        line.setPenJoinStyle(Qt.PenJoinStyle.MiterJoin)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(0 15, 0 0, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "lineburst_miterjoin",
                "lineburst_miterjoin",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testRenderBevelJoin(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(8)
        line.setPenJoinStyle(Qt.PenJoinStyle.BevelJoin)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(2 2, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "lineburst_beveljoin",
                "lineburst_beveljoin",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
            )
        )

    def testLineOffset(self):
        s = QgsLineSymbol()
        s.deleteSymbolLayer(0)

        line = QgsLineburstSymbolLayer()
        line.setColor(QColor(255, 0, 0))
        line.setColor2(QColor(0, 255, 0))
        line.setWidth(5)
        line.setOffset(5)

        s.appendSymbolLayer(line.clone())

        g = QgsGeometry.fromWkt("LineString(2 2, 10 10, 10 0)")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check(
                "lineburst_offset",
                "lineburst_offset",
                rendered_image,
                color_tolerance=2,
                allowed_mismatch=20,
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
