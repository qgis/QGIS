"""QGIS Unit tests for QgsOwsConnection

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "12.09.2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import QgsDataSourceUri, QgsOwsConnection, QgsSettings
import unittest
from qgis.testing import start_app, QgisTestCase


class TestQgsOwsConnection(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsColorScheme.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsColorScheme")
        QgsSettings().clear()
        start_app()

        # setup some fake connections
        settings = QgsSettings()
        key = "/connections/ows/items/wms/connections/items/test/"
        settings.setValue(key + "url", "aaa.bbb.com")
        settings.setValue(key + "http-header", {"referer": "my_ref"})
        settings.setValue(key + "ignore-get-map-uri", True)
        settings.setValue(key + "ignore-get-feature-info-uri", True)
        settings.setValue(key + "smooth-pixmap-transform", True)
        settings.setValue(key + "dpi-mode", 4)
        settings.setValue(key + "ignore-axis-orientation", True)
        settings.setValue(key + "invert-axis-orientation", True)
        settings.setValue(key + "feature-count", 9)

        key = "/connections/ows/items/wfs/connections/items/test/"
        settings.setValue(key + "url", "ccc.ddd.com")
        settings.setValue(key + "version", "1.1.0")
        settings.setValue(key + "max-num-features", "47")
        settings.setValue(key + "ignore-axis-orientation", True)
        settings.setValue(key + "invert-axis-orientation", True)

    def testWmsConnection(self):
        c = QgsOwsConnection("WMS", "test")
        uri = c.uri()

        self.assertEqual(uri.param("url"), "aaa.bbb.com")
        self.assertEqual(uri.httpHeader("referer"), "my_ref")
        self.assertEqual(uri.param("IgnoreGetMapUrl"), "1")
        self.assertEqual(uri.param("IgnoreGetFeatureInfoUrl"), "1")
        self.assertEqual(uri.param("SmoothPixmapTransform"), "1")
        self.assertEqual(uri.param("dpiMode"), "4")
        self.assertEqual(uri.param("IgnoreAxisOrientation"), "1")
        self.assertEqual(uri.param("InvertAxisOrientation"), "1")
        self.assertEqual(uri.param("featureCount"), "9")

    def testWmsSettings(self):
        uri = QgsDataSourceUri()
        QgsOwsConnection.addWmsWcsConnectionSettings(uri, "wms", "test")

        self.assertEqual(uri.httpHeader("referer"), "my_ref")
        self.assertEqual(uri.param("IgnoreGetMapUrl"), "1")
        self.assertEqual(uri.param("IgnoreGetFeatureInfoUrl"), "1")
        self.assertEqual(uri.param("SmoothPixmapTransform"), "1")
        self.assertEqual(uri.param("dpiMode"), "4")
        self.assertEqual(uri.param("IgnoreAxisOrientation"), "1")
        self.assertEqual(uri.param("InvertAxisOrientation"), "1")
        self.assertEqual(uri.param("featureCount"), "9")

    def testWfsConnection(self):
        c = QgsOwsConnection("WFS", "test")
        uri = c.uri()

        self.assertEqual(uri.param("url"), "ccc.ddd.com")
        self.assertEqual(uri.param("version"), "1.1.0")
        self.assertEqual(uri.param("maxNumFeatures"), "47")
        self.assertEqual(uri.param("IgnoreAxisOrientation"), "1")
        self.assertEqual(uri.param("InvertAxisOrientation"), "1")

    def testWfsSettings(self):
        uri = QgsDataSourceUri()
        QgsOwsConnection.addWfsConnectionSettings(uri, "wfs", "test")

        self.assertEqual(uri.param("version"), "1.1.0")
        self.assertEqual(uri.param("maxNumFeatures"), "47")
        self.assertEqual(uri.param("IgnoreAxisOrientation"), "1")
        self.assertEqual(uri.param("InvertAxisOrientation"), "1")


if __name__ == "__main__":
    unittest.main()
