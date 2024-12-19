"""QGIS Unit tests for QgsSubsetStringEditorProviderRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Even Rouault"
__date__ = "15/11/2020"
__copyright__ = "Copyright 2018, The QGIS Project"

from qgis.PyQt.QtCore import Qt
from qgis.core import QgsVectorLayer
from qgis.gui import (
    QgsGui,
    QgsQueryBuilder,
    QgsSubsetStringEditorInterface,
    QgsSubsetStringEditorProvider,
)
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class SubsetStringDialog(QgsSubsetStringEditorInterface):

    def __init__(self, parent=None, fl=Qt.WindowFlags()):
        super().__init__(parent, fl)
        self.setObjectName("my_custom_dialog")


class TestProvider(QgsSubsetStringEditorProvider):

    def __init__(self, name):
        super().__init__()
        self._name = name

    def providerKey(self):
        return self._name

    def canHandleLayer(self, layer):
        return layer.name() == "layer_for_this_provider"

    def createDialog(self, layer, parent, fl):
        return SubsetStringDialog(parent, fl)


class TestQgsSubsetStringEditorProviderRegistry(QgisTestCase):

    def testGuiRegistry(self):
        # ensure there is an application instance
        self.assertIsNotNone(QgsGui.subsetStringEditorProviderRegistry())

    def testRegistry(self):
        registry = QgsGui.subsetStringEditorProviderRegistry()
        initial_providers = registry.providers()
        self.assertTrue(initial_providers)  # we expect a bunch of default providers
        self.assertTrue([p.name() for p in initial_providers if p.name() == "WFS"])

        # add a new provider
        p1 = TestProvider("p1")
        registry.addProvider(p1)
        self.assertIn(p1, registry.providers())

        p2 = TestProvider("p2")
        registry.addProvider(p2)
        self.assertIn(p1, registry.providers())
        self.assertIn(p2, registry.providers())

        registry.removeProvider(None)
        p3 = TestProvider("p3")
        # not in registry yet
        registry.removeProvider(p3)

        registry.removeProvider(p1)
        self.assertNotIn("p1", [p.name() for p in registry.providers()])
        self.assertIn(p2, registry.providers())

        registry.removeProvider(p2)
        self.assertNotIn("p2", [p.name() for p in registry.providers()])
        self.assertEqual(registry.providers(), initial_providers)

    def testProviderKey(self):
        """Tests finding provider by name and return providerKey"""

        registry = QgsGui.subsetStringEditorProviderRegistry()
        self.assertIsNotNone(registry.providerByName("WFS"))
        self.assertIsNone(registry.providerByName("i_do_not_exist"))
        self.assertEqual(registry.providerByName("WFS").providerKey(), "WFS")

    # Test disabled since there's a memory corruption issue with QgsQueryBuilder()
    # creation in non-GUI mode.
    def DISABLED_testCreateDialogWithDefaultImplementation(self):
        """Tests that createDialog() returns the default implementation when no provider kicks in"""

        registry = QgsGui.subsetStringEditorProviderRegistry()
        p1 = TestProvider("p1")
        try:
            registry.addProvider(p1)

            vl = QgsVectorLayer("Polygon?crs=epsg:4326&field=id:int", "test", "memory")
            self.assertIsNotNone(registry.createDialog(vl))
            self.assertEqual(
                registry.createDialog(vl).objectName(), QgsQueryBuilder(vl).objectName()
            )
        finally:
            registry.removeProvider(p1)

    def testCreateDialogWithCustomImplementation(self):
        """Tests that createDialog() returns a custom implementation"""

        registry = QgsGui.subsetStringEditorProviderRegistry()
        p1 = TestProvider("p1")
        try:
            registry.addProvider(p1)

            vl = QgsVectorLayer(
                "Polygon?crs=epsg:4326&field=id:int",
                "layer_for_this_provider",
                "memory",
            )
            self.assertIsNotNone(registry.createDialog(vl))
            self.assertEqual(
                registry.createDialog(vl).objectName(),
                SubsetStringDialog().objectName(),
            )
        finally:
            registry.removeProvider(p1)


if __name__ == "__main__":
    unittest.main()
