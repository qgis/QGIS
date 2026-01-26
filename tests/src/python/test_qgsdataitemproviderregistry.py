"""QGIS Unit tests for QgsDataItemProviderRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "27/10/2018"
__copyright__ = "Copyright 2018, The QGIS Project"


from qgis.core import (
    QgsApplication,
    QgsDataItemProvider,
    QgsDataItemProviderRegistry,
    QgsDataProvider,
)
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestProvider(QgsDataItemProvider):

    def __init__(self, name):
        super().__init__()
        self._name = name

    def name(self):
        return self._name

    def capabilities(self):
        return QgsDataProvider.DataCapability.File

    def createDataItem(self, path, parent):
        return None


class TestQgsDataItemProviderRegistry(QgisTestCase):

    def testAppRegistry(self):
        # ensure there is an application instance
        self.assertIsNotNone(QgsApplication.dataItemProviderRegistry())

    def testRegistry(self):
        registry = QgsDataItemProviderRegistry()
        initial_providers = registry.providers()
        self.assertTrue(initial_providers)  # we expect a bunch of default providers
        self.assertTrue([p.name() for p in initial_providers if p.name() == "files"])

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
        """Tests finding provider by name and return dataProviderKey"""

        registry = QgsDataItemProviderRegistry()
        self.assertIsNotNone(registry.provider("PostGIS"))
        self.assertIsNone(registry.provider("paper_and_pencil"))
        self.assertEqual(registry.provider("PostGIS").dataProviderKey(), "postgres")


if __name__ == "__main__":
    unittest.main()
