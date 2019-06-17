# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayerTreeMapCanvasBridge.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '8/03/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import os

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsApplication,
                       QgsUnitTypes,
                       QgsCoordinateReferenceSystem,
                       QgsVectorLayer)
from qgis.gui import (QgsLayerTreeMapCanvasBridge,
                      QgsMapCanvas,
                      QgsCustomLayerOrderWidget)
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayerTreeMapCanvasBridge(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

    def testLayerOrderUpdatedThroughBridge(self):
        """ test that project layer order is updated when layer tree changes """

        prj = QgsProject.instance()
        prj.clear()
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer3", "memory")

        prj.addMapLayers([layer, layer2, layer3])

        canvas = QgsMapCanvas()
        bridge = QgsLayerTreeMapCanvasBridge(prj.layerTreeRoot(), canvas)

        #custom layer order
        prj.layerTreeRoot().setHasCustomLayerOrder(True)
        prj.layerTreeRoot().setCustomLayerOrder([layer3, layer, layer2])
        app.processEvents()
        self.assertEqual([l for l in prj.layerTreeRoot().customLayerOrder()], [layer3, layer, layer2])
        self.assertEqual([l for l in prj.layerTreeRoot().layerOrder()], [layer3, layer, layer2])

        # no custom layer order
        prj.layerTreeRoot().setHasCustomLayerOrder(False)
        app.processEvents()
        self.assertEqual([l for l in prj.layerTreeRoot().customLayerOrder()], [layer3, layer, layer2])
        self.assertEqual([l for l in prj.layerTreeRoot().layerOrder()], [layer, layer2, layer3])

        # mess around with the layer tree order
        root = prj.layerTreeRoot()
        layer_node = root.findLayer(layer2)
        cloned_node = layer_node.clone()
        parent = layer_node.parent()
        parent.insertChildNode(0, cloned_node)
        parent.removeChildNode(layer_node)
        app.processEvents()
        # make sure project respects this
        self.assertEqual([l for l in prj.layerTreeRoot().layerOrder()], [layer2, layer, layer3])

        # make sure project order includes ALL layers, not just visible ones
        layer_node = root.findLayer(layer)
        layer_node.setItemVisibilityChecked(False)
        app.processEvents()
        self.assertEqual([l for l in prj.layerTreeRoot().layerOrder()], [layer2, layer, layer3])

    def testCustomLayerOrderUpdatedFromProject(self):
        """ test that setting project layer order is reflected in custom layer order panel """

        prj = QgsProject.instance()
        prj.clear()
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer3", "memory")
        prj.addMapLayers([layer, layer2, layer3])

        canvas = QgsMapCanvas()
        bridge = QgsLayerTreeMapCanvasBridge(prj.layerTreeRoot(), canvas)
        custom_order_widget = QgsCustomLayerOrderWidget(bridge)

        #custom layer order
        prj.layerTreeRoot().setHasCustomLayerOrder(True)
        prj.layerTreeRoot().setCustomLayerOrder([layer3, layer, layer2])
        app.processEvents()
        self.assertEqual([l for l in prj.layerTreeRoot().customLayerOrder()], [layer3, layer, layer2])

        # no custom layer order
        prj.layerTreeRoot().setHasCustomLayerOrder(False)
        app.processEvents()
        self.assertEqual([l for l in prj.layerTreeRoot().layerOrder()], [layer, layer2, layer3])

        # mess around with the project layer order
        prj.layerTreeRoot().setCustomLayerOrder([layer3, layer, layer2])
        app.processEvents()
        self.assertEqual(prj.layerTreeRoot().layerOrder(), [layer, layer2, layer3])

        # try reordering through bridge
        prj.layerTreeRoot().setHasCustomLayerOrder(False)
        app.processEvents()
        self.assertEqual([l for l in prj.layerTreeRoot().layerOrder()], [layer, layer2, layer3])
        root = prj.layerTreeRoot()
        layer_node = root.findLayer(layer2)
        cloned_node = layer_node.clone()
        parent = layer_node.parent()
        parent.insertChildNode(0, cloned_node)
        parent.removeChildNode(layer_node)
        app.processEvents()
        # make sure project respects this
        self.assertEqual([l for l in prj.layerTreeRoot().layerOrder()], [layer2, layer, layer3])
        self.assertFalse(prj.layerTreeRoot().hasCustomLayerOrder())

    def testNonSpatialLayer(self):
        """ test that non spatial layers are not passed to canvas """

        prj = QgsProject.instance()
        prj.clear()
        layer = QgsVectorLayer("Point?field=fldtxt:string",
                               "layer1", "memory")
        layer2 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer2", "memory")
        layer3 = QgsVectorLayer("Point?field=fldtxt:string",
                                "layer3", "memory")
        non_spatial = QgsVectorLayer("None?field=fldtxt:string",
                                     "non_spatial", "memory")

        prj.addMapLayers([layer, layer2, layer3, non_spatial])

        canvas = QgsMapCanvas()
        bridge = QgsLayerTreeMapCanvasBridge(prj.layerTreeRoot(), canvas)

        #custom layer order
        prj.layerTreeRoot().setHasCustomLayerOrder(True)
        prj.layerTreeRoot().setCustomLayerOrder([layer3, layer, layer2])
        app.processEvents()
        self.assertEqual(canvas.mapSettings().layers(), [layer3, layer, layer2])

        # with non-spatial (should not be possible through ui, but is through api)
        prj.layerTreeRoot().setCustomLayerOrder([layer3, layer, layer2, non_spatial])
        app.processEvents()
        #self.assertEqual(canvas.mapSettings().layers(),[layer3,layer,layer2])

        # no custom layer order
        prj.layerTreeRoot().setHasCustomLayerOrder(False)
        app.processEvents()
        self.assertEqual(canvas.mapSettings().layers(), [layer, layer2, layer3])


if __name__ == '__main__':
    unittest.main()
