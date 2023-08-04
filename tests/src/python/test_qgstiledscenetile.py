"""QGIS Unit tests for QgsTiledSceneTile

From build dir, run: ctest -R QgsTiledSceneTile -V

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
    QgsTiledSceneBoundingVolumeRegion,
    QgsBox3d,
    QgsMatrix4x4,
    QgsTiledSceneTile,
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTiledSceneTile(QgisTestCase):
    def test_basic(self):
        node = QgsTiledSceneTile()
        self.assertFalse(node.isValid())
        self.assertFalse(node.id())

        node = QgsTiledSceneTile("id")
        self.assertTrue(node.isValid())
        self.assertEqual(node.id(), "id")

        node.setRefinementProcess(Qgis.TileRefinementProcess.Additive)
        self.assertEqual(node.refinementProcess(), Qgis.TileRefinementProcess.Additive)
        self.assertTrue(node.isValid())

        node = QgsTiledSceneTile()
        node.setBoundingVolume(
            QgsTiledSceneBoundingVolumeRegion(QgsBox3d(1, 2, 3, 10, 11, 12))
        )
        self.assertEqual(node.boundingVolume().region(), QgsBox3d(1, 2, 3, 10, 11, 12))

        node = QgsTiledSceneTile()
        node.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        self.assertEqual(
            node.transform(),
            QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0),
        )

        node = QgsTiledSceneTile()
        node.setResources({"content": "uri"})
        self.assertEqual(node.resources(), {"content": "uri"})

        node = QgsTiledSceneTile()
        node.setGeometricError(1.2)
        self.assertEqual(node.geometricError(), 1.2)

    def test_copy(self):
        node = QgsTiledSceneTile("id")
        node.setRefinementProcess(Qgis.TileRefinementProcess.Additive)
        node.setBoundingVolume(
            QgsTiledSceneBoundingVolumeRegion(QgsBox3d(1, 2, 3, 10, 11, 12))
        )
        node.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        node.setResources({"content": "parent"})
        node.setGeometricError(1.2)

        copy = QgsTiledSceneTile(node)
        self.assertTrue(copy.isValid())
        self.assertEqual(copy.id(), "id")
        self.assertEqual(copy.refinementProcess(), Qgis.TileRefinementProcess.Additive)
        self.assertEqual(copy.boundingVolume().region(), QgsBox3d(1, 2, 3, 10, 11, 12))
        self.assertEqual(
            copy.transform(),
            QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0),
        )
        self.assertEqual(copy.resources(), {"content": "parent"})
        self.assertEqual(copy.geometricError(), 1.2)

    def test_set_transform(self):
        node = QgsTiledSceneTile()
        self.assertIsNone(node.transform())
        node.setBoundingVolume(
            QgsTiledSceneBoundingVolumeRegion(QgsBox3d(1, 2, 3, 10, 11, 12))
        )
        node.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))

        node.setTransform(QgsMatrix4x4())
        self.assertIsNone(node.transform())


if __name__ == "__main__":
    unittest.main()
