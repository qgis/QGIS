"""QGIS Unit tests for QgsSymbolConverter classes

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import json
import unittest

from qgis.core import (
    Qgis,
    QgsApplication,
    QgsFillSymbol,
    QgsLineSymbol,
    QgsMarkerSymbol,
    QgsNotSupportedException,
    QgsReadWriteContext,
    QgsSymbolConverterContext,
)
from qgis.PyQt.QtGui import QColor
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsSymbolConverters(QgisTestCase):
    def test_qml_converter(self):
        """
        Test native QML XML symbol conversion.
        """
        converter = QgsApplication.symbolConverterRegistry().converter("qml")
        self.assertIsNotNone(converter)
        rw_context = QgsReadWriteContext()
        context = QgsSymbolConverterContext(rw_context)

        symbol = QgsMarkerSymbol.createSimple({"color": "255,30,10", "size": "4"})
        variant = converter.toVariant(symbol, context)

        self.assertEqual(
            variant[:58], "<!DOCTYPE qgis PUBLIC 'http://mrcc.com/qgis.dtd' 'SYSTEM'>"
        )
        self.assertIn("<symbol", variant)

        restored_symbol = converter.createSymbol(variant, context)
        self.assertIsInstance(restored_symbol, QgsMarkerSymbol)
        self.assertEqual(restored_symbol.color(), QColor(255, 30, 10))

        # invalid variants
        self.assertIsNone(converter.createSymbol(None, context))
        self.assertIsNone(converter.createSymbol("", context))

    def test_sld_converter(self):
        """
        Test SLD symbol conversion.

        This tests the converter class logic only -- the bulk of the SLD conversion
        tests live elsewhere
        """
        converter = QgsApplication.symbolConverterRegistry().converter("sld")
        self.assertIsNotNone(converter)
        rw_context = QgsReadWriteContext()
        context = QgsSymbolConverterContext(rw_context)

        symbol = QgsMarkerSymbol.createSimple({"color": "255,30,10", "size": "4"})
        variant = converter.toVariant(symbol, context)

        self.assertEqual(variant[:28], "<Rule>\n <se:PointSymbolizer>")
        self.assertIn("<se:PointSymbolizer", variant)

        context.setTypeHint(Qgis.SymbolType.Marker)
        restored_symbol = converter.createSymbol(variant, context)
        self.assertIsInstance(restored_symbol, QgsMarkerSymbol)
        self.assertEqual(restored_symbol.color(), QColor(255, 30, 10))

        # try with a full xml document
        xml = """<?xml version="1.0" encoding="UTF-8"?>
        <StyledLayerDescriptor xmlns="http://www.opengis.net/sld" xmlns:ogc="http://www.opengis.net/ogc" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.1.0" xmlns:xlink="http://www.w3.org/1999/xlink" xsi:schemaLocation="http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" xmlns:se="http://www.opengis.net/se">
        <UserStyle>
        <se:FeatureTypeStyle>
        <se:Rule>
        <se:PointSymbolizer>
        <se:Graphic><se:Mark><se:WellKnownName>circle</se:WellKnownName>\
        <se:Fill>
        <se:SvgParameter name="fill">#ff1e0a</se:SvgParameter></se:Fill>
        <se:Stroke>
        <se:SvgParameter name="stroke">#232323</se:SvgParameter>
        <se:SvgParameter name="stroke-width">0.5</se:SvgParameter>
        </se:Stroke></se:Mark>
        <se:Size>14</se:Size>
        </se:Graphic></se:PointSymbolizer>
        </se:Rule></se:FeatureTypeStyle></UserStyle></StyledLayerDescriptor>"""
        restored_symbol = converter.createSymbol(variant, context)
        self.assertIsInstance(restored_symbol, QgsMarkerSymbol)
        self.assertEqual(restored_symbol.color(), QColor(255, 30, 10))

        # invalid variants
        self.assertIsNone(converter.createSymbol(None, context))
        self.assertIsNone(converter.createSymbol("", context))

    def test_esri_rest_converter(self):
        """
        Test Esri REST JSON symbol conversion.

        This tests the converter class logic only -- the bulk of the actual conversion
        tests are in testqgsarcgisrestutils.cpp
        """
        converter = QgsApplication.symbolConverterRegistry().converter("esri_rest")
        self.assertIsNotNone(converter)
        rw_context = QgsReadWriteContext()
        context = QgsSymbolConverterContext(rw_context)

        # does not support writing
        with self.assertRaises(QgsNotSupportedException):
            symbol = QgsMarkerSymbol.createSimple({"color": "255,30,10", "size": "4"})
            converter.toVariant(symbol, context)

        esri_json = {
            "type": "esriSMS",
            "style": "esriSMSSquare",
            "color": [76, 115, 10, 200],
            "size": 8,
            "angle": 10,
            "xoffset": 7,
            "yoffset": 17,
            "outline": {"color": [152, 230, 17, 176], "width": 5},
        }

        restored_marker = converter.createSymbol(esri_json, context)

        self.assertIsNotNone(restored_marker)
        self.assertEqual(restored_marker.type(), Qgis.SymbolType.Marker)
        self.assertEqual(restored_marker.color(), QColor(76, 115, 10, 200))
        self.assertEqual(restored_marker.size(), 8.0)

        # should also support JSON string values:
        restored_marker = converter.createSymbol(json.dumps(esri_json), context)

        self.assertIsNotNone(restored_marker)
        self.assertEqual(restored_marker.type(), Qgis.SymbolType.Marker)
        self.assertEqual(restored_marker.color(), QColor(76, 115, 10, 200))
        self.assertEqual(restored_marker.size(), 8.0)

        # invalid variants
        self.assertIsNone(converter.createSymbol(None, context))
        self.assertIsNone(converter.createSymbol("", context))

    def test_ogr_converter(self):
        """
        Test OGR Style String converter.

        This tests the converter class logic only -- the bulk of the actual conversion
        tests are in testqgsogrutils.cpp
        """
        converter = QgsApplication.symbolConverterRegistry().converter("ogr")
        self.assertIsNotNone(converter)
        rw_context = QgsReadWriteContext()
        context = QgsSymbolConverterContext(rw_context)

        # line symbol
        context.setTypeHint(Qgis.SymbolType.Line)
        ogr_style = "PEN(c:#0000ff,w:1.5mm)"

        restored_symbol = converter.createSymbol(ogr_style, context)

        self.assertIsNotNone(restored_symbol)
        self.assertIsInstance(restored_symbol, QgsLineSymbol)

        self.assertEqual(restored_symbol.color().name().lower(), "#0000ff")

        # invalid variants
        self.assertIsNone(converter.createSymbol(None, context))
        self.assertIsNone(converter.createSymbol("", context))

    def test_mapboxgl_create_line_symbol(self):
        """
        Test parsing a MapBox GL line layer into a QgsLineSymbol.

        This tests the converter class logic only -- the bulk of the actual conversion
        tests are in test_qgsmapboxglconverter.py
        """
        converter = QgsApplication.symbolConverterRegistry().converter("mapboxgl")
        self.assertIsNotNone(converter)
        rw_context = QgsReadWriteContext()
        context = QgsSymbolConverterContext(rw_context)

        json_layer = {
            "id": "test-line",
            "type": "line",
            "paint": {"line-color": "#ff0000", "line-width": 3, "line-opacity": 0.5},
        }

        restored_symbol = converter.createSymbol(json_layer, context)
        self.assertIsInstance(restored_symbol, QgsLineSymbol)

        self.assertEqual(restored_symbol.color(), QColor(255, 0, 0, 255))
        self.assertAlmostEqual(restored_symbol.opacity(), 0.5, places=2)

    def test_mapboxgl_create_fill_symbol(self):
        """
        Test parsing a MapBox GL fill layer into a QgsFillSymbol.

        This tests the converter class logic only -- the bulk of the actual conversion
        tests are in test_qgsmapboxglconverter.py
        """
        converter = QgsApplication.symbolConverterRegistry().converter("mapboxgl")
        self.assertIsNotNone(converter)
        rw_context = QgsReadWriteContext()
        context = QgsSymbolConverterContext(rw_context)

        json_layer = {
            "id": "test-fill",
            "type": "fill",
            "paint": {"fill-color": "#00ff00", "fill-outline-color": "#0000ff"},
        }

        restored_symbol = converter.createSymbol(json_layer, context)
        self.assertIsInstance(restored_symbol, QgsFillSymbol)

        fill_layer = restored_symbol.symbolLayer(0)
        self.assertEqual(fill_layer.fillColor(), QColor(0, 255, 0))
        self.assertEqual(fill_layer.strokeColor(), QColor(0, 0, 255))

    def test_mapboxgl_create_circle_symbol(self):
        """
        Test parsing a MapBox GL circle layer into a QgsMarkerSymbol.

        This tests the converter class logic only -- the bulk of the actual conversion
        tests are in test_qgsmapboxglconverter.py
        """
        converter = QgsApplication.symbolConverterRegistry().converter("mapboxgl")
        self.assertIsNotNone(converter)
        rw_context = QgsReadWriteContext()
        context = QgsSymbolConverterContext(rw_context)

        json_layer = {
            "id": "test-circle",
            "type": "circle",
            "paint": {"circle-color": "#0000ff", "circle-radius": 5},
        }

        restored_symbol = converter.createSymbol(json_layer, context)
        self.assertIsInstance(restored_symbol, QgsMarkerSymbol)
        self.assertEqual(restored_symbol.color(), QColor(0, 0, 255))

    def test_mapboxgl_create_symbol_invalid(self):
        """
        Test parsing a invalid MapBox variant.
        """
        converter = QgsApplication.symbolConverterRegistry().converter("mapboxgl")
        self.assertIsNotNone(converter)
        rw_context = QgsReadWriteContext()
        context = QgsSymbolConverterContext(rw_context)

        self.assertIsNone(converter.createSymbol(None, context))
        self.assertIsNone(converter.createSymbol("", context))

        # missing type property
        self.assertIsNone(converter.createSymbol({"id": "broken"}, context))


if __name__ == "__main__":
    unittest.main()
