"""QGIS Unit tests for QgsSelectiveMaskSource.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis.core import Qgis, QgsSelectiveMaskSource
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSelectiveMaskSource(QgisTestCase):
    def test_constructors(self):
        # Default constructor
        source = QgsSelectiveMaskSource()
        self.assertFalse(source.isValid())
        self.assertFalse(source.layerId())
        self.assertFalse(source.sourceId())
        self.assertEqual(source.sourceType(), Qgis.SelectiveMaskSourceType.SymbolLayer)

        source = QgsSelectiveMaskSource(
            "layer_1", Qgis.SelectiveMaskSourceType.Label, "rule_1"
        )
        self.assertTrue(source.isValid())
        self.assertEqual(source.layerId(), "layer_1")
        self.assertEqual(source.sourceType(), Qgis.SelectiveMaskSourceType.Label)
        self.assertEqual(source.sourceId(), "rule_1")

    def test_getters_setters(self):
        source = QgsSelectiveMaskSource()

        source.setLayerId("layer_a")
        self.assertEqual(source.layerId(), "layer_a")
        self.assertTrue(source.isValid())

        source.setSourceId("rule_a")
        self.assertEqual(source.sourceId(), "rule_a")

        source.setSourceType(Qgis.SelectiveMaskSourceType.Label)
        self.assertEqual(source.sourceType(), Qgis.SelectiveMaskSourceType.Label)
        source.setSourceType(Qgis.SelectiveMaskSourceType.SymbolLayer)
        self.assertEqual(source.sourceType(), Qgis.SelectiveMaskSourceType.SymbolLayer)

    def test_equality(self):
        s1 = QgsSelectiveMaskSource(
            "layer_1", Qgis.SelectiveMaskSourceType.SymbolLayer, "id_1"
        )
        s2 = QgsSelectiveMaskSource(
            "layer_1", Qgis.SelectiveMaskSourceType.SymbolLayer, "id_1"
        )

        self.assertEqual(s1, s2)
        self.assertFalse(s1 != s2)

        s3 = QgsSelectiveMaskSource(
            "layer_2", Qgis.SelectiveMaskSourceType.SymbolLayer, "id_1"
        )
        self.assertNotEqual(s1, s3)
        self.assertTrue(s1 != s3)

        s4 = QgsSelectiveMaskSource(
            "layer_1", Qgis.SelectiveMaskSourceType.Label, "id_1"
        )
        self.assertNotEqual(s1, s4)
        self.assertTrue(s1 != s4)

        s5 = QgsSelectiveMaskSource(
            "layer_1", Qgis.SelectiveMaskSourceType.SymbolLayer, "id_2"
        )
        self.assertNotEqual(s1, s5)
        self.assertTrue(s1 != s5)

    def test_repr(self):
        source = QgsSelectiveMaskSource()
        self.assertEqual(repr(source), "<QgsSelectiveMaskSource: invalid>")

        source = QgsSelectiveMaskSource(
            "layer_1", Qgis.SelectiveMaskSourceType.SymbolLayer, "rule_1"
        )
        self.assertEqual(
            repr(source), "<QgsSelectiveMaskSource: layer_1 - rule_1 (SymbolLayer)>"
        )

        source = QgsSelectiveMaskSource(
            "layer_2", Qgis.SelectiveMaskSourceType.Label, "rule_2"
        )
        self.assertEqual(
            repr(source), "<QgsSelectiveMaskSource: layer_2 - rule_2 (Label)>"
        )

        source = QgsSelectiveMaskSource("layer_2", Qgis.SelectiveMaskSourceType.Label)
        self.assertEqual(repr(source), "<QgsSelectiveMaskSource: layer_2 (Label)>")


if __name__ == "__main__":
    unittest.main()
