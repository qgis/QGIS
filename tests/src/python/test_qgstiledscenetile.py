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

from qgis.PyQt.QtCore import QUrl
from qgis.core import (
    Qgis,
    QgsTiledSceneBoundingVolume,
    QgsBox3d,
    QgsMatrix4x4,
    QgsTiledSceneTile,
    QgsOrientedBox3D,
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTiledSceneTile(QgisTestCase):
    def test_basic(self):
        node = QgsTiledSceneTile()
        self.assertFalse(node.isValid())
        self.assertEqual(node.id(), -1)

        node = QgsTiledSceneTile(11)
        self.assertTrue(node.isValid())
        self.assertEqual(node.id(), 11)

        node.setRefinementProcess(Qgis.TileRefinementProcess.Additive)
        self.assertEqual(node.refinementProcess(), Qgis.TileRefinementProcess.Additive)
        self.assertTrue(node.isValid())

        node = QgsTiledSceneTile()
        node.setBoundingVolume(
            QgsTiledSceneBoundingVolume(
                QgsOrientedBox3D.fromBox3D(QgsBox3d(1, 2, 3, 10, 11, 12))
            )
        )
        self.assertEqual(
            node.boundingVolume().box(),
            QgsOrientedBox3D([5.5, 6.5, 7.5], [4.5, 0, 0, 0, 4.5, 0, 0, 0, 4.5]),
        )

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

        node = QgsTiledSceneTile()
        node.setBaseUrl(QUrl("http://example.com/foo.txt"))
        self.assertEqual(node.baseUrl(), QUrl("http://example.com/foo.txt"))

    def test_copy(self):
        node = QgsTiledSceneTile(11)
        node.setRefinementProcess(Qgis.TileRefinementProcess.Additive)
        node.setBoundingVolume(
            QgsTiledSceneBoundingVolume(
                QgsOrientedBox3D.fromBox3D(QgsBox3d(1, 2, 3, 10, 11, 12))
            )
        )
        node.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        node.setResources({"content": "parent"})
        node.setGeometricError(1.2)
        node.setBaseUrl(QUrl("http://example.com/hello.json"))

        copy = QgsTiledSceneTile(node)
        self.assertTrue(copy.isValid())
        self.assertEqual(copy.id(), 11)
        self.assertEqual(copy.refinementProcess(), Qgis.TileRefinementProcess.Additive)
        self.assertEqual(
            copy.boundingVolume().box(),
            QgsOrientedBox3D([5.5, 6.5, 7.5], [4.5, 0, 0, 0, 4.5, 0, 0, 0, 4.5]),
        )
        self.assertEqual(
            copy.transform(),
            QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0),
        )
        self.assertEqual(copy.resources(), {"content": "parent"})
        self.assertEqual(copy.geometricError(), 1.2)
        self.assertEqual(copy.baseUrl(), QUrl("http://example.com/hello.json"))

    def test_set_transform(self):
        node = QgsTiledSceneTile()
        self.assertIsNone(node.transform())
        node.setBoundingVolume(
            QgsTiledSceneBoundingVolume(
                QgsOrientedBox3D.fromBox3D(QgsBox3d(1, 2, 3, 10, 11, 12))
            )
        )
        node.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))

        node.setTransform(QgsMatrix4x4())
        self.assertIsNone(node.transform())


if __name__ == "__main__":
    unittest.main()
