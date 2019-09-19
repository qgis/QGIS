# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayerTreeView.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '02.04.2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

import os

from qgis.core import (
    QgsLayerTreeModel,
    QgsProject,
    QgsVectorLayer,
    QgsLayerTreeLayer,
    QgsLayerTree,
)
from qgis.gui import (
    QgsLayerTreeView,
    QgsLayerTreeViewDefaultActions,
)
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)
from qgis.PyQt.QtCore import QStringListModel
from qgis.PyQt.QtTest import QSignalSpy

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayerTreeView(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

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

    def nodeOrder(self, group):
        nodeorder = []
        layerTree = QgsLayerTree()
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
        self.assertEqual(view.model(), self.model)

    def testSetCurrentLayer(self):
        view = QgsLayerTreeView()
        view.setModel(self.model)
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
        actions = QgsLayerTreeViewDefaultActions(view)

        # show in overview action
        view.setCurrentLayer(self.layer)
        self.assertEqual(view.currentNode().customProperty('overview', 0), False)
        show_in_overview = actions.actionShowInOverview()
        show_in_overview.trigger()
        self.assertEqual(view.currentNode().customProperty('overview', 0), True)
        show_in_overview.trigger()
        self.assertEqual(view.currentNode().customProperty('overview', 0), False)

    def testMoveOutOfGroupActionLayer(self):
        """Test move out of group action on layer"""
        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().addGroup("embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
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
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.project.layerTreeRoot().layerOrder(), [self.layer, self.layer2, self.layer3])
        view.setCurrentLayer(self.layer3)
        movetotop = actions.actionMoveToTop()
        movetotop.trigger()
        self.assertEqual(self.project.layerTreeRoot().layerOrder(), [self.layer3, self.layer, self.layer2])

    def testMoveToTopActionGroup(self):
        """Test move to top action on group"""
        view = QgsLayerTreeView()
        group = self.project.layerTreeRoot().addGroup("embeddedgroup")
        group.addLayer(self.layer4)
        group.addLayer(self.layer5)
        groupname = group.name()
        view.setModel(self.model)
        actions = QgsLayerTreeViewDefaultActions(view)
        self.assertEqual(self.nodeOrder(self.project.layerTreeRoot().children()), [
            self.layer.name(),
            self.layer2.name(),
            self.layer3.name(),
            groupname,
            groupname + '-' + self.layer4.name(),
            groupname + '-' + self.layer5.name(),
        ])

        nodeLayerIndex = self.model.node2index(group)
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

    def testSetLayerVisible(self):
        view = QgsLayerTreeView()
        view.setModel(self.model)
        self.project.layerTreeRoot().findLayer(self.layer).setItemVisibilityChecked(True)
        self.project.layerTreeRoot().findLayer(self.layer2).setItemVisibilityChecked(True)
        self.assertTrue(self.project.layerTreeRoot().findLayer(self.layer).itemVisibilityChecked())
        self.assertTrue(self.project.layerTreeRoot().findLayer(self.layer2).itemVisibilityChecked())

        view.setLayerVisible(None, True)
        view.setLayerVisible(self.layer, True)
        self.assertTrue(self.project.layerTreeRoot().findLayer(self.layer).itemVisibilityChecked())
        view.setLayerVisible(self.layer2, False)
        self.assertFalse(self.project.layerTreeRoot().findLayer(self.layer2).itemVisibilityChecked())
        view.setLayerVisible(self.layer2, True)
        self.assertTrue(self.project.layerTreeRoot().findLayer(self.layer2).itemVisibilityChecked())


if __name__ == '__main__':
    unittest.main()
