"""QGIS Unit tests for QgsAnnotationItemEditOperation

From build dir, run: ctest -R QgsAnnotationItemEditOperation -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2020 by Nyall Dawson"
__date__ = "09/09/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.core import (
    QgsAnnotationItemEditOperationAddNode,
    QgsAnnotationItemEditOperationDeleteNode,
    QgsAnnotationItemEditOperationMoveNode,
    QgsAnnotationItemEditOperationTranslateItem,
    QgsPoint,
    QgsVertexId,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationItemEditOperation(QgisTestCase):

    def test_move_operation(self):
        operation = QgsAnnotationItemEditOperationMoveNode(
            "item id", QgsVertexId(1, 2, 3), QgsPoint(4, 5), QgsPoint(6, 7)
        )
        self.assertEqual(operation.itemId(), "item id")
        self.assertEqual(operation.nodeId(), QgsVertexId(1, 2, 3))
        self.assertEqual(operation.before(), QgsPoint(4, 5))
        self.assertEqual(operation.after(), QgsPoint(6, 7))

    def test_delete_node_operation(self):
        operation = QgsAnnotationItemEditOperationDeleteNode(
            "item id", QgsVertexId(1, 2, 3), QgsPoint(6, 7)
        )
        self.assertEqual(operation.itemId(), "item id")
        self.assertEqual(operation.nodeId(), QgsVertexId(1, 2, 3))
        self.assertEqual(operation.before(), QgsPoint(6, 7))

    def test_add_node_operation(self):
        operation = QgsAnnotationItemEditOperationAddNode("item id", QgsPoint(6, 7))
        self.assertEqual(operation.itemId(), "item id")
        self.assertEqual(operation.point(), QgsPoint(6, 7))

    def test_translate_operation(self):
        operation = QgsAnnotationItemEditOperationTranslateItem("item id", 6, 7)
        self.assertEqual(operation.itemId(), "item id")
        self.assertEqual(operation.translationX(), 6)
        self.assertEqual(operation.translationY(), 7)


if __name__ == "__main__":
    unittest.main()
