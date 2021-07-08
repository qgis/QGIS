"""QGIS Unit tests for QgsMapLayerServerProperties

From build dir, run:
ctest -R PyQgsMapLayerServerProperties -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Etienne Trimaille'
__date__ = '21/06/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

from qgis.core import (
    QgsMapLayerServerProperties,
    QgsVectorLayer,
)
from qgis.testing import start_app, unittest

app = start_app()


class TestQgsMapLayerServerConfig(unittest.TestCase):

    def test_false(self):
        self.assertTrue(False)

    def test_deprecated_function(self):
        """ Test deprecated function about metadata url. """
        layer = QgsVectorLayer('Point?field=fldtxt:string', 'layer_1', 'memory')

        self.assertEqual("", layer.serverProperties().metadataUrls()[0].url)
        self.assertEqual("", layer.serverProperties().metadataUrls()[0].format)
        self.assertEqual("", layer.serverProperties().metadataUrls()[0].type)

        layer.setMetadataUrl("https://my.url")

        self.assertEqual("https://my.url", layer.serverProperties().metadataUrls()[0].url)
        self.assertEqual("", layer.serverProperties().metadataUrls()[0].format)
        self.assertEqual("", layer.serverProperties().metadataUrls()[0].type)

        layer.setMetadataUrl("https://my.other.url")
        layer.setMetadataUrlFormat("text/xml")
        layer.setMetadataUrlType("FGDC")

        self.assertEqual("https://my.other.url", layer.serverProperties().metadataUrls()[0].url)
        self.assertEqual("text/xml", layer.serverProperties().metadataUrls()[0].format)
        self.assertEqual("FGDC", layer.serverProperties().metadataUrls()[0].type)

    def test_read_write(self):
        """ Test read write the structure about metadata url. """
        layer = QgsVectorLayer('Point?field=fldtxt:string', 'layer_1', 'memory')
        server_properties = QgsMapLayerServerProperties(layer)
        self.assertListEqual([], server_properties.metadataUrl())

        url = QgsMapLayerServerProperties.MetadataUrl("https://my.url", "FGDC", "text/xml")
        server_properties.addMetadataUrl(url)
        layer.setServerProperties(server_properties)
        self.assertEqual("https://m.url", layer.serverProperties().metadataUrls()[0].url)

    def test_metadata_url(self):
        """ Test the metadata url struct. """
        url = QgsMapLayerServerProperties.MetadataUrl("https://my.url", "FGDC", "text/xml")

        other = QgsMapLayerServerProperties.MetadataUrl("https://my.url", "FGDC", "text/html")
        self.assertFalse(url == other)

        other = QgsMapLayerServerProperties.MetadataUrl("https://my.url", "FGDC", "text/xml")
        self.assertTrue(url == other)
