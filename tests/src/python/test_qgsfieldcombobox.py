# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsFieldComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '20/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsFields, QgsVectorLayer, QgsFieldProxyModel
from qgis.gui import QgsFieldComboBox
from qgis.PyQt.QtCore import QVariant, Qt

from qgis.testing import start_app, unittest

start_app()


def create_layer():
    layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=fldint2:integer",
                           "addfeat", "memory")
    assert layer.isValid()
    return layer


def create_model():
    l = create_layer()
    m = QgsFieldModel()
    m.setLayer(l)
    return l, m


class TestQgsFieldComboBox(unittest.TestCase):

    def testGettersSetters(self):
        """ test combobox getters/setters """
        l = create_layer()
        w = QgsFieldComboBox()
        w.setLayer(l)
        self.assertEqual(w.layer(), l)

        w.setField('fldint')
        self.assertEqual(w.currentField(), 'fldint')

    def testFilter(self):
        """ test setting field with filter """
        l = create_layer()
        w = QgsFieldComboBox()
        w.setLayer(l)
        w.setFilters(QgsFieldProxyModel.Int)
        self.assertEqual(w.layer(), l)

        w.setField('fldint')
        self.assertEqual(w.currentField(), 'fldint')


if __name__ == '__main__':
    unittest.main()
