"""QGIS Unit tests for QgsLayoutNorthArrowHandler.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2020 by Nyall Dawson"
__date__ = "05/04/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsLayout,
    QgsLayoutItemMap,
    QgsLayoutNorthArrowHandler,
    QgsProject,
    QgsRectangle,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutNorthArrowHandler(QgisTestCase):

    def testNorthArrowWithMapItemRotation(self):
        """Test arrow rotation when map item is also rotated"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.setExtent(QgsRectangle(0, -256, 256, 0))
        layout.addLayoutItem(map)

        handler = QgsLayoutNorthArrowHandler(layout)
        spy = QSignalSpy(handler.arrowRotationChanged)

        handler.setLinkedMap(map)
        self.assertEqual(handler.linkedMap(), map)
        self.assertEqual(len(spy), 0)

        handler.setNorthMode(QgsLayoutNorthArrowHandler.NorthMode.GridNorth)
        map.setItemRotation(45)
        self.assertEqual(handler.arrowRotation(), 45)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], 45)
        map.setMapRotation(-34)
        self.assertEqual(handler.arrowRotation(), 11)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], 11)

        # add an offset
        handler.setNorthOffset(-10)
        self.assertEqual(handler.arrowRotation(), 1)
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1][0], 1)

        map.setItemRotation(55)
        self.assertEqual(handler.arrowRotation(), 11)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1][0], 11)

    def testMapWithInitialRotation(self):
        """Test arrow rotation when map item is initially rotated"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.setExtent(QgsRectangle(0, -256, 256, 0))
        map.setRotation(45)
        layout.addLayoutItem(map)

        handler = QgsLayoutNorthArrowHandler(layout)
        spy = QSignalSpy(handler.arrowRotationChanged)

        handler.setLinkedMap(map)
        self.assertEqual(handler.linkedMap(), map)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], 45)

        handler.setLinkedMap(None)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], 0)

    def testGridNorth(self):
        """Test syncing arrow to grid north"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.setExtent(QgsRectangle(0, -256, 256, 0))
        layout.addLayoutItem(map)

        handler = QgsLayoutNorthArrowHandler(layout)
        spy = QSignalSpy(handler.arrowRotationChanged)

        handler.setLinkedMap(map)
        self.assertEqual(handler.linkedMap(), map)
        self.assertEqual(len(spy), 0)

        handler.setNorthMode(QgsLayoutNorthArrowHandler.NorthMode.GridNorth)
        map.setMapRotation(45)
        self.assertEqual(handler.arrowRotation(), 45)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], 45)

        # add an offset
        handler.setNorthOffset(-10)
        self.assertEqual(handler.arrowRotation(), 35)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], 35)

    def testTrueNorth(self):
        """Test syncing arrow to true north"""

        layout = QgsLayout(QgsProject.instance())

        map = QgsLayoutItemMap(layout)
        map.attemptSetSceneRect(QRectF(0, 0, 10, 10))
        map.setCrs(QgsCoordinateReferenceSystem.fromEpsgId(3575))
        map.setExtent(
            QgsRectangle(-2126029.962, -2200807.749, -119078.102, -757031.156)
        )
        layout.addLayoutItem(map)

        handler = QgsLayoutNorthArrowHandler(layout)
        spy = QSignalSpy(handler.arrowRotationChanged)

        handler.setLinkedMap(map)
        self.assertEqual(handler.linkedMap(), map)
        self.assertEqual(len(spy), 0)

        handler.setNorthMode(QgsLayoutNorthArrowHandler.NorthMode.TrueNorth)
        self.assertAlmostEqual(handler.arrowRotation(), 37.20, 1)
        self.assertEqual(len(spy), 1)
        self.assertAlmostEqual(spy[-1][0], 37.20, 1)

        # shift map
        map.setExtent(
            QgsRectangle(2120672.293, -3056394.691, 2481640.226, -2796718.780)
        )
        self.assertAlmostEqual(handler.arrowRotation(), -38.18, 1)
        self.assertEqual(len(spy), 2)
        self.assertAlmostEqual(spy[-1][0], -38.18, 1)

        # rotate map
        map.setMapRotation(45)
        self.assertAlmostEqual(handler.arrowRotation(), -38.18 + 45, 1)
        self.assertEqual(len(spy), 3)
        self.assertAlmostEqual(spy[-1][0], -38.18 + 45, 1)

        # add an offset
        handler.setNorthOffset(-10)
        self.assertAlmostEqual(handler.arrowRotation(), -38.18 + 35, 1)
        self.assertEqual(len(spy), 4)
        self.assertAlmostEqual(spy[-1][0], -38.18 + 35, 1)


if __name__ == "__main__":
    unittest.main()
