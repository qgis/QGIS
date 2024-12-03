"""QGIS Unit tests for QgsFillSymbolLayers.

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
    QgsFeature,
    QgsFillSymbol,
    QgsGeometry,
    QgsMapSettings,
    QgsRenderContext,
    QgsSimpleLineSymbolLayer,
)
from qgis.testing import QgisTestCase, unittest


class TestQgsFillSymbolLayers(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_layer"

    def testSimpleLineWithOffset(self):
        """test that rendering a polygon with simple line symbol with offset results in closed line"""
        layer = QgsSimpleLineSymbolLayer()
        layer.setOffset(-1)
        layer.setColor(QColor(0, 0, 0))

        symbol = QgsFillSymbol()
        symbol.changeSymbolLayer(0, layer)

        image = QImage(200, 200, QImage.Format.Format_RGB32)
        painter = QPainter()
        ms = QgsMapSettings()

        geom = QgsGeometry.fromWkt("Polygon((0 0, 10 0, 10 10, 0 10, 0 0))")
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
                "fill_simpleline_offset",
                image,
                color_tolerance=2,
                allowed_mismatch=0,
            )
        )


if __name__ == "__main__":
    unittest.main()
