"""QGIS Unit tests for QgsTiledMeshTile

From build dir, run: ctest -R QgsTiledMeshTile -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = "(C) 2023 by Nyall Dawson"
__date__ = "26/07/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import unittest

import qgis  # NOQA
from qgis.core import (
    Qgis,
    QgsTiledMeshNodeBoundingVolumeRegion,
    QgsBox3d,
    QgsMatrix4x4,
    QgsTiledMeshTile,
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTiledMeshTile(QgisTestCase):
    def test_basic(self):
        node = QgsTiledMeshTile()
        self.assertFalse(node.isValid())
        self.assertFalse(node.id())

        node = QgsTiledMeshTile('id')
        self.assertTrue(node.isValid())
        self.assertEqual(node.id(), 'id')

        node.setRefinementProcess(Qgis.TileRefinementProcess.Additive)
        self.assertEqual(node.refinementProcess(), Qgis.TileRefinementProcess.Additive)
        self.assertTrue(node.isValid())

        node = QgsTiledMeshTile()
        node.setBoundingVolume(
            QgsTiledMeshNodeBoundingVolumeRegion(QgsBox3d(1, 2, 3, 10, 11, 12))
        )
        self.assertEqual(node.boundingVolume().region(), QgsBox3d(1, 2, 3, 10, 11, 12))

        node = QgsTiledMeshTile()
        node.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        self.assertEqual(
            node.transform(),
            QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0),
        )

        node = QgsTiledMeshTile()
        node.setResources({"content": "uri"})
        self.assertEqual(node.resources(), {"content": "uri"})

        node = QgsTiledMeshTile()
        node.setGeometricError(1.2)
        self.assertEqual(node.geometricError(), 1.2)

    def test_copy(self):
        node = QgsTiledMeshTile('id')
        node.setRefinementProcess(Qgis.TileRefinementProcess.Additive)
        node.setBoundingVolume(
            QgsTiledMeshNodeBoundingVolumeRegion(QgsBox3d(1, 2, 3, 10, 11, 12))
        )
        node.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        node.setResources({"content": "parent"})
        node.setGeometricError(1.2)

        copy = QgsTiledMeshTile(node)
        self.assertTrue(copy.isValid())
        self.assertEqual(copy.id(), 'id')
        self.assertEqual(copy.refinementProcess(), Qgis.TileRefinementProcess.Additive)
        self.assertEqual(copy.boundingVolume().region(), QgsBox3d(1, 2, 3, 10, 11, 12))
        self.assertEqual(
            copy.transform(),
            QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0),
        )
        self.assertEqual(copy.resources(), {"content": "parent"})
        self.assertEqual(copy.geometricError(), 1.2)

    def test_set_transform(self):
        node = QgsTiledMeshTile()
        self.assertIsNone(node.transform())
        node.setBoundingVolume(
            QgsTiledMeshNodeBoundingVolumeRegion(QgsBox3d(1, 2, 3, 10, 11, 12))
        )
        node.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))

        node.setTransform(QgsMatrix4x4())
        self.assertIsNone(node.transform())


if __name__ == "__main__":
    unittest.main()
