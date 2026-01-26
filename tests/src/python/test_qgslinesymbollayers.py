"""QGIS Unit tests for QgsLineSymbolLayers.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2017-01"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    Qgis,
    QgsFeature,
    QgsGeometry,
    QgsLineSymbol,
    QgsMapSettings,
    QgsRenderContext,
    QgsSimpleLineSymbolLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLineSymbolLayers(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_layer"

    @unittest.skipIf(
        Qgis.geosVersionMajor() == 3 and Qgis.geosVersionMinor() >= 11,
        "Broken upstream due to https://github.com/libgeos/geos/issues/1037",
    )
    def testSimpleLineWithOffset(self):
        """test that rendering a simple line symbol with offset"""
        layer = QgsSimpleLineSymbolLayer(QColor(0, 0, 0))
        layer.setOffset(1)

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format.Format_RGB32)
        painter = QPainter()
        ms = QgsMapSettings()

        geom = QgsGeometry.fromWkt("LineString (0 0, 10 0, 10 10, 0 10, 0 0)")
        f = QgsFeature()
        f.setGeometry(geom)

        extent = geom.constGet().boundingBox()
        # buffer extent by 10%
        extent = extent.buffered((extent.height() + extent.width()) / 20.0)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        image.fill(QColor(255, 255, 255))

        symbol.startRender(context)
        symbol.renderFeature(f, context)
        symbol.stopRender(context)
        painter.end()

        self.assertTrue(
            self.image_check(
                "symbol_layer",
                "simpleline_offset",
                image,
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )

    def testSimpleLineWithCustomDashPattern(self):
        """test that rendering a simple line symbol with custom dash pattern"""
        layer = QgsSimpleLineSymbolLayer(QColor(0, 0, 0))
        layer.setWidth(0.5)
        layer.setCustomDashVector([2, 5])
        layer.setUseCustomDashPattern(True)

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format.Format_RGB32)
        painter = QPainter()
        ms = QgsMapSettings()

        geom = QgsGeometry.fromWkt("LineString (0 0, 10 0, 10 10, 0 10, 0 0)")
        f = QgsFeature()
        f.setGeometry(geom)

        extent = geom.constGet().boundingBox()
        # buffer extent by 10%
        extent = extent.buffered((extent.height() + extent.width()) / 20.0)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        image.fill(QColor(255, 255, 255))

        symbol.startRender(context)
        symbol.renderFeature(f, context)
        symbol.stopRender(context)
        painter.end()

        self.assertTrue(
            self.image_check(
                "simpleline_customdashpattern",
                "simpleline_customdashpattern",
                image,
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )

    def testSimpleLineWithCustomDashPatternHairline(self):
        """test that rendering a simple line symbol with custom dash pattern"""
        layer = QgsSimpleLineSymbolLayer(QColor(0, 0, 0))
        layer.setWidth(0)
        layer.setCustomDashVector([3, 3, 2, 2])
        layer.setUseCustomDashPattern(True)

        symbol = QgsLineSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format.Format_RGB32)
        painter = QPainter()
        ms = QgsMapSettings()

        geom = QgsGeometry.fromWkt("LineString (0 0, 10 0, 10 10, 0 10, 0 0)")
        f = QgsFeature()
        f.setGeometry(geom)

        extent = geom.constGet().boundingBox()
        # buffer extent by 10%
        extent = extent.buffered((extent.height() + extent.width()) / 20.0)

        ms.setExtent(extent)
        ms.setOutputSize(image.size())
        context = QgsRenderContext.fromMapSettings(ms)
        context.setPainter(painter)
        context.setScaleFactor(96 / 25.4)  # 96 DPI

        painter.begin(image)
        image.fill(QColor(255, 255, 255))

        symbol.startRender(context)
        symbol.renderFeature(f, context)
        symbol.stopRender(context)
        painter.end()

        self.assertTrue(
            self.image_check(
                "simpleline_customdashpattern_hairline",
                "simpleline_customdashpattern_hairline",
                image,
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )


if __name__ == "__main__":
    unittest.main()
