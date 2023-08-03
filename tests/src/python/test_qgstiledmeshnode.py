"""QGIS Unit tests for QgsTiledMeshNode

From build dir, run: ctest -R QgsTiledMeshNode -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = "(C) 2023 by Nyall Dawson"
__date__ = "26/07/2023"
__copyright__ = "Copyright 2023, The QGIS Project"

import unittest

from qgis.core import (
    QgsTiledMeshNode,
    QgsTiledMeshTile,
)
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsTiledMeshNode(QgisTestCase):
    def test_basic(self):
        tile = QgsTiledMeshTile("my tile")
        node = QgsTiledMeshNode(tile)
        self.assertEqual(node.tile().id(), "my tile")

    def test_children(self):
        node = QgsTiledMeshNode(QgsTiledMeshTile("parent"))
        self.assertFalse(node.children())

        child_node = QgsTiledMeshNode(QgsTiledMeshTile("child"))
        node.addChild(child_node)
        self.assertEqual(node.children(), [child_node])
        self.assertEqual(child_node.parentNode(), node)

        child_node2 = QgsTiledMeshNode(QgsTiledMeshTile("child2"))
        node.addChild(child_node2)
        self.assertEqual(node.children(), [child_node, child_node2])
        self.assertEqual(child_node2.parentNode(), node)


if __name__ == "__main__":
    unittest.main()
