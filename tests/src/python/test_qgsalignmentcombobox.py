# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAlignmentComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '26/06/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import Qt
from qgis.gui import QgsAlignmentComboBox

from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import start_app, unittest

start_app()


class TestQgsAlignmentComboBox(unittest.TestCase):

    def testGettersSetters(self):
        """ test widget getters/setters """
        w = QgsAlignmentComboBox()
        w.setAvailableAlignments(Qt.AlignRight | Qt.AlignJustify)
        w.setCurrentAlignment(Qt.AlignRight)
        self.assertEqual(w.currentAlignment(), Qt.AlignRight)
        w.setCurrentAlignment(Qt.AlignJustify)
        self.assertEqual(w.currentAlignment(), Qt.AlignJustify)
        # not a choice
        w.setCurrentAlignment(Qt.AlignLeft)
        self.assertEqual(w.currentAlignment(), Qt.AlignJustify)

    def test_ChangedSignals(self):
        """ test that signals are correctly emitted when setting alignment"""
        w = QgsAlignmentComboBox()

        spy = QSignalSpy(w.changed)
        w.setCurrentAlignment(Qt.AlignRight)
        self.assertEqual(len(spy), 1)
        w.setCurrentAlignment(Qt.AlignRight)
        self.assertEqual(len(spy), 1)
        w.setCurrentAlignment(Qt.AlignLeft)
        self.assertEqual(len(spy), 2)
        w.setAvailableAlignments(Qt.AlignRight | Qt.AlignJustify)
        self.assertEqual(len(spy), 3)
        self.assertEqual(w.currentAlignment(), Qt.AlignRight)
        w.setAvailableAlignments(Qt.AlignLeft | Qt.AlignRight)
        self.assertEqual(len(spy), 3)


if __name__ == '__main__':
    unittest.main()
