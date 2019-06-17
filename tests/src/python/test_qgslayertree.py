# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayerTree.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '22.3.2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

import os

from qgis.core import (
    QgsLayerTree,
    QgsProject,
    QgsVectorLayer,
    QgsLayerTreeLayer
)
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtCore import QDir

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayerTree(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

    def testCustomLayerOrder(self):
        """ test project layer order"""
        prj = QgsProject()
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer3", "memory")
        prj.addMapLayers([layer, layer2, layer3])

        layer_order_changed_spy = QSignalSpy(prj.layerTreeRoot().customLayerOrderChanged)
        prj.layerTreeRoot().setCustomLayerOrder([layer2, layer])
        self.assertEqual(len(layer_order_changed_spy), 1)
        prj.layerTreeRoot().setCustomLayerOrder([layer2, layer])
        self.assertEqual(len(layer_order_changed_spy), 1) # no signal, order not changed

        self.assertEqual(prj.layerTreeRoot().customLayerOrder(), [layer2, layer])
        prj.layerTreeRoot().setCustomLayerOrder([layer])
        self.assertEqual(prj.layerTreeRoot().customLayerOrder(), [layer])
        self.assertEqual(len(layer_order_changed_spy), 2)

        # remove a layer
        prj.layerTreeRoot().setCustomLayerOrder([layer2, layer, layer3])
        self.assertEqual(len(layer_order_changed_spy), 3)
        prj.removeMapLayer(layer)
        self.assertEqual(prj.layerTreeRoot().customLayerOrder(), [layer2, layer3])
        self.assertEqual(len(layer_order_changed_spy), 4)

        # save and restore
        file_name = os.path.join(QDir.tempPath(), 'proj.qgs')
        prj.setFileName(file_name)
        prj.write()
        prj2 = QgsProject()
        prj2.setFileName(file_name)
        prj2.read()
        self.assertEqual([l.id() for l in prj2.layerTreeRoot().customLayerOrder()], [layer2.id(), layer3.id()])

        # clear project
        prj.clear()
        self.assertEqual(prj.layerTreeRoot().customLayerOrder(), [])

    def testCustomLayerOrderChanged(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")

        layer_tree = QgsLayerTree()
        layer_order_changed_spy = QSignalSpy(layer_tree.customLayerOrderChanged)
        layer1_node = QgsLayerTreeLayer(layer)
        layer_tree.addChildNode(layer1_node)
        self.assertEqual(len(layer_order_changed_spy), 1)
        layer2_node = QgsLayerTreeLayer(layer2)
        layer_tree.addChildNode(layer2_node)
        self.assertEqual(len(layer_order_changed_spy), 2)

        # simulate a layer move in the tree
        layer3_node = QgsLayerTreeLayer(layer)
        layer_tree.addChildNode(layer3_node)
        self.assertEqual(len(layer_order_changed_spy), 3)
        layer_tree.removeChildNode(layer1_node)
        self.assertEqual(len(layer_order_changed_spy), 4)


if __name__ == '__main__':
    unittest.main()
