# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsGraph.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '08/11/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import qgis  # NOQA
from qgis.analysis import (
    QgsGraph
)
from qgis.core import (
    QgsPointXY
)
from qgis.testing import start_app, unittest

start_app()


class TestQgsGraph(unittest.TestCase):

    def test_empty_graph(self):
        graph = QgsGraph()
        self.assertEqual(graph.vertexCount(), 0)
        self.assertEqual(graph.edgeCount(), 0)

    def test_graph_vertices(self):
        graph = QgsGraph()

        self.assertEqual(graph.findVertex(QgsPointXY(10, 11)), -1)

        with self.assertRaises(IndexError):
            graph.vertex(-1)
        with self.assertRaises(IndexError):
            graph.vertex(0)

        vertex_1 = graph.addVertex(QgsPointXY(1, 2))
        self.assertEqual(graph.vertexCount(), 1)
        self.assertEqual(graph.vertex(vertex_1).point(), QgsPointXY(1, 2))
        self.assertFalse(graph.vertex(vertex_1).incomingEdges())
        self.assertFalse(graph.vertex(vertex_1).outgoingEdges())

        self.assertEqual(graph.findVertex(QgsPointXY(10, 11)), -1)
        self.assertEqual(graph.findVertex(QgsPointXY(1, 2)), vertex_1)

        with self.assertRaises(IndexError):
            graph.vertex(-1)
        with self.assertRaises(IndexError):
            graph.vertex(2)

        vertex_2 = graph.addVertex(QgsPointXY(3, 4))
        self.assertEqual(graph.vertexCount(), 2)
        self.assertEqual(graph.vertex(vertex_1).point(), QgsPointXY(1, 2))
        self.assertFalse(graph.vertex(vertex_1).incomingEdges())
        self.assertFalse(graph.vertex(vertex_1).outgoingEdges())
        self.assertEqual(graph.vertex(vertex_2).point(), QgsPointXY(3, 4))
        self.assertFalse(graph.vertex(vertex_2).incomingEdges())
        self.assertFalse(graph.vertex(vertex_2).outgoingEdges())

        with self.assertRaises(IndexError):
            graph.vertex(-1)
        with self.assertRaises(IndexError):
            graph.vertex(3)

        self.assertEqual(graph.findVertex(QgsPointXY(10, 11)), -1)
        self.assertEqual(graph.findVertex(QgsPointXY(1, 2)), vertex_1)
        self.assertEqual(graph.findVertex(QgsPointXY(3, 4)), vertex_2)

    def test_graph_edges(self):
        graph = QgsGraph()
        with self.assertRaises(IndexError):
            graph.edge(0)

        vertex_1 = graph.addVertex(QgsPointXY(1, 2))
        vertex_2 = graph.addVertex(QgsPointXY(3, 4))
        vertex_3 = graph.addVertex(QgsPointXY(5, 6))

        edge_1 = graph.addEdge(vertex_1, vertex_2, [1, 2])
        self.assertEqual(graph.edgeCount(), 1)
        self.assertEqual(graph.edge(edge_1).cost(0), 1)
        self.assertEqual(graph.edge(edge_1).cost(1), 2)
        self.assertEqual(graph.edge(edge_1).strategies(), [1, 2])
        self.assertEqual(graph.edge(edge_1).fromVertex(), vertex_1)
        self.assertEqual(graph.edge(edge_1).toVertex(), vertex_2)

        with self.assertRaises(IndexError):
            graph.edge(-1)
        with self.assertRaises(IndexError):
            graph.edge(1)

        edge_2 = graph.addEdge(vertex_2, vertex_1, [1, 2])
        self.assertEqual(graph.edgeCount(), 2)
        self.assertEqual(graph.edge(edge_1).fromVertex(), vertex_1)
        self.assertEqual(graph.edge(edge_1).toVertex(), vertex_2)
        self.assertEqual(graph.edge(edge_2).cost(0), 1)
        self.assertEqual(graph.edge(edge_2).cost(1), 2)
        self.assertEqual(graph.edge(edge_2).strategies(), [1, 2])
        self.assertEqual(graph.edge(edge_2).fromVertex(), vertex_2)
        self.assertEqual(graph.edge(edge_2).toVertex(), vertex_1)

        with self.assertRaises(IndexError):
            graph.edge(2)

        edge_3 = graph.addEdge(vertex_3, vertex_1, [11, 12])
        self.assertEqual(graph.edgeCount(), 3)
        self.assertEqual(graph.edge(edge_1).fromVertex(), vertex_1)
        self.assertEqual(graph.edge(edge_1).toVertex(), vertex_2)
        self.assertEqual(graph.edge(edge_2).fromVertex(), vertex_2)
        self.assertEqual(graph.edge(edge_2).toVertex(), vertex_1)
        self.assertEqual(graph.edge(edge_3).cost(0), 11)
        self.assertEqual(graph.edge(edge_3).cost(1), 12)
        self.assertEqual(graph.edge(edge_3).strategies(), [11, 12])
        self.assertEqual(graph.edge(edge_3).fromVertex(), vertex_3)
        self.assertEqual(graph.edge(edge_3).toVertex(), vertex_1)

        with self.assertRaises(IndexError):
            graph.edge(3)


if __name__ == '__main__':
    unittest.main()
