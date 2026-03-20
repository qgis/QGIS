"""QGIS Unit tests for QgsSymbolConverterRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import (
    Qgis,
    QgsAbstractSymbolConverter,
    QgsSymbolConverterContext,
    QgsSymbolConverterRegistry,
)
from qgis.PyQt import sip
from qgis.testing import QgisTestCase, start_app

start_app()


class TestConverter(QgsAbstractSymbolConverter):
    """
    Dummy converter for testing the registry.
    """

    def name(self):
        return "test"

    def formatName(self):
        return "TesT Format"

    def capabilities(self):
        return Qgis.SymbolConverterCapability.ReadSymbols

    def toVariant(self, symbol, context):
        return None

    def createSymbol(self, variant, context):
        return None


class TestQgsSymbolConverterRegistry(QgisTestCase):
    def testRegistry(self):
        registry = QgsSymbolConverterRegistry()

        self.assertIsNone(registry.converter("bad"))
        self.assertNotIn("bad", registry.converterNames())

        converter = TestConverter()
        self.assertTrue(registry.addConverter(converter))
        self.assertIn("test", registry.converterNames())

        # adding a converter with an existing name should return False and be rejected
        self.assertFalse(registry.addConverter(TestConverter()))

        retrieved_converter = registry.converter("test")
        self.assertTrue(isinstance(retrieved_converter, TestConverter))
        self.assertEqual(retrieved_converter.name(), "test")
        self.assertEqual(retrieved_converter.formatName(), "TesT Format")

        self.assertTrue(registry.removeConverter("test"))
        self.assertNotIn("test", registry.converterNames())
        self.assertIsNone(registry.converter("test"))
        self.assertTrue(sip.isdeleted(converter))

        self.assertFalse(registry.removeConverter("test"))


if __name__ == "__main__":
    unittest.main()
