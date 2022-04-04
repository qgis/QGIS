# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCheckableComboBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alexander Bruy'
__date__ = '22/03/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtTest import QSignalSpy

from qgis.gui import QgsCheckableComboBox
from qgis.testing import start_app, unittest

start_app()


class TestQgsCheckableComboBox(unittest.TestCase):

    def testGettersSetters(self):
        """ test widget getters/setters """
        w = qgis.gui.QgsCheckableComboBox()

        w.setSeparator('|')
        self.assertEqual(w.separator(), '|')
        w.setDefaultText('Select items...')
        self.assertEqual(w.defaultText(), 'Select items...')

        w.addItems(['One', 'Two', 'Three'])

        w.setCheckedItems(['Two'])
        self.assertEqual(len(w.checkedItems()), 1)
        self.assertEqual(w.checkedItems(), ['Two'])
        w.setCheckedItems(['Three'])
        self.assertEqual(len(w.checkedItems()), 2)
        self.assertEqual(w.checkedItems(), ['Two', 'Three'])

        w.setItemCheckState(2, Qt.Unchecked)
        self.assertEqual(w.itemCheckState(2), Qt.Unchecked)

    def test_ChangedSignals(self):
        """ test that signals are correctly emitted when clearing"""

        w = qgis.gui.QgsCheckableComboBox()

        w.addItems(['One', 'Two', 'Three'])

        checkedItemsChanged_spy = QSignalSpy(w.checkedItemsChanged)
        w.setCheckedItems(['Two'])

        self.assertEqual(len(checkedItemsChanged_spy), 1)

    def test_readonly(self):
        w = qgis.gui.QgsCheckableComboBox()
        w.setEditable(False)
        w.show()  # Should not crash


if __name__ == '__main__':
    unittest.main()
