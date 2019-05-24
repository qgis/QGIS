# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutUnitsComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsUnitTypes, QgsLayoutMeasurementConverter
from qgis.gui import QgsLayoutUnitsComboBox

from qgis.PyQt.QtWidgets import QDoubleSpinBox

from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import start_app, unittest

start_app()


class TestQgsLayoutUnitsComboBox(unittest.TestCase):

    def testGettersSetters(self):
        """ test widget getters/setters """
        w = qgis.gui.QgsLayoutUnitsComboBox()

        w.setUnit(QgsUnitTypes.LayoutPixels)
        self.assertEqual(w.unit(), QgsUnitTypes.LayoutPixels)

    def test_ChangedSignals(self):
        """ test that signals are correctly emitted when setting unit"""
        w = qgis.gui.QgsLayoutUnitsComboBox()

        spy = QSignalSpy(w.changed)
        w.setUnit(QgsUnitTypes.LayoutPixels)

        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0], QgsUnitTypes.LayoutPixels)

    def testLinkedWidgets(self):
        """ test linking spin boxes to combobox"""
        w = qgis.gui.QgsLayoutUnitsComboBox()
        self.assertFalse(w.converter())
        c = QgsLayoutMeasurementConverter()
        w.setConverter(c)
        self.assertEqual(w.converter(), c)

        spin = QDoubleSpinBox()
        spin.setMaximum(1000000)
        spin.setValue(100)
        w.setUnit(QgsUnitTypes.LayoutCentimeters)
        w.linkToWidget(spin)
        w.setUnit(QgsUnitTypes.LayoutMeters)
        self.assertAlmostEqual(spin.value(), 1.0, 2)
        w.setUnit(QgsUnitTypes.LayoutMillimeters)
        self.assertAlmostEqual(spin.value(), 1000.0, 2)

        spin2 = QDoubleSpinBox()
        spin2.setValue(50)
        spin2.setMaximum(1000000)
        w.linkToWidget(spin2)
        w.setUnit(QgsUnitTypes.LayoutCentimeters)
        self.assertAlmostEqual(spin.value(), 100.0, 2)
        self.assertAlmostEqual(spin2.value(), 5.0, 2)

        # no crash!
        del spin
        w.setUnit(QgsUnitTypes.LayoutMeters)
        self.assertAlmostEqual(spin2.value(), 0.05, 2)


if __name__ == '__main__':
    unittest.main()
