"""
***************************************************************************
    test_qgsarrowsymbollayer.py
    ---------------------
    Date                 : March 2016
    Copyright            : (C) 2016 by Hugo Mercier
    Email                : hugo dot mercier at oslandia dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Hugo Mercier"
__date__ = "March 2016"
__copyright__ = "(C) 2016, Hugo Mercier"

import os

from qgis.PyQt.QtCore import QSize
from qgis.PyQt.QtGui import QColor, QImage, QPainter
from qgis.core import (
    QgsArrowSymbolLayer,
    QgsFeature,
    QgsFillSymbol,
    QgsGeometry,
    QgsLineSymbol,
    QgsMapSettings,
    QgsProject,
    QgsProperty,
    QgsRectangle,
    QgsRenderContext,
    QgsSingleSymbolRenderer,
    QgsSymbol,
    QgsSymbolLayer,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.testing.mocked import get_iface

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsArrowSymbolLayer(QgisTestCase):

    def setUp(self):
        self.iface = get_iface()

        lines_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        self.lines_layer = QgsVectorLayer(lines_shp, "Lines", "ogr")
        QgsProject.instance().addMapLayer(self.lines_layer)

        # Create style
        sym2 = QgsLineSymbol.createSimple({"color": "#fdbf6f"})
        self.lines_layer.setRenderer(QgsSingleSymbolRenderer(sym2))

        self.mapsettings = self.iface.mapCanvas().mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-113, 28, -91, 40))
        self.mapsettings.setBackgroundColor(QColor("white"))

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()

    def test_1(self):
        sym = self.lines_layer.renderer().symbol()
        sym_layer = QgsArrowSymbolLayer.create(
            {"head_length": "6.5", "head_thickness": "6.5"}
        )
        dd = QgsProperty.fromExpression("(@geometry_point_num % 4) * 2")
        sym_layer.setDataDefinedProperty(QgsSymbolLayer.Property.PropertyArrowWidth, dd)
        dd2 = QgsProperty.fromExpression("(@geometry_point_num % 4) * 2")
        sym_layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyArrowHeadLength, dd2
        )
        dd3 = QgsProperty.fromExpression("(@geometry_point_num % 4) * 2")
        sym_layer.setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyArrowHeadThickness, dd3
        )
        fill_sym = QgsFillSymbol.createSimple(
            {
                "color": "#8bcfff",
                "outline_color": "#000000",
                "outline_style": "solid",
                "outline_width": "1",
            }
        )
        sym_layer.setSubSymbol(fill_sym)
        sym.changeSymbolLayer(0, sym_layer)

        rendered_layers = [self.lines_layer]
        self.mapsettings.setLayers(rendered_layers)

        self.assertTrue(
            self.render_map_settings_check(
                "arrowsymbollayer_1", "arrowsymbollayer_1", self.mapsettings
            )
        )

    def test_2(self):
        sym = self.lines_layer.renderer().symbol()
        # double headed
        sym_layer = QgsArrowSymbolLayer.create(
            {
                "arrow_width": "5",
                "head_length": "4",
                "head_thickness": "6",
                "head_type": "2",
            }
        )
        fill_sym = QgsFillSymbol.createSimple(
            {
                "color": "#8bcfff",
                "outline_color": "#000000",
                "outline_style": "solid",
                "outline_width": "1",
            }
        )
        sym_layer.setSubSymbol(fill_sym)
        sym.changeSymbolLayer(0, sym_layer)

        rendered_layers = [self.lines_layer]
        self.mapsettings.setLayers(rendered_layers)

        self.assertTrue(
            self.render_map_settings_check(
                "arrowsymbollayer_2", "arrowsymbollayer_2", self.mapsettings
            )
        )

    def test_3(self):
        sym = self.lines_layer.renderer().symbol()
        # double headed
        sym_layer = QgsArrowSymbolLayer.create(
            {
                "arrow_width": "7",
                "head_length": "6",
                "head_thickness": "8",
                "head_type": "0",
                "arrow_type": "1",
                "is_curved": "0",
            }
        )
        fill_sym = QgsFillSymbol.createSimple(
            {
                "color": "#8bcfff",
                "outline_color": "#000000",
                "outline_style": "solid",
                "outline_width": "1",
            }
        )
        sym_layer.setSubSymbol(fill_sym)
        sym.changeSymbolLayer(0, sym_layer)

        rendered_layers = [self.lines_layer]
        self.mapsettings.setLayers(rendered_layers)

        ms = self.mapsettings
        ms.setExtent(QgsRectangle(-101, 35, -99, 37))
        self.assertTrue(
            self.render_map_settings_check(
                "arrowsymbollayer_3", "arrowsymbollayer_3", ms
            )
        )

    def test_unrepeated(self):
        sym = self.lines_layer.renderer().symbol()
        # double headed
        sym_layer = QgsArrowSymbolLayer.create(
            {
                "arrow_width": "7",
                "head_length": "6",
                "head_thickness": "8",
                "head_type": "0",
                "arrow_type": "0",
            }
        )
        # no repetition
        sym_layer.setIsRepeated(False)
        fill_sym = QgsFillSymbol.createSimple(
            {
                "color": "#8bcfff",
                "outline_color": "#000000",
                "outline_style": "solid",
                "outline_width": "1",
            }
        )
        sym_layer.setSubSymbol(fill_sym)
        sym.changeSymbolLayer(0, sym_layer)

        rendered_layers = [self.lines_layer]
        self.mapsettings.setLayers(rendered_layers)

        ms = self.mapsettings
        ms.setExtent(QgsRectangle(-119, 17, -82, 50))

        self.assertTrue(
            self.render_map_settings_check(
                "arrowsymbollayer_4", "arrowsymbollayer_4", ms
            )
        )

    def testColors(self):
        """
        Test colors, need to make sure colors are passed/retrieved from subsymbol
        """
        sym_layer = QgsArrowSymbolLayer.create()
        sym_layer.setColor(QColor(150, 50, 100))
        self.assertEqual(sym_layer.color(), QColor(150, 50, 100))
        self.assertEqual(sym_layer.subSymbol().color(), QColor(150, 50, 100))
        sym_layer.subSymbol().setColor(QColor(250, 150, 200))
        self.assertEqual(sym_layer.subSymbol().color(), QColor(250, 150, 200))
        self.assertEqual(sym_layer.color(), QColor(250, 150, 200))

    def testRingNumberVariable(self):
        # test test geometry_ring_num variable
        s3 = QgsFillSymbol()
        s3.deleteSymbolLayer(0)
        s3.appendSymbolLayer(QgsArrowSymbolLayer())
        s3.symbolLayer(0).setIsCurved(False)
        s3.symbolLayer(0).subSymbol()[0].setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyFillColor,
            QgsProperty.fromExpression(
                "case when @geometry_ring_num=0 then 'green' when @geometry_ring_num=1 then 'blue' when @geometry_ring_num=2 then 'red' end"
            ),
        )

        g = QgsGeometry.fromWkt(
            "Polygon((0 0, 10 0, 10 10, 0 10, 0 0),(1 1, 1 2, 2 2, 2 1, 1 1),(8 8, 9 8, 9 9, 8 9, 8 8))"
        )
        rendered_image = self.renderGeometry(s3, g)
        self.assertTrue(
            self.image_check(
                "arrow_ring_num",
                "arrow_ring_num",
                rendered_image,
                control_path_prefix="symbol_arrow",
            )
        )

    def testOpacityWithDataDefinedColor(self):
        line_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        line_layer = QgsVectorLayer(line_shp, "Lines", "ogr")
        self.assertTrue(line_layer.isValid())

        sym = QgsLineSymbol()
        sym_layer = QgsArrowSymbolLayer.create(
            {
                "arrow_width": "7",
                "head_length": "6",
                "head_thickness": "8",
                "head_type": "0",
                "arrow_type": "0",
                "is_repeated": "0",
                "is_curved": "0",
            }
        )
        fill_sym = QgsFillSymbol.createSimple(
            {
                "color": "#8bcfff",
                "outline_color": "#000000",
                "outline_style": "solid",
                "outline_width": "1",
            }
        )
        fill_sym.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyFillColor,
            QgsProperty.fromExpression("if(Name='Arterial', 'red', 'green')"),
        )
        fill_sym.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeColor,
            QgsProperty.fromExpression("if(Name='Arterial', 'magenta', 'blue')"),
        )

        sym_layer.setSubSymbol(fill_sym)
        sym.changeSymbolLayer(0, sym_layer)

        # set opacity on both the symbol and subsymbol, to test that they get combined
        fill_sym.setOpacity(0.5)
        sym.setOpacity(0.5)

        line_layer.setRenderer(QgsSingleSymbolRenderer(sym))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([line_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "arrow_opacityddcolor",
                "arrow_opacityddcolor",
                ms,
                control_path_prefix="symbol_arrow",
            )
        )

    def testDataDefinedOpacity(self):
        line_shp = os.path.join(TEST_DATA_DIR, "lines.shp")
        line_layer = QgsVectorLayer(line_shp, "Lines", "ogr")
        self.assertTrue(line_layer.isValid())

        sym = QgsLineSymbol()
        sym_layer = QgsArrowSymbolLayer.create(
            {
                "arrow_width": "7",
                "head_length": "6",
                "head_thickness": "8",
                "head_type": "0",
                "arrow_type": "0",
                "is_repeated": "0",
                "is_curved": "0",
            }
        )
        fill_sym = QgsFillSymbol.createSimple(
            {
                "color": "#8bcfff",
                "outline_color": "#000000",
                "outline_style": "solid",
                "outline_width": "1",
            }
        )
        fill_sym.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyFillColor,
            QgsProperty.fromExpression("if(Name='Arterial', 'red', 'green')"),
        )
        fill_sym.symbolLayer(0).setDataDefinedProperty(
            QgsSymbolLayer.Property.PropertyStrokeColor,
            QgsProperty.fromExpression("if(Name='Arterial', 'magenta', 'blue')"),
        )

        sym_layer.setSubSymbol(fill_sym)
        sym.changeSymbolLayer(0, sym_layer)

        sym.setDataDefinedProperty(
            QgsSymbol.Property.PropertyOpacity,
            QgsProperty.fromExpression('if("Value" = 1, 25, 50)'),
        )

        line_layer.setRenderer(QgsSingleSymbolRenderer(sym))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-118.5, 19.0, -81.4, 50.4))
        ms.setLayers([line_layer])

        self.assertTrue(
            self.render_map_settings_check(
                "arrow_ddopacity",
                "arrow_ddopacity",
                ms,
                control_path_prefix="symbol_arrow",
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
