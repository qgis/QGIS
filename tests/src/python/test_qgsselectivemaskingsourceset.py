"""QGIS Unit tests for QgsSelectiveMaskingSourceSet.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtXml import QDomDocument, QDomElement
from qgis.core import (
    QgsSelectiveMaskSource,
    QgsSelectiveMaskingSourceSet,
    Qgis,
    QgsReadWriteContext,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSelectiveMaskingSourceSet(QgisTestCase):

    def test_getters_setters(self):
        mask_set = QgsSelectiveMaskingSourceSet()

        self.assertTrue(mask_set.id())
        mask_set.setId("xxx")
        self.assertEqual(mask_set.id(), "xxx")

        self.assertFalse(mask_set.name())
        mask_set.setName("Test Set")
        self.assertEqual(mask_set.name(), "Test Set")

        self.assertFalse(mask_set.sources())
        self.assertEqual(len(mask_set), 0)
        self.assertTrue(mask_set.isEmpty())

        s1 = QgsSelectiveMaskSource(
            "layer_1", Qgis.SelectiveMaskSourceType.SymbolLayer, "rule_1"
        )
        s2 = QgsSelectiveMaskSource(
            "layer_2", Qgis.SelectiveMaskSourceType.Label, "rule_2"
        )

        mask_set.setSources([s1, s2])
        self.assertEqual(mask_set.sources(), [s1, s2])
        self.assertEqual(len(mask_set), 2)
        self.assertFalse(mask_set.isEmpty())

    def test_append(self):
        mask_set = QgsSelectiveMaskingSourceSet()
        self.assertTrue(mask_set.isEmpty())
        mask_set.append(
            QgsSelectiveMaskSource("layer_1", Qgis.SelectiveMaskSourceType.Label)
        )
        self.assertEqual(len(mask_set), 1)
        self.assertEqual(mask_set.sources()[0].layerId(), "layer_1")
        mask_set.append(
            QgsSelectiveMaskSource("layer_2", Qgis.SelectiveMaskSourceType.Label)
        )
        self.assertEqual(len(mask_set), 2)
        self.assertEqual(mask_set.sources()[0].layerId(), "layer_1")
        self.assertEqual(mask_set.sources()[1].layerId(), "layer_2")

    def test_getitem(self):
        mask_set = QgsSelectiveMaskingSourceSet()
        s1 = QgsSelectiveMaskSource(
            "layer_1", Qgis.SelectiveMaskSourceType.SymbolLayer, "rule_1"
        )
        s2 = QgsSelectiveMaskSource(
            "layer_2", Qgis.SelectiveMaskSourceType.Label, "rule_2"
        )
        mask_set.setSources([s1, s2])

        self.assertEqual(mask_set[0], s1)
        self.assertEqual(mask_set[1], s2)
        self.assertEqual(mask_set[-1], s2)

        with self.assertRaises(IndexError):
            _ = mask_set[2]

        # iteration
        res = [i for i in mask_set]
        self.assertEqual(res, [s1, s2])

    def test_read_write_xml(self):
        mask_set = QgsSelectiveMaskingSourceSet()
        mask_set.setName("Test Set")

        s1 = QgsSelectiveMaskSource(
            "layer_a", Qgis.SelectiveMaskSourceType.SymbolLayer, "id_a"
        )
        s2 = QgsSelectiveMaskSource(
            "layer_b", Qgis.SelectiveMaskSourceType.Label, "id_b"
        )
        mask_set.setSources([s1, s2])

        # write to XML
        doc = QDomDocument("testdoc")
        context = QgsReadWriteContext()
        element = mask_set.writeXml(doc, context)

        # read from XML
        new_set = QgsSelectiveMaskingSourceSet()
        self.assertTrue(new_set.readXml(element, doc, context))

        self.assertEqual(new_set.id(), mask_set.id())
        self.assertEqual(new_set.name(), "Test Set")
        self.assertEqual(len(new_set), 2)
        restored_s1 = new_set[0]
        self.assertEqual(restored_s1.layerId(), "layer_a")
        self.assertEqual(
            restored_s1.sourceType(), Qgis.SelectiveMaskSourceType.SymbolLayer
        )
        self.assertEqual(restored_s1.sourceId(), "id_a")

        restored_s2 = new_set[1]
        self.assertEqual(restored_s2.layerId(), "layer_b")
        self.assertEqual(restored_s2.sourceType(), Qgis.SelectiveMaskSourceType.Label)
        self.assertEqual(restored_s2.sourceId(), "id_b")

    def test_read_invalid_xml(self):
        doc = QDomDocument()
        element = doc.createElement("InvalidTag")

        mask_set = QgsSelectiveMaskingSourceSet()
        context = QgsReadWriteContext()

        self.assertFalse(mask_set.readXml(element, doc, context))

    def test_repr(self):
        source_set = QgsSelectiveMaskingSourceSet()
        self.assertEqual(repr(source_set), "<QgsSelectiveMaskingSourceSet: invalid>")

        source_set.setId("id")
        source_set.setName("")
        self.assertEqual(repr(source_set), "<QgsSelectiveMaskingSourceSet: id>")

        source_set.setName("xxx")
        self.assertEqual(repr(source_set), "<QgsSelectiveMaskingSourceSet: id (xxx)>")

    def test_is_valid(self):
        source_set = QgsSelectiveMaskingSourceSet()
        self.assertFalse(source_set.isValid())

        source_set.setName("xxx")
        self.assertTrue(source_set.isValid())

        source_set = QgsSelectiveMaskingSourceSet()
        source_set.setSources([])
        self.assertTrue(source_set.isValid())


if __name__ == "__main__":
    unittest.main()
