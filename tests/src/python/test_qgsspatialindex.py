"""QGIS Unit tests for QgsSpatialIndex.

.. note:: This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
"""

__author__ = "Alexander Bruy"
__date__ = "20/01/2011"
__copyright__ = "Copyright 2012, The QGIS Project"


from qgis.core import (
    QgsFeature,
    QgsGeometry,
    QgsPointXY,
    QgsRectangle,
    QgsSpatialIndex,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsSpatialIndex(QgisTestCase):

    def testIndex(self):
        idx = QgsSpatialIndex()
        fid = 0
        for y in range(5, 15, 5):
            for x in range(5, 25, 5):
                ft = QgsFeature()
                ft.setId(fid)
                ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(x, y)))
                idx.addFeature(ft)
                fid += 1

        # intersection test
        rect = QgsRectangle(7.0, 3.0, 17.0, 13.0)
        fids = idx.intersects(rect)
        myExpectedValue = 4
        myValue = len(fids)
        myMessage = f"Expected: {myExpectedValue} Got: {myValue}"
        self.assertEqual(myValue, myExpectedValue, myMessage)
        fids.sort()
        myMessage = f"Expected: {[1, 2, 5, 6]}\nGot: {fids}\n"
        assert fids == [1, 2, 5, 6], myMessage

        # nearest neighbor test
        fids = idx.nearestNeighbor(QgsPointXY(8.75, 6.25), 3)
        myExpectedValue = 0
        myValue = len(fids)
        myMessage = f"Expected: {myExpectedValue} Got: {myValue}"

        fids.sort()
        myMessage = f"Expected: {[0, 1, 5]}\nGot: {fids}\n"
        assert fids == [0, 1, 5], myMessage

    def testGetGeometry(self):
        idx = QgsSpatialIndex()
        idx2 = QgsSpatialIndex(QgsSpatialIndex.Flag.FlagStoreFeatureGeometries)
        fid = 0
        for y in range(5):
            for x in range(10, 15):
                ft = QgsFeature()
                ft.setId(fid)
                ft.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(x, y)))
                idx.addFeature(ft)
                idx2.addFeature(ft)
                fid += 1

        # not storing geometries, a keyerror should be raised
        with self.assertRaises(KeyError):
            idx.geometry(-100)
        with self.assertRaises(KeyError):
            idx.geometry(1)
        with self.assertRaises(KeyError):
            idx.geometry(2)
        with self.assertRaises(KeyError):
            idx.geometry(1000)

        self.assertEqual(idx2.geometry(1).asWkt(1), "Point (11 0)")
        self.assertEqual(idx2.geometry(2).asWkt(1), "Point (12 0)")
        with self.assertRaises(KeyError):
            idx2.geometry(-100)
        with self.assertRaises(KeyError):
            idx2.geometry(1000)


if __name__ == "__main__":
    unittest.main()
