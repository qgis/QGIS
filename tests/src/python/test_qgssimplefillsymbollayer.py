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

__author__ = "Nyall Dawson"
__date__ = "September 2020"
__copyright__ = "(C) 2020, Nyall Dawson"

import os

from qgis.PyQt.QtCore import QDir, QPointF, QSize, Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    QgsFeature,
    QgsFillSymbol,
    QgsGeometry,
    QgsMapSettings,
    QgsProperty,
    QgsRectangle,
    QgsRenderContext,
    QgsSimpleFillSymbolLayer,
    QgsSingleSymbolRenderer,
    QgsSymbol,
    QgsSymbolLayer,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSimpleFillSymbolLayer(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "symbol_simplefill"

    def testRender(self):
        # rendering test
        s = QgsFillSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2", "color": "#ff5588"}
        )

        g = QgsGeometry.fromWkt("Polygon((0 0, 10 0, 10 10, 0 0))")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("simplefill_render", "simplefill_render", rendered_image)
        )

    def testRenderWithOffset(self):
        # rendering test with offset
        s = QgsFillSymbol.createSimple(
            {"outline_color": "#ff0000", "outline_width": "2", "color": "#ff5588"}
        )
        s[0].setOffset(QPointF(5, 3))

        g = QgsGeometry.fromWkt("Polygon((0 0, 10 0, 10 10, 0 0))")
        rendered_image = self.renderGeometry(s, g)
        self.assertTrue(
            self.image_check("simplefill_offset", "simplefill_offset", rendered_image)
        )

    def testDataDefinedOffset(self):
        """test that rendering a fill symbol with data defined offset works"""

        polys_shp = os.path.join(TEST_DATA_DIR, "polys.shp")
        polys_layer = QgsVectorLayer(polys_shp, "Polygons", "ogr")

        # lets render two layers, to make comparison easier
        layer = QgsSimpleFillSymbolLayer()
        layer.setStrokeStyle(Qt.PenStyle.NoPen)
        layer.setColor(QColor(200, 250, 50))

        symbol = QgsFillSymbol()
        symbol.changeSymbolLayer(0, layer)

        layer = QgsSimpleFillSymbolLayer()
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyOffset,
            QgsProperty.fromExpression(
                "array(-(x_min($geometry)+100)/5, (y_min($geometry)-35)/5)"
            ),
        )
        layer.setStrokeStyle(Qt.PenStyle.NoPen)
        layer.setColor(QColor(100, 150, 150))

        symbol.appendSymbolLayer(layer)

        polys_layer.setRenderer(QgsSingleSymbolRenderer(symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-133, 22, -70, 52))
        ms.setLayers([polys_layer])

        # Test rendering
        self.assertTrue(
            self.render_map_settings_check(
                "simplefill_ddoffset", "simplefill_ddoffset", ms
            )
        )

    def testOpacityWithDataDefinedColor(self):
        poly_shp = os.path.join(TEST_DATA_DIR, "polys.shp")
        poly_layer = QgsVectorLayer(poly_shp, "Polys", "ogr")
        self.assertTrue(poly_layer.isValid())

        layer = QgsSimpleFillSymbolLayer()
        layer.setStrokeStyle(Qt.PenStyle.NoPen)
        layer.setColor(QColor(200, 250, 50))
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyFillColor,
            QgsProperty.fromExpression("if(Name='Dam', 'red', 'green')"),
        )
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeColor,
            QgsProperty.fromExpression("if(Name='Dam', 'magenta', 'blue')"),
        )

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
        self.assertTrue(
            self.render_map_settings_check(
                "simplefill_opacityddcolor", "simplefill_opacityddcolor", ms
            )
        )

    def testDataDefinedOpacity(self):
        poly_shp = os.path.join(TEST_DATA_DIR, "polys.shp")
        poly_layer = QgsVectorLayer(poly_shp, "Polys", "ogr")
        self.assertTrue(poly_layer.isValid())

        layer = QgsSimpleFillSymbolLayer()
        layer.setStrokeStyle(Qt.PenStyle.NoPen)
        layer.setColor(QColor(200, 250, 50))
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyFillColor,
            QgsProperty.fromExpression("if(Name='Dam', 'red', 'green')"),
        )
        layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeColor,
            QgsProperty.fromExpression("if(Name='Dam', 'magenta', 'blue')"),
        )

        symbol = QgsFillSymbol()
        symbol.changeSymbolLayer(0, layer)

        symbol.setDataDefinedProperty(
            QgsSymbol.Property.PropertyOpacity,
            QgsProperty.fromExpression('if("Value" >10, 25, 50)'),
        )

        poly_layer.setRenderer(QgsSingleSymbolRenderer(symbol))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([poly_layer])

        # Test rendering
        self.assertTrue(
            self.render_map_settings_check(
                "simplefill_ddopacity", "simplefill_ddopacity", ms
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
