"""QGIS Unit tests for QgsLayerTreeRegistryBridge

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Jean Felder"
__date__ = "05/02/2025"
__copyright__ = "Copyright 2025, The QGIS Project"

from qgis.core import (
    QgsLayerTreeRegistryBridge,
    QgsLayerTree,
    QgsProject,
    QgsLayerTreeGroup,
)

import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLayerTreeRegistryBridge(QgisTestCase):

    def test_constructor(self):
        project = QgsProject()
        root_group = QgsLayerTree()
        bridge = QgsLayerTreeRegistryBridge(root_group, project)

        self.assertTrue(bridge.isEnabled())
        self.assertTrue(bridge.newLayersVisible())
        self.assertEqual(bridge.layerInsertionPoint().group, root_group)
        self.assertEqual(bridge.layerInsertionPoint().position, 0)

    def test_insertion_point(self):
        project = QgsProject()
        root_group = QgsLayerTree()
        bridge = QgsLayerTreeRegistryBridge(root_group, project)

        self.assertEqual(bridge.layerInsertionPoint().group, root_group)
        self.assertEqual(bridge.layerInsertionPoint().position, 0)

        group_node = QgsLayerTreeGroup()
        root_group.addChildNode(group_node)
        bridge.setLayerInsertionPoint(
            QgsLayerTreeRegistryBridge.InsertionPoint(group_node, 4)
        )
        self.assertEqual(bridge.layerInsertionPoint().group, group_node)
        self.assertEqual(bridge.layerInsertionPoint().position, 4)

        bridge.setLayerInsertionPoint(
            QgsLayerTreeRegistryBridge.InsertionPoint(root_group, 0)
        )
        self.assertEqual(bridge.layerInsertionPoint().group, root_group)
        self.assertEqual(bridge.layerInsertionPoint().position, 0)


if __name__ == "__main__":
    unittest.main()
