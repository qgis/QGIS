# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCrsSelectionWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '12/12/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsCoordinateReferenceSystem
from qgis.gui import (QgsCrsSelectionWidget)

from qgis.testing import start_app, unittest

start_app()


class TestQgsCrsSelectionWidget(unittest.TestCase):

    def testWidget(self):
        """
        Test widget logic
        """
        w = QgsCrsSelectionWidget()

        self.assertFalse(w.crs().isValid())
        self.assertFalse(w.hasValidSelection())

        spy = QSignalSpy(w.crsChanged)
        spy_valid_selection = QSignalSpy(w.hasValidSelectionChanged)
        c = QgsCoordinateReferenceSystem('EPSG:3111')

        w.setCrs(c)
        self.assertEqual(w.crs(), c)
        self.assertEqual(len(spy), 1)
        self.assertEqual(len(spy_valid_selection), 1)
        self.assertTrue(w.hasValidSelection())

        w.setCrs(QgsCoordinateReferenceSystem())
        # we aren't showing the no crs option yet!
        self.assertTrue(w.crs().isValid())
        self.assertEqual(len(spy), 2)
        self.assertEqual(len(spy_valid_selection), 2)
        self.assertFalse(w.hasValidSelection())

        w.setShowNoCrs(True)
        self.assertFalse(w.crs().isValid())
        self.assertEqual(len(spy), 3)
        self.assertEqual(len(spy_valid_selection), 3)
        self.assertTrue(w.hasValidSelection())

        w.setCrs(c)
        self.assertEqual(w.crs(), c)
        self.assertEqual(len(spy), 4)
        self.assertEqual(len(spy_valid_selection), 4)
        self.assertTrue(w.hasValidSelection())

        c2 = QgsCoordinateReferenceSystem('EPSG:3113')
        w.setCrs(c2)
        self.assertEqual(w.crs(), c2)
        self.assertEqual(len(spy), 5)
        self.assertEqual(len(spy_valid_selection), 5)
        self.assertTrue(w.hasValidSelection())

        w.setCrs(QgsCoordinateReferenceSystem())
        self.assertFalse(w.crs().isValid())
        self.assertEqual(len(spy), 6)
        self.assertEqual(len(spy_valid_selection), 6)
        self.assertTrue(w.hasValidSelection())


if __name__ == '__main__':
    unittest.main()

start_app()
