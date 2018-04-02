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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.core import (
    QgsLayerTreeModel,
    QgsProject,
    QgsVectorLayer
)
from qgis.gui import (QgsLayerTreeView,
                      QgsLayerTreeViewDefaultActions)
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
        self.project.addMapLayers([self.layer, self.layer2, self.layer3])
        self.model = QgsLayerTreeModel(self.project.layerTreeRoot())

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


if __name__ == '__main__':
    unittest.main()
