"""QGIS Unit tests for QgsFieldComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "20/07/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsField,
    QgsFieldModel,
    QgsFieldProxyModel,
    QgsFields,
    QgsVectorLayer,
)
from qgis.gui import QgsFieldComboBox
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


def create_layer():
    layer = QgsVectorLayer(
        "Point?field=fldtxt:string&field=fldint:integer&field=fldint2:integer",
        "addfeat",
        "memory",
    )
    assert layer.isValid()
    return layer


def create_model():
    l = create_layer()
    m = QgsFieldModel()
    m.setLayer(l)
    return l, m


class TestQgsFieldComboBox(QgisTestCase):

    def testGettersSetters(self):
        """test combobox getters/setters"""
        l = create_layer()
        w = QgsFieldComboBox()
        w.setLayer(l)
        self.assertEqual(w.layer(), l)

        w.setField("fldint")
        self.assertEqual(w.currentField(), "fldint")

        fields = QgsFields()
        fields.append(QgsField("test1", QVariant.String))
        fields.append(QgsField("test2", QVariant.String))
        w.setFields(fields)
        self.assertIsNone(w.layer())
        self.assertEqual(w.fields(), fields)

    def testFilter(self):
        """test setting field with filter"""
        l = create_layer()
        w = QgsFieldComboBox()
        w.setLayer(l)
        w.setFilters(QgsFieldProxyModel.Filter.Int)
        self.assertEqual(w.layer(), l)

        w.setField("fldint")
        self.assertEqual(w.currentField(), "fldint")

    def testSignals(self):
        l = create_layer()
        w = QgsFieldComboBox()
        w.setLayer(l)

        spy = QSignalSpy(w.fieldChanged)
        w.setField("fldint2")
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], "fldint2")
        w.setField("fldint2")
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], "fldint2")
        w.setField("fldint")
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], "fldint")
        w.setField(None)
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1][0], None)
        w.setField(None)
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1][0], None)

    def testManualFields(self):
        fields = QgsFields()
        fields.append(QgsField("test1", QVariant.String))
        fields.append(QgsField("test2", QVariant.String))
        w = QgsFieldComboBox()
        w.setFields(fields)
        self.assertEqual(w.count(), 2)
        self.assertEqual(w.itemText(0), "test1")
        self.assertEqual(w.itemText(1), "test2")


if __name__ == "__main__":
    unittest.main()
