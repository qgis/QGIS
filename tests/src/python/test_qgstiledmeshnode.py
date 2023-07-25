"""QGIS Unit tests for QgsTiledMeshNode

From build dir, run: ctest -R QgsTiledMeshNode -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2023 by Nyall Dawson'
__date__ = '26/07/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import math
import qgis  # NOQA
from qgis.core import (
    Qgis,
    QgsSphere,
    QgsOrientedBox3D,
    QgsTiledMeshNodeBoundingVolumeSphere,
    QgsTiledMeshNodeBoundingVolumeRegion,
    QgsTiledMeshNodeBoundingVolumeBox,
    QgsBox3d,
    QgsCoordinateReferenceSystem,
    QgsCoordinateTransform,
    QgsCoordinateTransformContext,
    QgsMatrix4x4,
    QgsTiledMeshNode
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTiledMeshNode(QgisTestCase):

    def test_basic(self):
        node = QgsTiledMeshNode()
        self.assertFalse(node.isValid())

        node.setRefinementProcess(Qgis.TileRefinementProcess.Additive)
        self.assertEqual(node.refinementProcess(), Qgis.TileRefinementProcess.Additive)
        self.assertTrue(node.isValid())

        node = QgsTiledMeshNode()
        node.setBoundingVolume(QgsTiledMeshNodeBoundingVolumeRegion(QgsBox3d(1, 2, 3, 10, 11, 12)))
        self.assertEqual(node.boundingVolume().region(), QgsBox3d(1, 2, 3, 10, 11, 12))
        self.assertTrue(node.isValid())

        node = QgsTiledMeshNode()
        node.setTransform(QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        self.assertEqual(node.transform(), QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        self.assertTrue(node.isValid())

        node = QgsTiledMeshNode()
        node.setContentUri('content')
        self.assertEqual(node.contentUri(), 'content')
        self.assertTrue(node.isValid())

        node = QgsTiledMeshNode()
        node.setGeometricError(1.2)
        self.assertEqual(node.geometricError(), 1.2)
        self.assertTrue(node.isValid())

    def test_children(self):
        node = QgsTiledMeshNode()
        node.setContentUri('parent')
        self.assertFalse(node.children())

        child_node = QgsTiledMeshNode()
        child_node.setContentUri('child')
        node.addChild(child_node)
        self.assertEqual(node.children(), [child_node])
        self.assertEqual(child_node.parentNode(), node)

    def test_copy(self):
        node = QgsTiledMeshNode()
        node.setRefinementProcess(Qgis.TileRefinementProcess.Additive)
        node.setBoundingVolume(QgsTiledMeshNodeBoundingVolumeRegion(
            QgsBox3d(1, 2, 3, 10, 11, 12)))
        node.setTransform(
            QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))
        node.setContentUri('parent')
        node.setGeometricError(1.2)

        child_node = QgsTiledMeshNode()
        child_node.setContentUri('child')
        node.addChild(child_node)

        copy = QgsTiledMeshNode(node)
        self.assertTrue(copy.isValid())
        self.assertEqual(copy.refinementProcess(),
                         Qgis.TileRefinementProcess.Additive)
        self.assertEqual(copy.boundingVolume().region(),
                         QgsBox3d(1, 2, 3, 10, 11, 12))
        self.assertEqual(copy.transform(),
                         QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0,
                                      0, 0))
        self.assertEqual(copy.contentUri(), 'parent')
        self.assertEqual(copy.geometricError(), 1.2)

        # children must not be copied
        self.assertFalse(copy.children())

    def test_set_transform(self):
        node = QgsTiledMeshNode()
        node.setBoundingVolume(QgsTiledMeshNodeBoundingVolumeRegion(
            QgsBox3d(1, 2, 3, 10, 11, 12)))
        node.setTransform(
            QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0))

        # transform must be set for node's bounding volume too
        self.assertEqual(node.boundingVolume().transform(),
                         QgsMatrix4x4(1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 3, 0, 0, 0,
                                      0, 0))


if __name__ == '__main__':
    unittest.main()
