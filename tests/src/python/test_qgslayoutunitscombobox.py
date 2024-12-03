"""QGIS Unit tests for QgsLayoutUnitsComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "18/07/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtWidgets import QDoubleSpinBox
from qgis.core import QgsLayoutMeasurementConverter, QgsUnitTypes
from qgis.gui import QgsLayoutUnitsComboBox
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLayoutUnitsComboBox(QgisTestCase):

    def testGettersSetters(self):
        """test widget getters/setters"""
        w = QgsLayoutUnitsComboBox()

        w.setUnit(QgsUnitTypes.LayoutUnit.LayoutPixels)
        self.assertEqual(w.unit(), QgsUnitTypes.LayoutUnit.LayoutPixels)

    def test_ChangedSignals(self):
        """test that signals are correctly emitted when setting unit"""
        w = QgsLayoutUnitsComboBox()

        spy = QSignalSpy(w.changed)
        w.setUnit(QgsUnitTypes.LayoutUnit.LayoutPixels)

        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0], QgsUnitTypes.LayoutUnit.LayoutPixels)

    def testLinkedWidgets(self):
        """test linking spin boxes to combobox"""
        w = QgsLayoutUnitsComboBox()
        self.assertFalse(w.converter())
        c = QgsLayoutMeasurementConverter()
        w.setConverter(c)
        self.assertEqual(w.converter(), c)

        spin = QDoubleSpinBox()
        spin.setMaximum(1000000)
        spin.setValue(100)
        w.setUnit(QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        w.linkToWidget(spin)
        w.setUnit(QgsUnitTypes.LayoutUnit.LayoutMeters)
        self.assertAlmostEqual(spin.value(), 1.0, 2)
        w.setUnit(QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        self.assertAlmostEqual(spin.value(), 1000.0, 2)

        spin2 = QDoubleSpinBox()
        spin2.setValue(50)
        spin2.setMaximum(1000000)
        w.linkToWidget(spin2)
        w.setUnit(QgsUnitTypes.LayoutUnit.LayoutCentimeters)
        self.assertAlmostEqual(spin.value(), 100.0, 2)
        self.assertAlmostEqual(spin2.value(), 5.0, 2)

        # no crash!
        del spin
        w.setUnit(QgsUnitTypes.LayoutUnit.LayoutMeters)
        self.assertAlmostEqual(spin2.value(), 0.05, 2)


if __name__ == "__main__":
    unittest.main()
