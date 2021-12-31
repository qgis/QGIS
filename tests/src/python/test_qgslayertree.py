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
    QgsLayerTreeLayer,
    QgsLayerTreeGroup,
    QgsGroupLayer,
    QgsCoordinateTransformContext
)
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtCore import (
    QDir,
    QCoreApplication,
    QEvent
)
from tempfile import TemporaryDirectory

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
        self.assertEqual(len(layer_order_changed_spy), 1)  # no signal, order not changed

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

    def testNodeCustomProperties(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        layer1_node = QgsLayerTreeLayer(layer)
        spy = QSignalSpy(layer1_node.customPropertyChanged)

        self.assertFalse(layer1_node.customProperty('test'))
        self.assertNotIn('test', layer1_node.customProperties())

        layer1_node.setCustomProperty('test', 'value')
        self.assertEqual(len(spy), 1)
        # set to same value, should be no extra signal
        layer1_node.setCustomProperty('test', 'value')
        self.assertEqual(len(spy), 1)
        self.assertIn('test', layer1_node.customProperties())
        self.assertEqual(layer1_node.customProperty('test'), 'value')
        layer1_node.setCustomProperty('test', 'value2')
        self.assertEqual(len(spy), 2)
        self.assertIn('test', layer1_node.customProperties())
        self.assertEqual(layer1_node.customProperty('test'), 'value2')

        layer1_node.removeCustomProperty('test')
        self.assertEqual(len(spy), 3)
        self.assertFalse(layer1_node.customProperty('test'))
        self.assertNotIn('test', layer1_node.customProperties())

        # already removed, should be no extra signal
        layer1_node.removeCustomProperty('test')
        self.assertEqual(len(spy), 3)

    def test_layer_tree_group_layer(self):
        """
        Test setting a group layer on a QgsLayerTreeGroup
        """
        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('group', options)

        group_node = QgsLayerTreeGroup()
        self.assertFalse(group_node.groupLayer())
        group_node.setGroupLayer(group_layer)
        self.assertEqual(group_node.groupLayer(), group_layer)

        group_layer.deleteLater()
        group_layer = None
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)
        # should be automatically cleaned
        self.assertFalse(group_node.groupLayer())

    def test_copy_layer_tree_group(self):
        # copying layer tree group should also copy group layer setting
        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = QgsGroupLayer('group', options)

        group_node = QgsLayerTreeGroup()
        group_node.setGroupLayer(group_layer)
        self.assertEqual(group_node.groupLayer(), group_layer)

        group_node2 = group_node.clone()
        self.assertEqual(group_node2.groupLayer(), group_layer)

    def test_convert_group_to_group_layer(self):
        """
        Test converting a QgsLayerTreeGroup to a QgsGroupLayer
        """
        group_node = QgsLayerTreeGroup()

        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = group_node.convertToGroupLayer(options)
        self.assertFalse(group_layer.childLayers())
        self.assertEqual(group_node.groupLayer(), group_layer)

        # if a group layer is already assigned, convertToGroupLayer should do nothing
        self.assertIsNone(group_node.convertToGroupLayer(options))

        group_node.setGroupLayer(None)
        # add some child layers to node
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        group_node.addLayer(layer)
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        group_node.addLayer(layer2)

        group_layer = group_node.convertToGroupLayer(options)
        self.assertEqual(group_layer.childLayers(), [layer2, layer])
        self.assertEqual(group_node.groupLayer(), group_layer)

    def test_restore_group_node_group_layer(self):
        """
        Test that group node's QgsGroupLayers are restored with projects
        """
        p = QgsProject()
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        p.addMapLayer(layer, False)
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        p.addMapLayer(layer2, False)

        group_node = p.layerTreeRoot().addGroup('my group')
        group_node.addLayer(layer)
        group_node.addLayer(layer2)
        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = group_node.convertToGroupLayer(options)
        p.addMapLayer(group_layer, False)

        with TemporaryDirectory() as d:
            path = os.path.join(d, 'group_layers.qgs')

            p.setFileName(path)
            p.write()

            p2 = QgsProject()
            p2.read(path)

            restored_group_node = p2.layerTreeRoot().children()[0]
            self.assertEqual(restored_group_node.name(), 'my group')

            restored_group_layer = restored_group_node.groupLayer()
            self.assertIsNotNone(restored_group_layer)

            self.assertEqual(restored_group_layer.childLayers()[0].name(), 'layer2')
            self.assertEqual(restored_group_layer.childLayers()[1].name(), 'layer1')

    def test_group_layer_updates_from_node(self):
        """
        Test that group layer child layers are synced correctly from the group node
        """
        group_node = QgsLayerTreeGroup('my group')
        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = group_node.convertToGroupLayer(options)
        self.assertFalse(group_layer.childLayers())

        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        group_node.addLayer(layer)
        self.assertEqual(group_layer.childLayers(), [layer])

        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        group_node.insertLayer(0, layer2)
        self.assertEqual(group_layer.childLayers(), [layer, layer2])

        layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer3", "memory")
        layer4 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer4", "memory")
        layer3_node = QgsLayerTreeLayer(layer3)
        layer4_node = QgsLayerTreeLayer(layer4)
        group_node.insertChildNodes(1, [layer3_node, layer4_node])
        self.assertEqual(group_layer.childLayers(), [layer, layer4, layer3, layer2])

        layer5 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer5", "memory")
        layer5_node = QgsLayerTreeLayer(layer5)
        group_node.addChildNode(layer5_node)
        self.assertEqual(group_layer.childLayers(), [layer5, layer, layer4, layer3, layer2])

        group_node.removeChildNode(layer3_node)
        self.assertEqual(group_layer.childLayers(), [layer5, layer, layer4, layer2])

        group_node.removeLayer(layer)
        self.assertEqual(group_layer.childLayers(), [layer5, layer4, layer2])

        group_node.removeChildren(0, 2)
        self.assertEqual(group_layer.childLayers(), [layer5])

        group_node.removeAllChildren()
        self.assertEqual(group_layer.childLayers(), [])

    def test_group_layer_updates_from_node_visibility(self):
        """
        Test that group layer child layers are synced correctly from the group node when layer visibility is changed
        """
        group_node = QgsLayerTreeGroup('my group')
        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = group_node.convertToGroupLayer(options)
        self.assertFalse(group_layer.childLayers())

        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        group_node.addLayer(layer)
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        layer2_node = group_node.addLayer(layer2)
        layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer3", "memory")
        group_node.addLayer(layer3)
        self.assertEqual(group_layer.childLayers(), [layer3, layer2, layer])

        layer2_node.setItemVisibilityChecked(False)
        self.assertEqual(group_layer.childLayers(), [layer3, layer])

        layer2_node.setItemVisibilityChecked(True)
        self.assertEqual(group_layer.childLayers(), [layer3, layer2, layer])

    def test_group_layer_nested(self):
        """
        Test group node with child nodes converted to group layer
        """
        group_node = QgsLayerTreeGroup('my group')
        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = group_node.convertToGroupLayer(options)
        self.assertFalse(group_layer.childLayers())

        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        group_node.addLayer(layer)
        group2 = group_node.addGroup('child group 1')
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        group_node.addLayer(layer2)

        layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer3", "memory")
        layer3_node = group2.addLayer(layer3)

        group3 = group2.addGroup('grand child group 1')
        group4 = group2.addGroup('grand child group 2')

        layer4 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer4", "memory")
        layer4_node = group3.addLayer(layer4)
        layer5 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer5", "memory")
        layer5_node = group4.addLayer(layer5)

        self.assertEqual(group_layer.childLayers(), [layer2, layer5, layer4, layer3, layer])

        layer5_node.setItemVisibilityChecked(False)
        self.assertEqual(group_layer.childLayers(), [layer2, layer4, layer3, layer])

        group2.setItemVisibilityChecked(False)
        self.assertEqual(group_layer.childLayers(), [layer2, layer])
        group2.setItemVisibilityCheckedRecursive(True)
        self.assertEqual(group_layer.childLayers(), [layer2, layer5, layer4, layer3, layer])

    def test_layer_order_with_group_layer(self):
        """
        Test retrieving layer order with group layers present
        """
        p = QgsProject()
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        p.addMapLayer(layer, False)
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        p.addMapLayer(layer2, False)
        layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer3", "memory")
        p.addMapLayer(layer3, False)
        layer4 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer4", "memory")
        p.addMapLayer(layer4, False)

        p.layerTreeRoot().addLayer(layer)
        group_node = p.layerTreeRoot().addGroup('my group')
        group_node.addLayer(layer2)
        group_node.addLayer(layer3)
        p.layerTreeRoot().addLayer(layer4)

        self.assertEqual(p.layerTreeRoot().layerOrder(), [layer, layer2, layer3, layer4])

        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = group_node.convertToGroupLayer(options)
        p.addMapLayer(group_layer, False)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [layer, group_layer, layer4])
        self.assertEqual(group_layer.childLayers(), [layer3, layer2])

    def test_nested_groups(self):
        """
        Test logic relating to nested groups with group layers
        """
        p = QgsProject()
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        p.addMapLayer(layer, False)
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        p.addMapLayer(layer2, False)
        layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer3", "memory")
        p.addMapLayer(layer3, False)
        layer4 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer4", "memory")
        p.addMapLayer(layer4, False)

        group_node = p.layerTreeRoot().addGroup('my group')
        group_node.addLayer(layer)
        group_node.addLayer(layer2)

        child_group = group_node.addGroup('child')
        layer3_node = child_group.addLayer(layer3)

        grandchild_group = child_group.addGroup('grandchild')
        layer4_node = grandchild_group.addLayer(layer4)

        self.assertEqual(p.layerTreeRoot().layerOrder(), [layer, layer2, layer3, layer4])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [layer, layer2, layer3, layer4])

        spy = QSignalSpy(p.layerTreeRoot().layerOrderChanged)

        options = QgsGroupLayer.LayerOptions(QgsCoordinateTransformContext())
        group_layer = group_node.convertToGroupLayer(options)
        p.addMapLayer(group_layer, False)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [layer4, layer3, layer2, layer])
        spy_count = len(spy)
        self.assertEqual(spy_count, 1)

        grandchild_group_layer = grandchild_group.convertToGroupLayer(options)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [grandchild_group_layer, layer3, layer2, layer])
        self.assertEqual(grandchild_group_layer.childLayers(), [layer4])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        layer4_node.setItemVisibilityChecked(False)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [grandchild_group_layer, layer3, layer2, layer])
        self.assertEqual(grandchild_group_layer.childLayers(), [])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        layer4_node.setItemVisibilityChecked(True)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [grandchild_group_layer, layer3, layer2, layer])
        self.assertEqual(grandchild_group_layer.childLayers(), [layer4])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        grandchild_group.setItemVisibilityChecked(False)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [layer3, layer2, layer])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        grandchild_group.setItemVisibilityChecked(True)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [grandchild_group_layer, layer3, layer2, layer])
        self.assertEqual(grandchild_group_layer.childLayers(), [layer4])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        child_group_layer = child_group.convertToGroupLayer(options)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [child_group_layer, layer2, layer])
        self.assertEqual(child_group_layer.childLayers(), [grandchild_group_layer, layer3])
        self.assertEqual(grandchild_group_layer.childLayers(), [layer4])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        layer4_node.setItemVisibilityChecked(False)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [child_group_layer, layer2, layer])
        self.assertEqual(child_group_layer.childLayers(), [grandchild_group_layer, layer3])
        self.assertEqual(grandchild_group_layer.childLayers(), [])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        layer4_node.setItemVisibilityChecked(True)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [child_group_layer, layer2, layer])
        self.assertEqual(child_group_layer.childLayers(), [grandchild_group_layer, layer3])
        self.assertEqual(grandchild_group_layer.childLayers(), [layer4])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        grandchild_group.setItemVisibilityChecked(False)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [child_group_layer, layer2, layer])
        self.assertEqual(child_group_layer.childLayers(), [layer3])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        grandchild_group.setItemVisibilityChecked(True)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [child_group_layer, layer2, layer])
        self.assertEqual(child_group_layer.childLayers(), [grandchild_group_layer, layer3])
        self.assertEqual(grandchild_group_layer.childLayers(), [layer4])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        layer3_node.setItemVisibilityChecked(False)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [child_group_layer, layer2, layer])
        self.assertEqual(child_group_layer.childLayers(), [grandchild_group_layer])
        self.assertEqual(grandchild_group_layer.childLayers(), [layer4])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        layer3_node.setItemVisibilityChecked(True)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [child_group_layer, layer2, layer])
        self.assertEqual(child_group_layer.childLayers(), [grandchild_group_layer, layer3])
        self.assertEqual(grandchild_group_layer.childLayers(), [layer4])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        grandchild_group.setGroupLayer(None)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [child_group_layer, layer2, layer])
        self.assertEqual(child_group_layer.childLayers(), [layer4, layer3])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        child_group.setGroupLayer(None)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [group_layer])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [group_layer])
        self.assertEqual(group_layer.childLayers(), [layer4, layer3, layer2, layer])
        self.assertGreater(len(spy), spy_count)
        spy_count = len(spy)

        group_node.setGroupLayer(None)
        self.assertEqual(p.layerTreeRoot().layerOrder(), [layer, layer2, layer3, layer4])
        self.assertEqual(p.layerTreeRoot().checkedLayers(), [layer, layer2, layer3, layer4])
        self.assertGreater(len(spy), spy_count)


if __name__ == '__main__':
    unittest.main()
