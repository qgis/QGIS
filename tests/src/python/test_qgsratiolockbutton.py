# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRatioLockButton

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.gui import QgsRatioLockButton

from qgis.PyQt.QtWidgets import QDoubleSpinBox

from qgis.testing import start_app, unittest

start_app()


class TestQgsRatioLockButton(unittest.TestCase):

    def testLinkedWidgets(self):
        """ test linking spin boxes to combobox"""
        w = qgis.gui.QgsRatioLockButton()

        spin_width = QDoubleSpinBox()
        spin_width.setMaximum(100000)
        spin_height = QDoubleSpinBox()
        spin_height.setMaximum(100000)

        w.setWidthSpinBox(spin_width)
        spin_width.setValue(1000)
        self.assertEqual(spin_width.value(), 1000)

        w.setLocked(True)
        spin_width.setValue(2000)
        self.assertEqual(spin_width.value(), 2000)
        w.setLocked(False)

        w.setHeightSpinBox(spin_height)
        spin_width.setValue(1000)
        self.assertEqual(spin_width.value(), 1000)
        self.assertEqual(spin_height.value(), 0)

        w.setLocked(True)
        spin_width.setValue(2000)
        self.assertEqual(spin_width.value(), 2000)
        self.assertEqual(spin_height.value(), 0)

        spin_height.setValue(1000)
        self.assertEqual(spin_width.value(), 2000)
        self.assertEqual(spin_height.value(), 1000)

        # ok, that was all setup tests... let's check the real thing now
        spin_width.setValue(1000)
        self.assertEqual(spin_width.value(), 1000)
        self.assertEqual(spin_height.value(), 500)
        spin_height.setValue(1000)
        self.assertEqual(spin_width.value(), 2000)
        self.assertEqual(spin_height.value(), 1000)

        w.setLocked(False)
        spin_width.setValue(1000)
        self.assertEqual(spin_width.value(), 1000)
        self.assertEqual(spin_height.value(), 1000)
        spin_height.setValue(2000)
        self.assertEqual(spin_width.value(), 1000)
        self.assertEqual(spin_height.value(), 2000)

        w.setLocked(True)
        spin_height.setValue(1000)
        self.assertEqual(spin_width.value(), 500)
        self.assertEqual(spin_height.value(), 1000)

        # setting to 0 should "break" lock
        spin_height.setValue(0)
        self.assertEqual(spin_width.value(), 500)
        self.assertEqual(spin_height.value(), 0)
        spin_width.setValue(1000)
        self.assertEqual(spin_width.value(), 1000)
        self.assertEqual(spin_height.value(), 0)
        spin_height.setValue(100)
        self.assertEqual(spin_width.value(), 1000)
        self.assertEqual(spin_height.value(), 100)

        spin_width.setValue(0)
        self.assertEqual(spin_width.value(), 0)
        self.assertEqual(spin_height.value(), 100)
        spin_height.setValue(1000)
        self.assertEqual(spin_width.value(), 0)
        self.assertEqual(spin_height.value(), 1000)
        spin_width.setValue(200)
        self.assertEqual(spin_width.value(), 200)
        self.assertEqual(spin_height.value(), 1000)

    def testResetRatio(self):
        w = qgis.gui.QgsRatioLockButton()

        spin_width = QDoubleSpinBox()
        spin_width.setMaximum(100000)
        spin_height = QDoubleSpinBox()
        spin_height.setMaximum(100000)

        spin_width.setValue(1000)
        w.setWidthSpinBox(spin_width)
        spin_height.setValue(500)
        w.setHeightSpinBox(spin_height)

        w.setLocked(True)
        spin_width.setValue(2000)
        self.assertEqual(spin_height.value(), 1000)

        spin_width.blockSignals(True)
        spin_width.setValue(1000)
        spin_width.blockSignals(False)

        spin_height.setValue(2000)
        self.assertEqual(spin_width.value(), 4000)  # signals were blocked, so ratio wasn't updated

        spin_width.blockSignals(True)
        spin_width.setValue(2000)
        spin_width.blockSignals(False)
        w.resetRatio() # since signals were blocked, we need to manually reset ratio
        spin_height.setValue(1000)
        self.assertEqual(spin_width.value(), 1000)


if __name__ == '__main__':
    unittest.main()
