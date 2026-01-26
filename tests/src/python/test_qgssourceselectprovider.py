"""
Test the QgsSourceSelectProvider
and QgsSourceSelectProviderRegistry classes

Run with: ctest -V -R PyQgsSourceSelectProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtTest import QSignalSpy
from qgis.gui import (
    QgsAbstractDataSourceWidget,
    QgsGui,
    QgsSourceSelectProvider,
    QgsSourceSelectProviderRegistry,
)
from qgis.core import QgsDataSourceUri, QgsSettings
import unittest
import os
from qgis.testing import start_app, QgisTestCase
from utilities import unitTestDataPath

__author__ = "Alessandro Pasotti"
__date__ = "01/09/2017"
__copyright__ = "Copyright 2017, The QGIS Project"


class ConcreteDataSourceWidget(QgsAbstractDataSourceWidget):
    pass


class ConcreteSourceSelectProvider(QgsSourceSelectProvider):

    def providerKey(self):
        return "MyTestProviderKey"

    def text(self):
        return "MyTestProviderText"

    def icon(self):
        return QIcon()

    def createDataSourceWidget(self):
        return ConcreteDataSourceWidget()

    def ordering(self):
        return 1


class ConcreteSourceSelectProvider2(QgsSourceSelectProvider):

    def providerKey(self):
        return "MyTestProviderKey2"

    def text(self):
        return "MyTestProviderText2"

    def name(self):
        return "MyName"

    def toolTip(self):
        return "MyToolTip"

    def icon(self):
        return QIcon()

    def createDataSourceWidget(self):
        return ConcreteDataSourceWidget()

    def ordering(self):
        return 2


class TestQgsSourceSelectProvider(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()

    def setUp(self):
        QgsSettings().clear()

    def testConcreteClass(self):
        provider = ConcreteSourceSelectProvider()
        self.assertTrue(isinstance(provider, ConcreteSourceSelectProvider))
        widget = provider.createDataSourceWidget()
        self.assertTrue(isinstance(widget, ConcreteDataSourceWidget))
        self.assertEqual(provider.providerKey(), "MyTestProviderKey")
        self.assertEqual(provider.name(), "MyTestProviderKey")
        self.assertEqual(provider.text(), "MyTestProviderText")
        self.assertEqual(provider.toolTip(), "")
        self.assertEqual(provider.ordering(), 1)
        self.assertTrue(isinstance(provider.icon(), QIcon))

        # test toolTip
        provider = ConcreteSourceSelectProvider2()
        self.assertEqual(provider.toolTip(), "MyToolTip")

    def _testRegistry(self, registry):
        registry.addProvider(ConcreteSourceSelectProvider())
        registry.addProvider(ConcreteSourceSelectProvider2())

        # Check order
        self.assertEqual(
            ["MyTestProviderKey", "MyName"],
            [
                p.name()
                for p in registry.providers()
                if p.providerKey().startswith("MyTestProviderKey")
            ],
        )

        registry = QgsSourceSelectProviderRegistry()
        registry.addProvider(ConcreteSourceSelectProvider())
        registry.addProvider(ConcreteSourceSelectProvider2())

        # Check order
        self.assertEqual(
            ["MyTestProviderKey", "MyName"],
            [
                p.name()
                for p in registry.providers()
                if p.providerKey().startswith("MyTestProviderKey")
            ],
        )

        # Get provider by name
        self.assertTrue(registry.providerByName("MyTestProviderKey"))
        self.assertTrue(registry.providerByName("MyName"))

        # Get not existent by name
        self.assertFalse(registry.providerByName("Oh This Is Missing!"))

        # Get providers by data provider key
        self.assertGreater(len(registry.providersByKey("MyTestProviderKey")), 0)
        self.assertGreater(len(registry.providersByKey("MyTestProviderKey2")), 0)

        # Get not existent by key
        self.assertEqual(len(registry.providersByKey("Oh This Is Missing!")), 0)

    def testRemoveProvider(self):
        registry = QgsSourceSelectProviderRegistry()
        registry.addProvider(ConcreteSourceSelectProvider())
        registry.addProvider(ConcreteSourceSelectProvider2())
        self.assertEqual(
            ["MyTestProviderKey", "MyName"],
            [
                p.name()
                for p in registry.providers()
                if p.providerKey().startswith("MyTestProviderKey")
            ],
        )

        self.assertTrue(registry.removeProvider(registry.providerByName("MyName")))
        self.assertEqual(
            ["MyTestProviderKey"],
            [
                p.name()
                for p in registry.providers()
                if p.providerKey().startswith("MyTestProviderKey")
            ],
        )

        self.assertTrue(
            registry.removeProvider(registry.providerByName("MyTestProviderKey"))
        )
        self.assertEqual(
            [],
            [
                p.name()
                for p in registry.providers()
                if p.providerKey().startswith("MyTestProviderKey")
            ],
        )

    def testRegistry(self):
        registry = QgsSourceSelectProviderRegistry()
        self._testRegistry(registry)

    def testRegistrySingleton(self):
        registry = QgsGui.sourceSelectProviderRegistry()
        self._testRegistry(registry)
        # Check that at least OGR and GDAL are here
        self.assertTrue(registry.providersByKey("ogr"))
        self.assertTrue(registry.providersByKey("gdal"))

    def testSourceSelectProvidersConfigureFromUri(self):
        """
        Test configure from URI
        """
        registry = QgsGui.sourceSelectProviderRegistry()
        enabled_entries = {
            reg_entry.name(): reg_entry
            for reg_entry in registry.providers()
            if reg_entry.capabilities()
            & QgsSourceSelectProvider.Capability.ConfigureFromUri
        }
        self.assertIn("ogr", enabled_entries.keys())
        self.assertIn("gdal", enabled_entries.keys())
        self.assertIn("GeoPackage", enabled_entries.keys())
        self.assertIn("spatialite", enabled_entries.keys())

        # Test ogr
        test_path = os.path.join(unitTestDataPath(), "points.shp")
        source_select = enabled_entries["ogr"].createDataSourceWidget()
        self.assertTrue(source_select.configureFromUri(test_path))
        spy = QSignalSpy(source_select.addVectorLayers)
        source_select.addButtonClicked()
        self.assertEqual(len(spy), 1)
        arg_path, _, arg_type = spy[0]
        self.assertEqual(arg_path[0], test_path)

        # Test GDAL
        test_path = os.path.join(unitTestDataPath(), "raster_layer.tiff")
        source_select = enabled_entries["gdal"].createDataSourceWidget()
        spy = QSignalSpy(source_select.addRasterLayers)
        self.assertTrue(source_select.configureFromUri(test_path))
        source_select.addButtonClicked()
        self.assertEqual(len(spy), 1)
        arg_path = spy[0][0]
        self.assertEqual(arg_path[0], test_path)

        # Test vector GPKG
        test_path = os.path.join(
            unitTestDataPath(), "mixed_layers.gpkg|layername=points"
        )
        source_select = enabled_entries["GeoPackage"].createDataSourceWidget()
        self.assertTrue(source_select.configureFromUri(test_path))
        spy = QSignalSpy(source_select.addLayer)
        source_select.addButtonClicked()
        self.assertEqual(len(spy), 1)
        _, arg_path, arg_name, arg_key = spy[0]
        self.assertEqual(arg_path, test_path)
        self.assertEqual(arg_name, "points")
        self.assertEqual(arg_key, "ogr")

        # Test vector spatialite
        test_path = os.path.join(unitTestDataPath(), "provider", "spatialite.db")
        uri = QgsDataSourceUri()
        uri.setDatabase(test_path)
        uri.setTable("some data")
        uri.setGeometryColumn("geom")
        uri.setSql("pk > 0")
        source_select = enabled_entries["spatialite"].createDataSourceWidget()
        self.assertTrue(source_select.configureFromUri(uri.uri()))
        spy = QSignalSpy(source_select.addDatabaseLayers)
        source_select.addButtonClicked()
        self.assertEqual(len(spy), 1)
        arg_tables, arg_key = spy[0]
        self.assertEqual(arg_tables[0], uri.uri())


if __name__ == "__main__":
    unittest.main()
