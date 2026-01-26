"""QGIS Unit tests for QgsObjectCustomProperties

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "02/06/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtXml import QDomDocument
from qgis.core import QgsObjectCustomProperties
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsObjectCustomProperties(QgisTestCase):

    def testSimple(self):
        """test storing/retrieving properties"""
        props = QgsObjectCustomProperties()
        self.assertFalse(props.keys())
        self.assertFalse(props.contains("a"))
        self.assertFalse(props.value("a"))
        self.assertEqual(props.value("a", defaultValue=6), 6)

        # remove non-present key, no crash
        props.remove("a")

        props.setValue("a", 7)
        self.assertEqual(props.keys(), ["a"])
        self.assertTrue(props.contains("a"))
        self.assertFalse(props.contains("b"))
        self.assertEqual(props.value("a", defaultValue=6), 7)
        self.assertEqual(props.value("b", defaultValue="yy"), "yy")
        props.setValue("b", "xx")
        self.assertCountEqual(props.keys(), ["a", "b"])
        self.assertTrue(props.contains("a"))
        self.assertTrue(props.contains("b"))
        self.assertEqual(props.value("a", defaultValue=6), 7)
        self.assertEqual(props.value("b", defaultValue="yy"), "xx")

        props.remove("a")
        self.assertCountEqual(props.keys(), ["b"])
        self.assertFalse(props.contains("a"))
        self.assertTrue(props.contains("b"))
        self.assertEqual(props.value("a", defaultValue=6), 6)
        self.assertEqual(props.value("b", defaultValue="yy"), "xx")

        props.remove("b")
        self.assertFalse(props.keys())
        self.assertFalse(props.contains("a"))
        self.assertFalse(props.contains("b"))
        self.assertEqual(props.value("a", defaultValue=6), 6)
        self.assertEqual(props.value("b", defaultValue="yy"), "yy")

    def testSaveRestore(self):
        doc = QDomDocument()
        elem = doc.createElement("test")

        props = QgsObjectCustomProperties()
        props.setValue("a", "7")
        props.setValue("b", "xx")

        props.writeXml(elem, doc)

        props2 = QgsObjectCustomProperties()
        props2.readXml(elem)

        self.assertCountEqual(props2.keys(), ["a", "b"])
        self.assertTrue(props2.contains("a"))
        self.assertTrue(props2.contains("b"))
        self.assertEqual(props2.value("a", defaultValue=6), "7")
        self.assertEqual(props2.value("b", defaultValue="yy"), "xx")

    def testCompatibilityRestore(self):
        # for pre 3.20
        legacy_xml = '<test>\n <customproperties>\n  <property key="a" value="7"/>\n  <property key="b" value="xx"/>\n </customproperties>\n</test>\n'
        doc = QDomDocument()
        doc.setContent(legacy_xml)

        props = QgsObjectCustomProperties()
        props.readXml(doc.documentElement())

        self.assertCountEqual(props.keys(), ["a", "b"])
        self.assertTrue(props.contains("a"))
        self.assertTrue(props.contains("b"))
        self.assertEqual(props.value("a", defaultValue=6), "7")
        self.assertEqual(props.value("b", defaultValue="yy"), "xx")


if __name__ == "__main__":
    unittest.main()
