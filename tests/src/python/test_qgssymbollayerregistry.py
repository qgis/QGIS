# -*- coding: utf-8 -*-
"""QGIS Unit tests

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Denis Rouzaud'
__date__ = '26/11/2021'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA
from qgis.core import Qgis, QgsApplication, QgsSymbolLayerAbstractMetadata, QgsSymbolLayer, QgsSimpleMarkerSymbolLayer
from qgis.testing import start_app, unittest
from qgis.PyQt import sip


start_app()


class MySuperMarkerMetadata(QgsSymbolLayerAbstractMetadata):
    def __init__(self):
        super(MySuperMarkerMetadata, self).__init__('MySuperMarker', 'My Super Marker', Qgis.SymbolType.Marker)

    def createSymbolLayer(self, map: dict) -> QgsSymbolLayer:
        return QgsSimpleMarkerSymbolLayer()


class TestQgsSymbolLayerRegistry(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """
        Setup the involved layers and relations for a n:m relation
        :return:
        """
        cls.registry = QgsApplication.instance().symbolLayerRegistry()

    def testCreateInstance(self):
        """Test registry creation"""
        self.assertTrue(self.registry)

    def testAddRemoveSymbolLayer(self):
        """Test adding/removing layers to the registry"""
        n = len(self.registry.symbolLayersForType(Qgis.SymbolType.Marker))
        metadata = MySuperMarkerMetadata()
        # add layer
        self.assertTrue(self.registry.addSymbolLayerType(metadata))
        # layer already added
        self.assertFalse(self.registry.addSymbolLayerType(metadata))
        # check there is one more layer
        self.assertEqual(len(self.registry.symbolLayersForType(Qgis.SymbolType.Marker)), n + 1)
        # remove layer
        self.assertTrue(self.registry.removeSymbolLayerType(metadata))
        # check layer has been deleted
        self.assertTrue(sip.isdeleted(metadata))
        # already removed
        self.assertFalse(self.registry.removeSymbolLayerType(MySuperMarkerMetadata()))

    def testOwnership(self):
        """
        Test that registered layers do not require that a reference to them is kept.
        They should be parented to the registry (on transfer) and even if there's no reference
        to the registry around (see the `del` below) this childship should continue to exist.
        """
        metadata = MySuperMarkerMetadata()
        self.assertTrue(self.registry.addSymbolLayerType(metadata))
        del metadata
        self.assertIn(MySuperMarkerMetadata().name(), self.registry.symbolLayersForType(Qgis.SymbolType.Marker))
        self.assertTrue(self.registry.removeSymbolLayerType(MySuperMarkerMetadata()))


if __name__ == '__main__':
    unittest.main()
