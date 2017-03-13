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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

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

    def testCustomLayerOrderUpdatedFromProject(self):
        """ test that setting project layer order is reflected in custom layer order panel """

        prj = QgsProject.instance()
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
        bridge.setHasCustomLayerOrder(True)
        bridge.setCustomLayerOrder([layer3.id(), layer.id(), layer2.id()])
        app.processEvents()
        self.assertEqual([l.id() for l in prj.layerOrder()], [layer3.id(), layer.id(), layer2.id()])

        # no custom layer order
        bridge.setHasCustomLayerOrder(False)
        app.processEvents()
        self.assertEqual([l.id() for l in prj.layerOrder()], [layer.id(), layer2.id(), layer3.id()])

        # mess around with the project layer order
        prj.setLayerOrder([layer3, layer, layer2])
        app.processEvents()
        # make sure bridge respects this new order
        self.assertTrue(bridge.hasCustomLayerOrder())
        self.assertEqual(bridge.customLayerOrder(), [layer3.id(), layer.id(), layer2.id()])

        # try reordering through bridge
        bridge.setHasCustomLayerOrder(False)
        app.processEvents()
        self.assertEqual([l.id() for l in prj.layerOrder()], [layer.id(), layer2.id(), layer3.id()])
        root = prj.layerTreeRoot()
        layer_node = root.findLayer(layer2.id())
        cloned_node = layer_node.clone()
        parent = layer_node.parent()
        parent.insertChildNode(0, cloned_node)
        parent.removeChildNode(layer_node)
        app.processEvents()
        # make sure project respects this
        self.assertEqual([l.id() for l in prj.layerOrder()], [layer2.id(), layer.id(), layer3.id()])
        self.assertFalse(bridge.hasCustomLayerOrder())


if __name__ == '__main__':
    unittest.main()
