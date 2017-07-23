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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import QgsUnitTypes
from qgis.gui import QgsLayoutUnitsComboBox

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


if __name__ == '__main__':
    unittest.main()
