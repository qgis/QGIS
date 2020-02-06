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

import qgis  # NOQA
import os

from qgis.core import QgsRectangle, QgsCoordinateReferenceSystem, QgsVectorLayer, QgsProject, QgsFeature, QgsGeometry
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

        QgsProject.instance().removeAllMapLayers()

    def testSetOutputCrs(self):
        w = qgis.gui.QgsExtentGroupBox()
        w.setCheckable(True)

        # ensure setting output crs doesn't change state of group box
        w.setChecked(False)
        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:4326'))
        self.assertFalse(w.isChecked())
        w.setChecked(True)
        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:4326'))
        self.assertTrue(w.isChecked())

        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:4326'))
        w.setCurrentExtent(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('epsg:4326'))
        w.setOutputExtentFromCurrent()
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))

        # with reprojection
        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:3785'))
        self.assertEqual(w.outputExtent().toString(4), QgsRectangle(111319.4908, 222684.2085, 333958.4724, 445640.1097).toString(4))
        # change CRS back
        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:4326'))
        # extent should be back to current - not a reprojection of the reprojected bounds
        self.assertEqual(w.outputExtent().toString(20), QgsRectangle(1, 2, 3, 4).toString(20))

        # repeat, this time using original extents
        w = qgis.gui.QgsExtentGroupBox()

        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:4326'))
        w.setOriginalExtent(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('epsg:4326'))
        w.setOutputExtentFromOriginal()
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))

        # with reprojection
        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:3785'))
        self.assertEqual(w.outputExtent().toString(4),
                         QgsRectangle(111319.4908, 222684.2085, 333958.4724, 445640.1097).toString(4))
        # change CRS back
        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:4326'))
        # extent should be back to original - not a reprojection of the reprojected bounds
        self.assertEqual(w.outputExtent().toString(20), QgsRectangle(1, 2, 3, 4).toString(20))

        # repeat, this time using layer extent
        layer = QgsVectorLayer("Polygon?crs=epsg:4326", 'memory', 'memory')
        self.assertTrue(layer.isValid())
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('Polygon((1 2, 3 2, 3 4, 1 4, 1 2))'))
        layer.dataProvider().addFeatures([f])
        QgsProject.instance().addMapLayer(layer)
        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:4326'))
        w.setOutputExtentFromLayer(layer)
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))

        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:3785'))
        self.assertEqual(w.outputExtent().toString(4),
                         QgsRectangle(111319.4908, 222684.2085, 333958.4724, 445640.1097).toString(4))
        # change CRS back
        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:4326'))
        # extent should be back to original - not a reprojection of the reprojected bounds
        self.assertEqual(w.outputExtent().toString(20), QgsRectangle(1, 2, 3, 4).toString(20))

        # custom extent
        w = qgis.gui.QgsExtentGroupBox()

        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:4326'))
        w.setOutputExtentFromUser(QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem('epsg:4326'))
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))

        # with reprojection
        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:3785'))
        self.assertEqual(w.outputExtent().toString(4),
                         QgsRectangle(111319.4908, 222684.2085, 333958.4724, 445640.1097).toString(4))
        # change CRS back
        w.setOutputCrs(QgsCoordinateReferenceSystem('epsg:4326'))
        # in this case we can't retrieve the original user extent in 4326, so we have a reprojection of the reprojected bounds
        # just test this by restricting the test to 4 decimals
        self.assertEqual(w.outputExtent().toString(4), QgsRectangle(1, 2, 3, 4).toString(4))


if __name__ == '__main__':
    unittest.main()
