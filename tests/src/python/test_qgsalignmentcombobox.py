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

import unittest

from qgis.core import Qgis
from qgis.gui import QgsAlignmentComboBox
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import QgisTestCase, start_app

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

    def testQgisHorizontalEnumGettersSetters(self):
        """
        test widget getters for Qgis horizontal alignment enum
        """
        w = QgsAlignmentComboBox()
        w.setAvailableAlignments(
            Qt.AlignmentFlag.AlignRight
            | Qt.AlignmentFlag.AlignJustify
            | Qt.AlignmentFlag.AlignLeft
            | Qt.AlignmentFlag.AlignHCenter
        )

        w.setCurrentAlignment(Qgis.TextHorizontalAlignment.Left)
        self.assertEqual(w.horizontalAlignment(), Qgis.TextHorizontalAlignment.Left)

        w.setCurrentAlignment(Qgis.TextHorizontalAlignment.Right)
        self.assertEqual(w.horizontalAlignment(), Qgis.TextHorizontalAlignment.Right)

        w.setCurrentAlignment(Qgis.TextHorizontalAlignment.Center)
        self.assertEqual(w.horizontalAlignment(), Qgis.TextHorizontalAlignment.Center)

        w.setCurrentAlignment(Qgis.TextHorizontalAlignment.Justify)
        self.assertEqual(w.horizontalAlignment(), Qgis.TextHorizontalAlignment.Justify)

    def testQgisVerticalEnumGettersSetters(self):
        """
        test widget getters for Qgis vertical alignment enum
        """
        w = QgsAlignmentComboBox()
        w.setAvailableAlignments(
            Qt.AlignmentFlag.AlignTop
            | Qt.AlignmentFlag.AlignVCenter
            | Qt.AlignmentFlag.AlignBottom
        )

        w.setCurrentAlignment(Qgis.TextVerticalAlignment.Top)
        self.assertEqual(w.verticalAlignment(), Qgis.TextVerticalAlignment.Top)

        w.setCurrentAlignment(Qgis.TextVerticalAlignment.VerticalCenter)
        self.assertEqual(
            w.verticalAlignment(), Qgis.TextVerticalAlignment.VerticalCenter
        )

        w.setCurrentAlignment(Qgis.TextVerticalAlignment.Bottom)
        self.assertEqual(w.verticalAlignment(), Qgis.TextVerticalAlignment.Bottom)


if __name__ == "__main__":
    unittest.main()
