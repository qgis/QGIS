"""QGIS Unit tests for QgsSymbolConverter classes

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import (
    Qgis,
    QgsApplication,
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

    def test_esri_rest_converter(self):
        """
        Test Esri REST JSON symbol conversion.

        Note that the bulk of these tests are in testqgsarcgisrestutils.cpp
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


if __name__ == "__main__":
    unittest.main()
