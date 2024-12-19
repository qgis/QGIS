"""QGIS Unit tests for QgsLayerTreeView.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '02.04.2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

from qgis.PyQt.QtCore import QStringListModel, QItemSelectionModel
from qgis.PyQt.QtTest import QAbstractItemModelTester, QSignalSpy
from qgis.core import (
    QgsLayerTree,
    QgsLayerTreeModel,
    QgsProject,
    QgsVectorLayer,
    QgsCategorizedSymbolRenderer,
    QgsRendererCategory,
    QgsMarkerSymbol,
    QgsMapLayerLegend
)
from qgis.gui import QgsLayerTreeView, QgsLayerTreeViewDefaultActions
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayerTreeView(QgisTestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""

        QgisTestCase.__init__(self, methodName)

        # setup a dummy project
        self.project = QgsProject()
        self.layer = QgsVectorLayer("Point?field=fldtxt:string",
                                    "layer1", "memory")
        self.layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                     "layer2", "memory")
        self.layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                     "layer3", "memory")
        self.layer4 = QgsVectorLayer("Point?field=fldtxt:string",
                                     "layer4", "memory")
        self.layer5 = QgsVectorLayer("Point?field=fldtxt:string",
                                     "layer5", "memory")
        self.project.addMapLayers([self.layer, self.layer2, self.layer3])
        self.model = QgsLayerTreeModel(self.project.layerTreeRoot())
        self.tester = QAbstractItemModelTester(self.model)

        self.groupname = "group"
        self.subgroupname = "sub-group"

    def nodeOrder(self, group):

        nodeorder = []
        for node in group:
            if QgsLayerTree.isGroup(node):
                groupname = node.name()
                nodeorder.append(groupname)
                for child in self.nodeOrder(node.children()):
                    nodeorder.append(groupname + '-' + child)
            elif QgsLayerTree.isLayer(node):
                nodeorder.append(node.layer().name())
        return nodeorder

    def testSetModel(self):

        view = QgsLayerTreeView()

        # should not work
        string_list_model = QStringListModel()
        view.setModel(string_list_model)
        self.assertFalse(view.model())

        # should work
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        self.assertEqual(view.layerTreeModel(), self.model)

    def testSetCurrentLayer(self):

        view = QgsLayerTreeView()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        current_layer_changed_spy = QSignalSpy(view.currentLayerChanged)
        self.assertFalse(view.currentLayer())
        view.setCurrentLayer(self.layer3)
        self.assertEqual(view.currentLayer(), self.layer3)
        self.assertEqual(len(current_layer_changed_spy), 1)
        view.setCurrentLayer(self.layer)
        self.assertEqual(view.currentLayer(), self.layer)
        self.assertEqual(len(current_layer_changed_spy), 2)
        view.setCurrentLayer(None)
        self.assertFalse(view.currentLayer())
        self.assertEqual(len(current_layer_changed_spy), 3)

    def testDefaultActions(self):
        view = QgsLayerTreeView()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)

        # show in overview action
        view.setCurrentLayer(self.layer)
        self.assertEqual(
            view.currentNode().customProperty('overview', 0), False)
        show_in_overview = actions.actionShowInOverview()
        show_in_overview.trigger()
        self.assertEqual(
            view.currentNode().customProperty('overview', 0), True)
        show_in_overview.trigger()
        self.assertEqual(
            view.currentNode().customProperty('overview', 0), False)

    def testMoveOutOfGroupActionLayer(self):
        """Test move out of group action on layer"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().addGroup("embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
        ])

        view.setCurrentLayer(self.layer5)
        moveOutOfGroup = actions.actionMoveOutOfGroup()
        moveOutOfGroup.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            self.layer5.name(),
            groupname,
            groupname + '-' + self.layer4.name(),
        ])

    def testMoveToTopActionLayer(self):
        """Test move to top action on layer"""

        view = QgsLayerTreeView()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.project.layerTreeRoot().layerOrder(), [
                         self.layer, self.layer2, self.layer3])
        view.setCurrentLayer(self.layer3)
        movetotop = actions.actionMoveToTop()
        movetotop.trigger()
        self.assertEqual(self.project.layerTreeRoot().layerOrder(), [
                         self.layer3, self.layer, self.layer2])

    def testMoveToTopActionGroup(self):
        """Test move to top action on group"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().addGroup("embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
        ])

        nodeLayerIndex = view.node2index(group)
        view.setCurrentIndex(nodeLayerIndex)
        movetotop = actions.actionMoveToTop()
        movetotop.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
        ])

    def testMoveToTopActionEmbeddedGroup(self):
        """Test move to top action on embeddedgroup layer"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().addGroup("embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
        ])

        view.setCurrentLayer(self.layer5)
        movetotop = actions.actionMoveToTop()
        movetotop.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            groupname,
            groupname + '-' + self.layer5.name(),
            groupname + '-' + self.layer4.name(),
        ])

    def testMoveToTopActionLayerAndGroup(self):
        """Test move to top action for a group and it's layer simultaneously"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().addGroup("embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
        ])

        selectionMode = view.selectionMode()
        view.setSelectionMode(QgsLayerTreeView.SelectionMode.MultiSelection)
        nodeLayerIndex = view.node2index(group)
        view.setCurrentIndex(nodeLayerIndex)
        view.setCurrentLayer(self.layer5)
        view.setSelectionMode(selectionMode)
        movetotop = actions.actionMoveToTop()
        movetotop.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname,
            groupname + '-' + self.layer5.name(),
            groupname + '-' + self.layer4.name(),
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
        ])

    def testMoveToBottomActionLayer(self):
        """Test move to bottom action on layer"""

        view = QgsLayerTreeView()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.project.layerTreeRoot().layerOrder(), [
                         self.layer, self.layer2, self.layer3])
        view.setCurrentLayer(self.layer)
        movetobottom = actions.actionMoveToBottom()
        movetobottom.trigger()
        self.assertEqual(self.project.layerTreeRoot().layerOrder(), [
                         self.layer2, self.layer3, self.layer])

    def testMoveToBottomActionGroup(self):
        """Test move to bottom action on group"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().insertGroup(0, "embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
        ])

        nodeLayerIndex = view.node2index(group)
        view.setCurrentIndex(nodeLayerIndex)
        movetobottom = actions.actionMoveToBottom()
        movetobottom.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
        ])

    def testMoveToBottomActionEmbeddedGroup(self):
        """Test move to bottom action on embeddedgroup layer"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().addGroup("embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
        ])

        view.setCurrentLayer(self.layer4)
        movetobottom = actions.actionMoveToBottom()
        movetobottom.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            groupname,
            groupname + '-' + self.layer5.name(),
            groupname + '-' + self.layer4.name(),
        ])

    def testMoveToBottomActionLayerAndGroup(self):
        """Test move to top action for a group and it's layer simultaneously"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().insertGroup(0, "embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
        ])

        selectionMode = view.selectionMode()
        view.setSelectionMode(QgsLayerTreeView.SelectionMode.MultiSelection)
        nodeLayerIndex = view.node2index(group)
        view.setCurrentIndex(nodeLayerIndex)
        view.setCurrentLayer(self.layer4)
        view.setSelectionMode(selectionMode)
        movetobottom = actions.actionMoveToBottom()
        movetobottom.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            groupname,
            groupname + '-' + self.layer5.name(),
            groupname + '-' + self.layer4.name(),
        ])

    def testAddGroupActionLayer(self):
        """Test add group action on single layer"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().insertGroup(0, "embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
        ])

        view.setCurrentLayer(self.layer2)
        addgroup = actions.actionAddGroup()
        addgroup.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
            self.layer.name(),
            self.groupname + '1',
            self.groupname + '1' + '-' + self.layer2.name(),
            self.layer3.name()
        ])

    def testAddGroupActionLayers(self):
        """Test add group action on several layers"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().insertGroup(0, "embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()

        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
        ])

        selectionMode = view.selectionMode()
        view.setSelectionMode(QgsLayerTreeView.SelectionMode.MultiSelection)
        view.setCurrentLayer(self.layer)
        view.setCurrentLayer(self.layer2)
        view.setSelectionMode(selectionMode)

        addgroup = actions.actionAddGroup()
        addgroup.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
            self.groupname + '1',
            self.groupname + '1' + '-' + self.layer.name(),
            self.groupname + '1' + '-' + self.layer2.name(),
            self.layer3.name()
        ])

    def testAddGroupActionGroup(self):
        """Test add group action on single group"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().insertGroup(0, "embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
        ])

        nodeLayerIndex = view.node2index(group)
        view.setCurrentIndex(nodeLayerIndex)
        addgroup = actions.actionAddGroup()
        addgroup.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
            groupname + '-' + self.subgroupname + '1',
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name()
        ])

    def testAddGroupActionGroups(self):
        """Test add group action on several groups"""

        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().insertGroup(0, "embeddedgroup")
        group.addLayer(self.layer4)
        groupname = group.name()
        group2 = self.project.layerTreeRoot().insertGroup(0, "embeddedgroup2")
        group2.addLayer(self.layer5)
        groupname2 = group2.name()

        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            groupname2,
            groupname2 + '-' + self.layer5.name(),
            groupname,
            groupname + '-' + self.layer4.name(),
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
        ])

        selectionMode = view.selectionMode()
        view.setSelectionMode(QgsLayerTreeView.SelectionMode.MultiSelection)
        nodeLayerIndex = view.node2index(group)
        view.setCurrentIndex(nodeLayerIndex)
        nodeLayerIndex2 = view.node2index(group2)
        view.setCurrentIndex(nodeLayerIndex2)
        view.setSelectionMode(selectionMode)

        addgroup = actions.actionAddGroup()
        addgroup.trigger()
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.groupname + '1',
            self.groupname + '1' + '-' + groupname,
            self.groupname + '1' + '-' + groupname + '-' + self.layer4.name(),
            self.groupname + '1' + '-' + groupname2,
            self.groupname + '1' + '-' + groupname2 + '-' + self.layer5.name(),
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name()
        ])

    def testSetLayerVisible(self):

        view = QgsLayerTreeView()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        self.project.layerTreeRoot().findLayer(
            self.layer).setItemVisibilityChecked(True)
        self.project.layerTreeRoot().findLayer(
            self.layer2).setItemVisibilityChecked(True)
        self.assertTrue(self.project.layerTreeRoot().findLayer(
            self.layer).itemVisibilityChecked())
        self.assertTrue(self.project.layerTreeRoot().findLayer(
            self.layer2).itemVisibilityChecked())

        view.setLayerVisible(None, True)
        view.setLayerVisible(self.layer, True)
        self.assertTrue(self.project.layerTreeRoot().findLayer(
            self.layer).itemVisibilityChecked())
        view.setLayerVisible(self.layer2, False)
        self.assertFalse(self.project.layerTreeRoot().findLayer(
            self.layer2).itemVisibilityChecked())
        view.setLayerVisible(self.layer2, True)
        self.assertTrue(self.project.layerTreeRoot().findLayer(
            self.layer2).itemVisibilityChecked())

    def testProxyModel(self):
        """Test proxy model filtering and private layers"""

        view = QgsLayerTreeView()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        tree_model = view.layerTreeModel()
        proxy_model = view.proxyModel()

        self.assertEqual(tree_model.rowCount(), 3)
        self.assertEqual(proxy_model.rowCount(), 3)

        items = []
        for r in range(tree_model.rowCount()):
            items.append(tree_model.data(tree_model.index(r, 0)))

        self.assertEqual(items, ['layer1', 'layer2', 'layer3'])

        proxy_items = []
        for r in range(proxy_model.rowCount()):
            proxy_items.append(proxy_model.data(proxy_model.index(r, 0)))

        self.assertEqual(proxy_items, ['layer1', 'layer2', 'layer3'])

        self.layer3.setFlags(self.layer.Private)
        self.assertEqual(tree_model.rowCount(), 3)
        self.assertEqual(proxy_model.rowCount(), 2)

        proxy_items = []
        for r in range(proxy_model.rowCount()):
            proxy_items.append(proxy_model.data(proxy_model.index(r, 0)))

        self.assertEqual(proxy_items, ['layer1', 'layer2'])

        view.setShowPrivateLayers(True)

        self.assertEqual(proxy_model.rowCount(), 3)

        proxy_items = []
        for r in range(proxy_model.rowCount()):
            proxy_items.append(proxy_model.data(proxy_model.index(r, 0)))

        self.assertEqual(proxy_items, ['layer1', 'layer2', 'layer3'])

        view.setShowPrivateLayers(False)

        self.assertEqual(proxy_model.rowCount(), 2)

        proxy_items = []
        for r in range(proxy_model.rowCount()):
            proxy_items.append(proxy_model.data(proxy_model.index(r, 0)))

        self.assertEqual(proxy_items, ['layer1', 'layer2'])

        # Test filters
        proxy_model.setFilterText('layer2')

        self.assertEqual(proxy_model.rowCount(), 1)

        proxy_items = []
        for r in range(proxy_model.rowCount()):
            proxy_items.append(proxy_model.data(proxy_model.index(r, 0)))

        self.assertEqual(proxy_items, ['layer2'])

        # test valid layer filtering
        broken_layer = QgsVectorLayer("xxxx", "broken", "ogr")
        self.assertFalse(broken_layer.isValid())
        self.project.addMapLayers([broken_layer])

        proxy_model.setFilterText(None)

        proxy_items = []
        for r in range(proxy_model.rowCount()):
            proxy_items.append(proxy_model.data(proxy_model.index(r, 0)))
        self.assertEqual(proxy_items, ['broken', 'layer1', 'layer2'])

        proxy_model.setHideValidLayers(True)

        proxy_items = []
        for r in range(proxy_model.rowCount()):
            proxy_items.append(proxy_model.data(proxy_model.index(r, 0)))
        self.assertEqual(proxy_items, ['broken'])

        proxy_model.setHideValidLayers(False)

        proxy_items = []
        for r in range(proxy_model.rowCount()):
            proxy_items.append(proxy_model.data(proxy_model.index(r, 0)))
        self.assertEqual(proxy_items, ['broken', 'layer1', 'layer2'])

        self.project.removeMapLayer(broken_layer)

    def testProxyModelCurrentIndex(self):
        """Test a crash spotted out while developing the proxy model"""

        view = QgsLayerTreeView()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        tree_tester = QAbstractItemModelTester(view.layerTreeModel())

        view.setCurrentLayer(self.layer3)
        self.layer3.setFlags(self.layer.Private)

    def testNode2IndexMethods(self):
        """Test node2index and node2sourceIndex"""

        view = QgsLayerTreeView()
        view.setModel(self.model)
        proxy_tester = QAbstractItemModelTester(view.model())
        tree_tester = QAbstractItemModelTester(view.layerTreeModel())

        tree_model = view.layerTreeModel()
        proxy_model = view.proxyModel()

        proxy_index = proxy_model.index(1, 0)
        node2 = view.index2node(proxy_index)
        self.assertEqual(node2.name(), 'layer2')

        proxy_layer2_index = view.node2index(node2)
        self.assertEqual(proxy_layer2_index, view.node2index(node2))

        source_index = tree_model.index(1, 0)
        tree_layer2_index = view.node2sourceIndex(node2)
        self.assertEqual(tree_layer2_index, view.node2sourceIndex(node2))

    def test_selected_legend_nodes(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")

        cat1 = QgsRendererCategory(1, QgsMarkerSymbol.createSimple({}), 'cat 1')
        cat2 = QgsRendererCategory(2, QgsMarkerSymbol.createSimple({}),
                                   'cat 2')
        cat3 = QgsRendererCategory(1, QgsMarkerSymbol.createSimple({}),
                                   'cat 3')

        renderer = QgsCategorizedSymbolRenderer('fldtext', [cat1, cat2, cat3])
        layer.setRenderer(renderer)
        layer.setLegend(QgsMapLayerLegend.defaultVectorLegend(layer))

        root = QgsLayerTree()
        model = QgsLayerTreeModel(root)
        layer_tree_layer = root.addLayer(layer)
        view = QgsLayerTreeView()
        view.setModel(model)

        legend_nodes = model.layerLegendNodes(layer_tree_layer)
        self.assertEqual(len(legend_nodes), 3)

        index = model.legendNode2index(legend_nodes[0])
        self.assertTrue(index.isValid())
        self.assertEqual(model.index2legendNode(index), legend_nodes[0])
        index2 = model.legendNode2index(legend_nodes[2])
        self.assertTrue(index2.isValid())
        self.assertEqual(model.index2legendNode(index2), legend_nodes[2])

        self.assertFalse(view.selectedLegendNodes())

        view.selectionModel().select(view.proxyModel().mapFromSource(index), QItemSelectionModel.SelectionFlag.ClearAndSelect)
        view.selectionModel().select(view.proxyModel().mapFromSource(index2), QItemSelectionModel.SelectionFlag.Select)

        self.assertCountEqual(view.selectedLegendNodes(), [legend_nodes[0], legend_nodes[2]])


if __name__ == '__main__':
    unittest.main()
