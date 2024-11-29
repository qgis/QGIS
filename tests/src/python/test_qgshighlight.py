"""QGIS Unit tests for QgsHighlight.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Matthias Kuhn"
__date__ = "8.11.2017"
__copyright__ = "Copyright 2017, The QGIS Project"

import os
import unittest
from typing import Optional

from qgis.PyQt.QtCore import QSize, Qt
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    QgsMapLayer,
    QgsCoordinateReferenceSystem,
    QgsFillSymbol,
    QgsGeometryGeneratorSymbolLayer,
    QgsRectangle,
    QgsSingleSymbolRenderer,
    QgsSymbol,
    QgsVectorLayer,
)
from qgis.gui import QgsHighlight, QgsMapCanvas
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsHighlight(QgisTestCase):
    @classmethod
    def control_path_prefix(cls):
        return "highlight"

    def run_test_for_layer(
        self,
        layer: QgsMapLayer,
        testname: str,
        use_feature: bool = False,
        highlight_color: Optional[QColor] = None,
    ):
        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(layer.crs())
        canvas.setFrameStyle(0)
        canvas.resize(400, 400)
        self.assertEqual(canvas.width(), 400)
        self.assertEqual(canvas.height(), 400)

        canvas.setExtent(layer.extent())
        canvas.show()

        # need to wait until first redraw can occur (note that we first need to wait till drawing starts!)
        app.processEvents()
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()

        feature = next(layer.getFeatures())
        geom = feature.geometry()

        if use_feature:
            highlight = QgsHighlight(canvas, feature, layer)
        else:
            highlight = QgsHighlight(canvas, geom, layer)

        if highlight_color:
            highlight.setColor(highlight_color)
            highlight.setFillColor(highlight_color)
        else:
            color = QColor(Qt.GlobalColor.red)
            highlight.setColor(color)
            color.setAlpha(50)
            highlight.setFillColor(color)

        highlight.setWidth(2)
        highlight.show()

        image = QImage(QSize(400, 400), QImage.Format.Format_ARGB32)
        image.fill(Qt.GlobalColor.white)
        painter = QPainter()
        painter.begin(image)
        canvas.render(painter)
        painter.end()

        return self.image_check(f"highlight_{testname}", f"highlight_{testname}", image)

    def testLine(self):
        lines_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        layer = QgsVectorLayer(lines_shp, "test", "ogr")
        self.assertTrue(layer.isValid())
        self.assertTrue(self.run_test_for_layer(layer, "lines"))

    def testPolygon(self):
        polys_shp = os.path.join(TEST_DATA_DIR, "polys.shp")
        layer = QgsVectorLayer(polys_shp, "test", "ogr")
        self.assertTrue(layer.isValid())
        self.assertTrue(self.run_test_for_layer(layer, "polygons"))

    def test_polygon_very_transparent(self):
        """
        Test that highlighting a layer with a very transparent style works
        """
        polys_shp = os.path.join(TEST_DATA_DIR, "polys.shp")
        layer = QgsVectorLayer(polys_shp, "test", "ogr")

        symbol = QgsFillSymbol.createSimple(
            {"color": "255,255,0", "outline_style": "no"}
        )
        symbol.setOpacity(0.3)
        layer.setRenderer(QgsSingleSymbolRenderer(symbol))
        self.assertTrue(layer.isValid())
        self.assertTrue(self.run_test_for_layer(layer, "polygons", use_feature=False))
        self.assertTrue(
            self.run_test_for_layer(
                layer,
                "very_transparent",
                use_feature=True,
                highlight_color=QColor(255, 0, 0, 128),
            )
        )

    def testBugfix48471(self):
        """Test scenario of https://github.com/qgis/QGIS/issues/48471"""

        lines_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        layer = QgsVectorLayer(lines_shp, "Layer", "ogr")
        self.assertTrue(layer.isValid())

        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(layer.crs())
        canvas.setFrameStyle(0)
        canvas.resize(400, 400)
        self.assertEqual(canvas.width(), 400)
        self.assertEqual(canvas.height(), 400)

        canvas.setLayers([layer])
        canvas.setExtent(layer.extent())
        canvas.show()

        # need to wait until first redraw can occur (note that we first need to wait till drawing starts!)
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()

        geom = next(layer.getFeatures()).geometry()

        highlight = QgsHighlight(canvas, geom, layer)
        highlight.setBuffer(12345)

        try:
            found = False
            for item in canvas.scene().items():
                if isinstance(item, QgsHighlight):
                    if item.buffer() == 12345:
                        found = True
            self.assertTrue(found)
        finally:
            canvas.scene().removeItem(highlight)

    def test_feature_transformation(self):
        poly_shp = os.path.join(TEST_DATA_DIR, "polys.shp")
        layer = QgsVectorLayer(poly_shp, "Layer", "ogr")

        sub_symbol = QgsFillSymbol.createSimple(
            {"color": "#8888ff", "outline_style": "no"}
        )

        sym = QgsFillSymbol()
        buffer_layer = QgsGeometryGeneratorSymbolLayer.create(
            {"geometryModifier": "buffer($geometry, -0.4)"}
        )
        buffer_layer.setSymbolType(QgsSymbol.SymbolType.Fill)
        buffer_layer.setSubSymbol(sub_symbol)
        sym.changeSymbolLayer(0, buffer_layer)
        layer.setRenderer(QgsSingleSymbolRenderer(sym))

        canvas = QgsMapCanvas()
        canvas.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        canvas.setFrameStyle(0)
        canvas.resize(600, 400)
        self.assertEqual(canvas.width(), 600)
        self.assertEqual(canvas.height(), 400)

        canvas.setLayers([layer])
        canvas.setExtent(QgsRectangle(-11960254, 4247568, -11072454, 4983088))
        canvas.show()

        # need to wait until first redraw can occur (note that we first need to wait till drawing starts!)
        while not canvas.isDrawing():
            app.processEvents()
        canvas.waitWhileRendering()

        feature = layer.getFeature(1)
        self.assertTrue(feature.isValid())

        highlight = QgsHighlight(canvas, feature, layer)
        color = QColor(Qt.GlobalColor.red)
        highlight.setColor(color)
        color.setAlpha(50)
        highlight.setFillColor(color)
        highlight.show()
        highlight.show()

        self.assertTrue(
            self.canvas_image_check(
                "highlight_transform", "highlight_transform", canvas
            )
        )

    def canvas_image_check(self, name, reference_image, canvas):
        image = QImage(canvas.size(), QImage.Format.Format_ARGB32)
        painter = QPainter(image)
        canvas.render(painter)
        painter.end()

        return self.image_check(name, reference_image, image)


if __name__ == "__main__":
    unittest.main()
