# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAnnotationItemEditOperation

From build dir, run: ctest -R QgsAnnotationItemEditOperation -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '09/09/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA
from qgis.core import (
    QgsAbstractAnnotationItemEditOperation,
    QgsAnnotationItemEditOperationMoveNode,
    QgsVertexId,
    QgsPointXY
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationItemEditOperation(unittest.TestCase):

    def test_basic(self):
        base = QgsAbstractAnnotationItemEditOperation('my item')
        self.assertEqual(base.itemId(), 'my item')

    def test_move_operation(self):
        operation = QgsAnnotationItemEditOperationMoveNode('item id', QgsVertexId(1, 2, 3), QgsPointXY(4, 5), QgsPointXY(6, 7))
        self.assertEqual(operation.itemId(), 'item id')
        self.assertEqual(operation.nodeId(), QgsVertexId(1, 2, 3))
        self.assertEqual(operation.before(), QgsPointXY(4, 5))
        self.assertEqual(operation.after(), QgsPointXY(6, 7))


if __name__ == '__main__':
    unittest.main()
