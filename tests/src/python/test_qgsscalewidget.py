"""QGIS Unit tests for QgsScaleWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "13/03/2019"
__copyright__ = "Copyright 2019, The QGIS Project"

import math

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtWidgets import QComboBox

from qgis.gui import QgsScaleWidget, QgsScaleComboBox
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsScaleWidget(QgisTestCase):

    def testBasic(self):
        w = QgsScaleWidget()
        spy = QSignalSpy(w.scaleChanged)
        w.setScaleString("1:2345")
        self.assertEqual(w.scaleString(), "1:2,345")
        self.assertEqual(w.scale(), 2345)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], 2345)

        w.setScaleString("0.02")
        self.assertEqual(w.scaleString(), "1:50")
        self.assertEqual(w.scale(), 50)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], 50)

        w.setScaleString("1:4,000")
        self.assertEqual(w.scaleString(), "1:4,000")
        self.assertEqual(w.scale(), 4000)
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1][0], 4000)

    def test_predefined_scales(self):
        w = QgsScaleWidget()
        combo = w.findChild(QComboBox)

        w.updateScales(["1:500", "1:100"])
        self.assertEqual(combo.count(), 2)
        self.assertEqual(combo.itemText(0), "1:500")
        self.assertEqual(combo.itemText(1), "1:100")

        w.setScale(100)
        self.assertEqual(w.scale(), 100)
        self.assertEqual(combo.currentText(), "1:100")

        w.setScale(500)
        self.assertEqual(w.scale(), 500)
        self.assertEqual(combo.currentText(), "1:500")

        w.setPredefinedScales([10.0, 20.0, 30.0, 500.0])
        self.assertEqual(combo.count(), 4)
        self.assertEqual(combo.itemText(0), "1:10")
        self.assertEqual(combo.itemText(1), "1:20")
        self.assertEqual(combo.itemText(2), "1:30")
        self.assertEqual(combo.itemText(3), "1:500")
        self.assertEqual(w.scale(), 500)
        self.assertEqual(combo.currentText(), "1:500")

    def testNull(self):
        w = QgsScaleWidget()

        w.setScale(50)
        self.assertFalse(w.allowNull())
        w.setNull()  # no effect
        self.assertEqual(w.scale(), 50.0)
        self.assertFalse(w.isNull())

        spy = QSignalSpy(w.scaleChanged)
        w.setAllowNull(True)
        self.assertTrue(w.allowNull())

        w.setScaleString("")
        self.assertEqual(len(spy), 1)
        self.assertTrue(math.isnan(w.scale()))
        self.assertTrue(math.isnan(spy[-1][0]))
        self.assertTrue(w.isNull())
        w.setScaleString("    ")
        self.assertTrue(math.isnan(w.scale()))
        self.assertTrue(w.isNull())

        w.setScaleString("0.02")
        self.assertEqual(w.scale(), 50.0)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], 50.0)
        self.assertFalse(w.isNull())

        w.setScaleString("")
        self.assertTrue(math.isnan(w.scale()))
        self.assertEqual(len(spy), 3)
        self.assertTrue(math.isnan(spy[-1][0]))
        self.assertTrue(w.isNull())

        w.setScaleString("0.02")
        self.assertEqual(w.scale(), 50.0)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1][0], 50.0)
        self.assertFalse(w.isNull())
        w.setNull()
        self.assertTrue(math.isnan(w.scale()))
        self.assertEqual(len(spy), 5)
        self.assertTrue(math.isnan(spy[-1][0]))
        self.assertTrue(w.isNull())

        w.setAllowNull(False)
        self.assertFalse(w.allowNull())

    def test_combo(self):
        w = QgsScaleComboBox()
        w.updateScales(["1:500", "1:100"])
        self.assertEqual(w.count(), 2)
        self.assertEqual(w.itemText(0), "1:500")
        self.assertEqual(w.itemText(1), "1:100")

        w.setScale(100)
        self.assertEqual(w.scale(), 100)
        self.assertEqual(w.currentText(), "1:100")

        w.setScale(500)
        self.assertEqual(w.scale(), 500)
        self.assertEqual(w.currentText(), "1:500")

        w.setPredefinedScales([10.0, 20.0, 30.0, 500.0])
        self.assertEqual(w.count(), 4)
        self.assertEqual(w.itemText(0), "1:10")
        self.assertEqual(w.itemText(1), "1:20")
        self.assertEqual(w.itemText(2), "1:30")
        self.assertEqual(w.itemText(3), "1:500")
        self.assertEqual(w.scale(), 500)
        self.assertEqual(w.currentText(), "1:500")


if __name__ == "__main__":
    unittest.main()
