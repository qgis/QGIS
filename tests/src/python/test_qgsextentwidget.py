"""QGIS Unit tests for QgsExtentWidget

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "25/03/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import os

from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsGeometry,
    QgsProject,
    QgsRectangle,
    QgsVectorLayer,
)
from qgis.gui import QgsExtentWidget, QgsExtentGroupBox
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsExtentWidget(QgisTestCase):

    def testGettersSetters(self):
        """test widget getters/setters"""
        w = QgsExtentWidget()

        w.setOriginalExtent(
            QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("epsg:3111")
        )
        self.assertEqual(w.originalExtent(), QgsRectangle(1, 2, 3, 4))
        self.assertEqual(w.originalCrs().authid(), "EPSG:3111")

        w.setCurrentExtent(
            QgsRectangle(11, 12, 13, 14), QgsCoordinateReferenceSystem("epsg:3113")
        )
        self.assertEqual(w.currentExtent(), QgsRectangle(11, 12, 13, 14))
        self.assertEqual(w.currentCrs().authid(), "EPSG:3113")

    def testValid(self):
        w = QgsExtentWidget()
        spy = QSignalSpy(w.validationChanged)
        self.assertFalse(w.isValid())
        w.setOriginalExtent(
            QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("epsg:3111")
        )
        w.setCurrentExtent(
            QgsRectangle(11, 12, 13, 14), QgsCoordinateReferenceSystem("epsg:3113")
        )
        w.setOutputExtentFromOriginal()
        self.assertEqual(len(spy), 1)
        self.assertTrue(w.isValid())

    def test_SettingExtent(self):
        w = QgsExtentWidget()

        spy = QSignalSpy(w.extentChanged)

        w.setOriginalExtent(
            QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("epsg:3111")
        )
        w.setCurrentExtent(
            QgsRectangle(11, 12, 13, 14), QgsCoordinateReferenceSystem("epsg:3113")
        )

        w.setOutputExtentFromOriginal()
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))
        self.assertEqual(w.extentState(), QgsExtentWidget.ExtentState.OriginalExtent)
        self.assertEqual(len(spy), 1)

        w.setOutputExtentFromCurrent()
        self.assertEqual(w.outputExtent(), QgsRectangle(11, 12, 13, 14))
        self.assertEqual(w.extentState(), QgsExtentWidget.ExtentState.CurrentExtent)
        self.assertEqual(len(spy), 2)

        w.setOutputExtentFromUser(
            QgsRectangle(21, 22, 23, 24), QgsCoordinateReferenceSystem("epsg:3111")
        )
        self.assertEqual(w.outputExtent(), QgsRectangle(21, 22, 23, 24))
        self.assertEqual(w.extentState(), QgsExtentWidget.ExtentState.UserExtent)
        self.assertEqual(len(spy), 3)

        shapefile = os.path.join(TEST_DATA_DIR, "polys.shp")
        layer = QgsVectorLayer(shapefile, "Polys", "ogr")
        QgsProject.instance().addMapLayer(layer)

        w.setOutputExtentFromLayer(None)
        # no layer - should be unchanged
        self.assertEqual(len(spy), 3)
        self.assertEqual(w.outputExtent(), QgsRectangle(21, 22, 23, 24))
        self.assertEqual(w.extentState(), QgsExtentWidget.ExtentState.UserExtent)
        self.assertEqual(len(spy), 3)

        w.setOutputExtentFromLayer(layer)
        self.assertEqual(
            w.outputExtent().toString(4),
            QgsRectangle(-118.9229, 24.5079, -83.7900, 46.7262).toString(4),
        )
        self.assertEqual(
            w.extentState(), QgsExtentWidget.ExtentState.ProjectLayerExtent
        )
        self.assertEqual(len(spy), 4)

        QgsProject.instance().removeAllMapLayers()

    def testSetOutputCrs(self):
        w = QgsExtentWidget()

        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:4326"))
        w.setCurrentExtent(
            QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("epsg:4326")
        )
        w.setOutputExtentFromCurrent()
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))

        # with reprojection
        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:3785"))
        self.assertEqual(
            w.outputExtent().toString(4),
            QgsRectangle(111319.4908, 222684.2085, 333958.4724, 445640.1097).toString(
                4
            ),
        )
        # change CRS back
        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:4326"))
        # extent should be back to current - not a reprojection of the reprojected bounds
        self.assertEqual(
            w.outputExtent().toString(20), QgsRectangle(1, 2, 3, 4).toString(20)
        )

        # repeat, this time using original extents
        w = QgsExtentGroupBox()

        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:4326"))
        w.setOriginalExtent(
            QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("epsg:4326")
        )
        w.setOutputExtentFromOriginal()
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))

        # with reprojection
        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:3785"))
        self.assertEqual(
            w.outputExtent().toString(4),
            QgsRectangle(111319.4908, 222684.2085, 333958.4724, 445640.1097).toString(
                4
            ),
        )
        # change CRS back
        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:4326"))
        # extent should be back to original - not a reprojection of the reprojected bounds
        self.assertEqual(
            w.outputExtent().toString(20), QgsRectangle(1, 2, 3, 4).toString(20)
        )

        # repeat, this time using layer extent
        layer = QgsVectorLayer("Polygon?crs=epsg:4326", "memory", "memory")
        self.assertTrue(layer.isValid())
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt("Polygon((1 2, 3 2, 3 4, 1 4, 1 2))"))
        layer.dataProvider().addFeatures([f])
        QgsProject.instance().addMapLayer(layer)
        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:4326"))
        w.setOutputExtentFromLayer(layer)
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))

        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:3785"))
        self.assertEqual(
            w.outputExtent().toString(4),
            QgsRectangle(111319.4908, 222684.2085, 333958.4724, 445640.1097).toString(
                4
            ),
        )
        # change CRS back
        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:4326"))
        # extent should be back to original - not a reprojection of the reprojected bounds
        self.assertEqual(
            w.outputExtent().toString(20), QgsRectangle(1, 2, 3, 4).toString(20)
        )

        # custom extent
        w = QgsExtentGroupBox()

        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:4326"))
        w.setOutputExtentFromUser(
            QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("epsg:4326")
        )
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))

        # with reprojection
        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:3785"))
        self.assertEqual(
            w.outputExtent().toString(4),
            QgsRectangle(111319.4908, 222684.2085, 333958.4724, 445640.1097).toString(
                4
            ),
        )
        # change CRS back
        w.setOutputCrs(QgsCoordinateReferenceSystem("epsg:4326"))
        # in this case we can't retrieve the original user extent in 4326, so we have a reprojection of the reprojected bounds
        # just test this by restricting the test to 4 decimals
        self.assertEqual(
            w.outputExtent().toString(4), QgsRectangle(1, 2, 3, 4).toString(4)
        )

    def testClear(self):
        w = QgsExtentWidget()
        w.setNullValueAllowed(True, "test")
        valid_spy = QSignalSpy(w.validationChanged)
        changed_spy = QSignalSpy(w.extentChanged)
        self.assertFalse(w.isValid())
        w.setOriginalExtent(
            QgsRectangle(1, 2, 3, 4), QgsCoordinateReferenceSystem("epsg:3111")
        )
        w.setCurrentExtent(
            QgsRectangle(11, 12, 13, 14), QgsCoordinateReferenceSystem("epsg:3113")
        )
        w.setOutputExtentFromOriginal()
        self.assertEqual(len(valid_spy), 1)
        self.assertEqual(len(changed_spy), 1)
        self.assertTrue(w.isValid())

        w.clear()
        self.assertEqual(len(valid_spy), 2)
        self.assertEqual(len(changed_spy), 2)
        self.assertFalse(w.isValid())
        self.assertTrue(w.outputExtent().isNull())
        w.clear()
        self.assertEqual(len(valid_spy), 2)
        self.assertEqual(len(changed_spy), 2)
        self.assertTrue(w.outputExtent().isNull())
        w.setOutputExtentFromOriginal()
        self.assertEqual(len(valid_spy), 3)
        self.assertEqual(len(changed_spy), 3)
        self.assertTrue(w.isValid())
        self.assertEqual(w.outputExtent(), QgsRectangle(1, 2, 3, 4))


if __name__ == "__main__":
    unittest.main()
