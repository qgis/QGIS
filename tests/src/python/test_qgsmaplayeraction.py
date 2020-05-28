# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMapLayerAction.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '24/02/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA switch sip api

from qgis.core import (QgsVectorLayer,
                       QgsRasterLayer,
                       QgsMapLayer)
from qgis.gui import (QgsMapLayerActionRegistry,
                      QgsMapLayerAction)

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

import os

start_app()


class TestQgsMapLayerAction(unittest.TestCase):

    def __init__(self, methodName):
        """Run once on class initialization."""
        unittest.TestCase.__init__(self, methodName)

        self.vector_layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=flddate:datetime",
                                           "test_layer", "memory")
        assert self.vector_layer.isValid()
        self.vector_layer2 = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=flddate:datetime",
                                            "test_layer", "memory")
        assert self.vector_layer2.isValid()
        raster_path = os.path.join(unitTestDataPath(), 'landsat.tif')
        self.raster_layer = QgsRasterLayer(raster_path, 'raster')
        assert self.raster_layer.isValid()

    def testCanRunUsingLayer(self):
        """
        Test that actions correctly indicate when they can run for a layer
        """
        action_all_layers = QgsMapLayerAction('action1', None)
        self.assertTrue(action_all_layers.canRunUsingLayer(None))
        self.assertTrue(action_all_layers.canRunUsingLayer(self.vector_layer))
        self.assertTrue(action_all_layers.canRunUsingLayer(self.raster_layer))

        action_vector_layers_only = QgsMapLayerAction('action2', None, QgsMapLayer.VectorLayer)
        self.assertFalse(action_vector_layers_only.canRunUsingLayer(None))
        self.assertTrue(action_vector_layers_only.canRunUsingLayer(self.vector_layer))
        self.assertFalse(action_vector_layers_only.canRunUsingLayer(self.raster_layer))

        action_raster_layers_only = QgsMapLayerAction('action3', None, QgsMapLayer.RasterLayer)
        self.assertFalse(action_raster_layers_only.canRunUsingLayer(None))
        self.assertFalse(action_raster_layers_only.canRunUsingLayer(self.vector_layer))
        self.assertTrue(action_raster_layers_only.canRunUsingLayer(self.raster_layer))

        action_specific_layer_only = QgsMapLayerAction('action4', None, self.vector_layer)
        self.assertFalse(action_specific_layer_only.canRunUsingLayer(None))
        self.assertTrue(action_specific_layer_only.canRunUsingLayer(self.vector_layer))
        self.assertFalse(action_specific_layer_only.canRunUsingLayer(self.vector_layer2))
        self.assertFalse(action_specific_layer_only.canRunUsingLayer(self.raster_layer))

        action_specific_raster_layer_only = QgsMapLayerAction('action4', None, self.raster_layer)
        self.assertFalse(action_specific_raster_layer_only.canRunUsingLayer(None))
        self.assertFalse(action_specific_raster_layer_only.canRunUsingLayer(self.vector_layer))
        self.assertFalse(action_specific_raster_layer_only.canRunUsingLayer(self.vector_layer2))
        self.assertTrue(action_specific_raster_layer_only.canRunUsingLayer(self.raster_layer))

        action_editable_layer_only = QgsMapLayerAction('action1', None, flags=QgsMapLayerAction.EnabledOnlyWhenEditable)
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(None))
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(self.vector_layer))
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(self.vector_layer2))
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(self.raster_layer))
        self.vector_layer.startEditing()
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(None))
        self.assertTrue(action_editable_layer_only.canRunUsingLayer(self.vector_layer))
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(self.vector_layer2))
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(self.raster_layer))
        self.vector_layer.commitChanges()
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(None))
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(self.vector_layer))
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(self.vector_layer2))
        self.assertFalse(action_editable_layer_only.canRunUsingLayer(self.raster_layer))


if __name__ == '__main__':
    unittest.main()
