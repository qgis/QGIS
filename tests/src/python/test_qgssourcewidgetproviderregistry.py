"""QGIS Unit tests for QgsProviderSourceWidgetProviderRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "23/12/2020"
__copyright__ = "Copyright 2020, The QGIS Project"


from qgis.core import QgsVectorLayer
from qgis.gui import (
    QgsGui,
    QgsProviderSourceWidget,
    QgsProviderSourceWidgetProvider,
)
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestSourceWidget(QgsProviderSourceWidget):

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("my_custom_widget")

    def setSourceUri(self, uri):
        pass

    def sourceUri(self):
        return ""


class TestProvider(QgsProviderSourceWidgetProvider):

    def __init__(self, name):
        super().__init__()
        self._name = name

    def providerKey(self):
        return self._name

    def canHandleLayer(self, layer):
        return layer.name() == "layer_for_this_provider"

    def createWidget(self, layer, parent):
        return TestSourceWidget(parent)


class TestQgsProviderSourceWidgetProviderRegistry(QgisTestCase):

    def testGuiRegistry(self):
        # ensure there is an application instance
        self.assertIsNotNone(QgsGui.sourceWidgetProviderRegistry())

    def testRegistry(self):
        registry = QgsGui.sourceWidgetProviderRegistry()
        initial_providers = registry.providers()
        self.assertTrue(initial_providers)  # we expect a bunch of default providers
        self.assertTrue([p.name() for p in initial_providers if p.name() == "xyz"])

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

        registry = QgsGui.sourceWidgetProviderRegistry()
        # self.assertIsNotNone(registry.providerByName('WFS'))
        self.assertIsNone(registry.providerByName("i_do_not_exist"))
        self.assertEqual(registry.providerByName("gdal").providerKey(), "gdal")

    def testCreateDialogWithCustomImplementation(self):
        """Tests that createWidget() returns a custom implementation"""

        registry = QgsGui.sourceWidgetProviderRegistry()
        p1 = TestProvider("p1")
        try:
            registry.addProvider(p1)

            vl = QgsVectorLayer(
                "Polygon?crs=epsg:4326&field=id:int",
                "layer_for_this_provider",
                "memory",
            )
            self.assertIsNotNone(registry.createWidget(vl))
            self.assertEqual(
                registry.createWidget(vl).objectName(), TestSourceWidget().objectName()
            )
        finally:
            registry.removeProvider(p1)


if __name__ == "__main__":
    unittest.main()
