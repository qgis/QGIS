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

    def test_remove_vertex(self):
        graph = QgsGraph()

        with self.assertRaises(IndexError):
            graph.removeVertex(0)
        with self.assertRaises(IndexError):
            graph.removeVertex(-1)

        v1 = graph.addVertex(QgsPointXY(1, 1))
        v2 = graph.addVertex(QgsPointXY(2, 2))
        v3 = graph.addVertex(QgsPointXY(3, 3))
        v4 = graph.addVertex(QgsPointXY(4, 4))
        edge_1 = graph.addEdge(v1, v2, [1])
        edge_2 = graph.addEdge(v2, v1, [1])
        edge_3 = graph.addEdge(v2, v3, [1])
        edge_4 = graph.addEdge(v2, v4, [1])
        edge_5 = graph.addEdge(v3, v4, [1])

        self.assertEqual(graph.vertexCount(), 4)
        self.assertEqual(graph.edgeCount(), 5)

        with self.assertRaises(IndexError):
            graph.removeVertex(5)

        # remove a vertex
        graph.removeVertex(v3)
        self.assertEqual(graph.vertexCount(), 3)
        with self.assertRaises(IndexError):
            graph.vertex(v3)
        self.assertEqual(graph.edgeCount(), 3)
        self.assertEqual(graph.edge(edge_1).fromVertex(), v1)
        self.assertEqual(graph.edge(edge_2).fromVertex(), v2)
        self.assertEqual(graph.edge(edge_4).fromVertex(), v2)

        # edges 3 and 5 must be removed
        with self.assertRaises(IndexError):
            graph.edge(edge_3)
        with self.assertRaises(IndexError):
            graph.edge(edge_5)

        with self.assertRaises(IndexError):
            graph.removeVertex(v3)

        # remove another vertex
        graph.removeVertex(v1)
        self.assertEqual(graph.vertexCount(), 2)
        with self.assertRaises(IndexError):
            graph.vertex(v1)
        self.assertEqual(graph.edgeCount(), 1)
        self.assertEqual(graph.edge(edge_4).fromVertex(), v2)
        with self.assertRaises(IndexError):
            graph.edge(edge_1)
        with self.assertRaises(IndexError):
            graph.edge(edge_2)
        with self.assertRaises(IndexError):
            graph.edge(edge_3)
        with self.assertRaises(IndexError):
            graph.edge(edge_5)

        with self.assertRaises(IndexError):
            graph.removeVertex(v1)

        # remove another vertex
        graph.removeVertex(v4)
        self.assertEqual(graph.vertexCount(), 1)
        with self.assertRaises(IndexError):
            graph.vertex(v4)
        self.assertEqual(graph.edgeCount(), 0)
        with self.assertRaises(IndexError):
            graph.edge(edge_1)
        with self.assertRaises(IndexError):
            graph.edge(edge_2)
        with self.assertRaises(IndexError):
            graph.edge(edge_3)
        with self.assertRaises(IndexError):
            graph.edge(edge_4)
        with self.assertRaises(IndexError):
            graph.edge(edge_5)

        with self.assertRaises(IndexError):
            graph.removeVertex(v4)

        # remove last vertex
        graph.removeVertex(v2)
        self.assertEqual(graph.vertexCount(), 0)
        self.assertEqual(graph.edgeCount(), 0)
        with self.assertRaises(IndexError):
            graph.vertex(v2)

        with self.assertRaises(IndexError):
            graph.removeVertex(v2)

    def test_remove_edge(self):
        graph = QgsGraph()

        with self.assertRaises(IndexError):
            graph.removeEdge(0)
        with self.assertRaises(IndexError):
            graph.removeEdge(-1)

        v1 = graph.addVertex(QgsPointXY(1, 1))
        v2 = graph.addVertex(QgsPointXY(2, 2))
        v3 = graph.addVertex(QgsPointXY(3, 3))
        v4 = graph.addVertex(QgsPointXY(4, 4))
        edge_1 = graph.addEdge(v1, v2, [1])
        edge_2 = graph.addEdge(v2, v1, [1])
        edge_3 = graph.addEdge(v2, v3, [1])
        edge_4 = graph.addEdge(v2, v4, [1])
        edge_5 = graph.addEdge(v3, v4, [1])

        self.assertEqual(graph.vertexCount(), 4)
        self.assertEqual(graph.edgeCount(), 5)

        graph.removeEdge(edge_1)
        self.assertEqual(graph.vertexCount(), 4)
        self.assertEqual(graph.edgeCount(), 4)
        with self.assertRaises(IndexError):
            graph.edge(edge_1)

        # make sure vertices are updated accordingly
        self.assertEqual(graph.vertex(v1).incomingEdges(), [edge_2])
        self.assertFalse(graph.vertex(v1).outgoingEdges())
        self.assertFalse(graph.vertex(v2).incomingEdges())
        self.assertCountEqual(graph.vertex(v2).outgoingEdges(), [edge_2, edge_3, edge_4])

        with self.assertRaises(IndexError):
            graph.removeEdge(edge_1)

        # remove another edge
        graph.removeEdge(edge_2)
        self.assertEqual(graph.vertexCount(), 3)
        self.assertEqual(graph.edgeCount(), 3)
        with self.assertRaises(IndexError):
            graph.edge(edge_2)

        # make sure vertices are updated accordingly
        # vertex 1 should be removed -- no incoming or outgoing edges remain
        with self.assertRaises(IndexError):
            graph.vertex(v1)
        self.assertFalse(graph.vertex(v2).incomingEdges())
        self.assertCountEqual(graph.vertex(v2).outgoingEdges(), [edge_3, edge_4])

        with self.assertRaises(IndexError):
            graph.removeEdge(edge_2)

        graph.removeEdge(edge_4)
        self.assertEqual(graph.vertexCount(), 3)
        self.assertEqual(graph.edgeCount(), 2)
        with self.assertRaises(IndexError):
            graph.edge(edge_4)
        self.assertFalse(graph.vertex(v2).incomingEdges())
        self.assertEqual(graph.vertex(v2).outgoingEdges(), [edge_3])
        self.assertEqual(graph.vertex(v4).incomingEdges(), [edge_5])
        self.assertFalse(graph.vertex(v4).outgoingEdges())

        with self.assertRaises(IndexError):
            graph.removeEdge(edge_4)

        graph.removeEdge(edge_3)
        self.assertEqual(graph.vertexCount(), 2)
        self.assertEqual(graph.edgeCount(), 1)
        with self.assertRaises(IndexError):
            graph.edge(edge_3)
        # v2 should be removed
        with self.assertRaises(IndexError):
            graph.vertex(v2)
        self.assertFalse(graph.vertex(v3).incomingEdges())
        self.assertEqual(graph.vertex(v3).outgoingEdges(), [edge_5])

        with self.assertRaises(IndexError):
            graph.removeEdge(edge_3)

        graph.removeEdge(edge_5)
        self.assertEqual(graph.vertexCount(), 0)
        self.assertEqual(graph.edgeCount(), 0)
        with self.assertRaises(IndexError):
            graph.edge(edge_5)
        with self.assertRaises(IndexError):
            graph.vertex(v3)
        with self.assertRaises(IndexError):
            graph.vertex(v4)

        with self.assertRaises(IndexError):
            graph.removeEdge(edge_5)

    def test_find_opposite_edge(self):
        graph = QgsGraph()

        with self.assertRaises(IndexError):
            graph.findOppositeEdge(0)
        with self.assertRaises(IndexError):
            graph.findOppositeEdge(-1)

        v1 = graph.addVertex(QgsPointXY(1, 1))
        v2 = graph.addVertex(QgsPointXY(2, 2))
        v3 = graph.addVertex(QgsPointXY(3, 3))
        v4 = graph.addVertex(QgsPointXY(4, 4))
        edge_1 = graph.addEdge(v1, v2, [1])
        edge_2 = graph.addEdge(v2, v1, [1])
        edge_3 = graph.addEdge(v2, v3, [1])
        edge_4 = graph.addEdge(v2, v4, [1])
        edge_5 = graph.addEdge(v3, v4, [1])

        self.assertEqual(graph.findOppositeEdge(edge_1), edge_2)
        self.assertEqual(graph.findOppositeEdge(edge_2), edge_1)
        self.assertEqual(graph.findOppositeEdge(edge_3), -1)
        self.assertEqual(graph.findOppositeEdge(edge_4), -1)
        self.assertEqual(graph.findOppositeEdge(edge_5), -1)


if __name__ == '__main__':
    unittest.main()
