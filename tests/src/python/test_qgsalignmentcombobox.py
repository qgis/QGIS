"""QGIS Unit tests for QgsAlignmentComboBox

From build dir, run: ctest -R PyQgsAlignmentComboBox -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "26/06/2019"
__copyright__ = "Copyright 2019, The QGIS Project"

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtTest import QSignalSpy
from qgis.gui import QgsAlignmentComboBox
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsAlignmentComboBox(QgisTestCase):

    def testGettersSetters(self):
        """test widget getters/setters"""
        w = QgsAlignmentComboBox()
        w.setAvailableAlignments(
            Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignJustify
        )
        w.setCurrentAlignment(Qt.AlignmentFlag.AlignRight)
        self.assertEqual(w.currentAlignment(), Qt.AlignmentFlag.AlignRight)
        w.setCurrentAlignment(Qt.AlignmentFlag.AlignJustify)
        self.assertEqual(w.currentAlignment(), Qt.AlignmentFlag.AlignJustify)
        # not a choice
        w.setCurrentAlignment(Qt.AlignmentFlag.AlignLeft)
        self.assertEqual(w.currentAlignment(), Qt.AlignmentFlag.AlignJustify)

    def test_ChangedSignals(self):
        """test that signals are correctly emitted when setting alignment"""
        w = QgsAlignmentComboBox()

        spy = QSignalSpy(w.changed)
        w.setCurrentAlignment(Qt.AlignmentFlag.AlignRight)
        self.assertEqual(len(spy), 1)
        w.setCurrentAlignment(Qt.AlignmentFlag.AlignRight)
        self.assertEqual(len(spy), 1)
        w.setCurrentAlignment(Qt.AlignmentFlag.AlignLeft)
        self.assertEqual(len(spy), 2)
        w.setAvailableAlignments(
            Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignJustify
        )
        self.assertEqual(len(spy), 3)
        self.assertEqual(w.currentAlignment(), Qt.AlignmentFlag.AlignRight)
        w.setAvailableAlignments(
            Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignRight
        )
        self.assertEqual(len(spy), 3)


if __name__ == "__main__":
    unittest.main()
