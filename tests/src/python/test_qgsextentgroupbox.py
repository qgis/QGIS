# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsExtentGroupBox

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '31/05/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import os

from qgis.core import QgsRectangle, QgsCoordinateReferenceSystem, QgsVectorLayer, QgsProject
from qgis.gui import QgsExtentGroupBox

from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsExtentGroupBox(unittest.TestCase):

    def testGettersSetters(self):
        """ test widget getters/setters """
        w = qgis.gui.QgsExtentGroupBox()

        w.setOriginalExtent(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('epsg:3111'))
        self.assertEqual(w.originalExtent(), QgsRectangle(1, 2, 3, 4))
        self.assertEqual(w.originalCrs().authid(), 'EPSG:3111')

        w.setCurrentExtent(QgsRectangle(11, 12, 13, 14), QgsCoordinateReferenceSystem('epsg:3113'))
        self.assertEqual(w.currentExtent(), QgsRectangle(11, 12, 13, 14))
        self.assertEqual(w.currentCrs().authid(), 'EPSG:3113')

        w.setTitleBase('abc')
        self.assertEqual(w.titleBase(), 'abc')

    def test_SettingExtent(self):
        w = qgis.gui.QgsExtentGroupBox()

        spy = QSignalSpy(w.extentChanged)

        w.setOriginalExtent(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('epsg:3111'))
        w.setCurrentExtent(QgsRectangle(11, 12, 13, 14), QgsCoordinateReferenceSystem('epsg:3113'))

        w.setOutputExtentFromOriginal()
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))
        self.assertEqual(w.extentState(), QgsExtentGroupBox.OriginalExtent)
        self.assertEqual(len(spy), 1)

        w.setOutputExtentFromCurrent()
        self.assertEqual(w.outputExtent(), QgsRectangle(11, 12, 13, 14))
        self.assertEqual(w.extentState(), QgsExtentGroupBox.CurrentExtent)
        self.assertEqual(len(spy), 2)

        w.setOutputExtentFromUser(QgsRectangle(21, 22, 23, 24), QgsCoordinateReferenceSystem('epsg:3111'))
        self.assertEqual(w.outputExtent(), QgsRectangle(21, 22, 23, 24))
        self.assertEqual(w.extentState(), QgsExtentGroupBox.UserExtent)
        self.assertEqual(len(spy), 3)

        shapefile = os.path.join(TEST_DATA_DIR, 'polys.shp')
        layer = QgsVectorLayer(shapefile, 'Polys', 'ogr')
        QgsProject.instance().addMapLayer(layer)

        w.setOutputExtentFromLayer(None)
        # no layer - should be unchanged
        self.assertEqual(len(spy), 3)
        self.assertEqual(w.outputExtent(), QgsRectangle(21, 22, 23, 24))
        self.assertEqual(w.extentState(), QgsExtentGroupBox.UserExtent)
        self.assertEqual(len(spy), 3)

        w.setOutputExtentFromLayer(layer)
        self.assertEqual(w.outputExtent().toString(4), QgsRectangle(-118.9229, 24.5079, -83.7900, 46.7262).toString(4))
        self.assertEqual(w.extentState(), QgsExtentGroupBox.ProjectLayerExtent)
        self.assertEqual(len(spy), 4)


if __name__ == '__main__':
    unittest.main()
