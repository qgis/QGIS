# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAnnotationItemNode

From build dir, run: ctest -R PyQgsAnnotationLayer -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '29/07/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA
from qgis.core import (
    QgsAnnotationItemNode,
    QgsPointXY,
    Qgis,
    QgsVertexId
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsAnnotationItemNode(unittest.TestCase):

    def test_basic(self):
        node = QgsAnnotationItemNode(QgsVertexId(0, 0, 1), QgsPointXY(1, 2), Qgis.AnnotationItemNodeType.VertexHandle)
        self.assertEqual(node.point(), QgsPointXY(1, 2))
        self.assertEqual(node.id(), QgsVertexId(0, 0, 1))

        node.setPoint(QgsPointXY(3, 4))
        self.assertEqual(node.point(), QgsPointXY(3, 4))

        self.assertEqual(node.type(), Qgis.AnnotationItemNodeType.VertexHandle)

    def test_repr(self):
        node = QgsAnnotationItemNode(QgsVertexId(0, 0, 1), QgsPointXY(1, 2), Qgis.AnnotationItemNodeType.VertexHandle)
        self.assertEqual(str(node), '<QgsAnnotationItemNode: 1 - VertexHandle (1, 2)>')

    def test_equality(self):
        node = QgsAnnotationItemNode(QgsVertexId(0, 0, 1), QgsPointXY(1, 2), Qgis.AnnotationItemNodeType.VertexHandle)
        node2 = QgsAnnotationItemNode(QgsVertexId(0, 0, 1), QgsPointXY(1, 2), Qgis.AnnotationItemNodeType.VertexHandle)
        self.assertEqual(node, node2)

        node2.setPoint(QgsPointXY(3, 4))
        self.assertNotEqual(node, node2)

        node = QgsAnnotationItemNode(QgsVertexId(0, 0, 1), QgsPointXY(1, 2), Qgis.AnnotationItemNodeType.VertexHandle)
        node2 = QgsAnnotationItemNode(QgsVertexId(0, 0, 2), QgsPointXY(1, 2), Qgis.AnnotationItemNodeType.VertexHandle)
        self.assertNotEqual(node, node2)


if __name__ == '__main__':
    unittest.main()
