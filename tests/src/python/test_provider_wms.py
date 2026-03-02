"""QGIS Unit tests for the WMS provider.

From build dir, run: ctest -R PyQgsWMSProvider -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Germán Carrillo"
__date__ = "2025-09-08"
__copyright__ = "Copyright 2025, Germán Carrillo"

import os.path
import shutil
import tempfile
import unittest

from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsRasterLayer,
    QgsSettings,
    QgsWmsUtils,
)
from qgis.PyQt.QtCore import QCoreApplication
from qgis.testing import QgisTestCase, start_app
from raster_provider_test_base import RasterProviderTestCase
from utilities import unitTestDataPath


class TestPyQgsWMSProvider(QgisTestCase, RasterProviderTestCase):
    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsWMSProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsWMSProvider")
        QgsSettings().clear()
        start_app()
        cls.TEST_DATA_DIR = unitTestDataPath()

        cls.basetestpath = tempfile.mkdtemp().replace("\\", "/")

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        if cls.basetestpath:
            shutil.rmtree(cls.basetestpath, True)
        cls.vl = (
            None
            # so as to properly close the provider and remove any temporary file
        )
        super().tearDownClass()

    def get_layer(self, test_id: str) -> QgsRasterLayer:
        endpoint = self.basetestpath + f"/{test_id}_fake_qgis_http_endpoint"
        with open(
            self.sanitize_local_url(endpoint, "?SERVICE=WMS&REQUEST=GetCapabilities"),
            "wb",
        ) as f:
            f.write(
                b"""
<WMS_Capabilities xmlns="http://www.opengis.net/wms" xmlns:sld="http://www.opengis.net/sld" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" version="1.3.0" xsi:schemaLocation="http://www.opengis.net/wms http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xsd http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/sld_capabilities.xsd">
<Service>
  <Name>WMS</Name>
  <Title>MapProxy-WMS Rasterbilder Bebauungsplaene rv auf Basiskarte grau</Title>
  <Abstract>WMS mit Rasterbilder der rechtsverbindlichen Bebauungsplaene auf grauer Basiskarte ueber MapProxy</Abstract>
    <MaxWidth>4000</MaxWidth>
    <MaxHeight>4000</MaxHeight>
</Service>
<Capability>
  <Request>
    <GetCapabilities>
      <Format>text/xml</Format>
    </GetCapabilities>
    <GetMap>
      <Format>image/png</Format>
      <Format>image/jpeg</Format>
      <Format>image/gif</Format>
      <Format>image/GeoTIFF</Format>
      <Format>image/tiff</Format>
    </GetMap>
    <GetFeatureInfo>
      <Format>text/plain</Format>
      <Format>text/html</Format>
      <Format>text/xml</Format>
    </GetFeatureInfo>
  </Request>
  <Exception>
    <Format>XML</Format>
    <Format>INIMAGE</Format>
    <Format>BLANK</Format>
  </Exception>
  <Layer>
    <Name>bplan_stadtkarte</Name>
    <Title>Bebauungsplaene Rasterbilder auf grauer Basiskarte</Title>
    <CRS>EPSG:25832</CRS>
    <CRS>EPSG:4326</CRS>
    <EX_GeographicBoundingBox>
      <westBoundLongitude>8.471329688231897</westBoundLongitude>
      <eastBoundLongitude>8.801042621684477</eastBoundLongitude>
      <southBoundLatitude>50.01482739264088</southBoundLatitude>
      <northBoundLatitude>50.22734893698391</northBoundLatitude>
    </EX_GeographicBoundingBox>
    <BoundingBox CRS="CRS:84" minx="8.471329688231897" miny="50.01482739264088" maxx="8.801042621684477" maxy="50.22734893698391" />
    <BoundingBox CRS="EPSG:4326" minx="50.01482739264088" miny="8.471329688231897" maxx="50.22734893698391" maxy="8.801042621684477" />
    <BoundingBox CRS="EPSG:25832" minx="462290" miny="5540412" maxx="485746" maxy="5563928" />
  </Layer>
</Capability>
</WMS_Capabilities>"""
            )
        return QgsRasterLayer(
            "layers=bplan_stadtkarte&styles&url=file://"
            + self.sanitize_local_url(endpoint, "?SERVICE=WMS&REQUEST=GetCapabilities"),
            "test",
            "wms",
        )

    def test_basic(self):
        """
        Test basic provider capabilities
        """
        rl = self.get_layer("basic")

        self.assertTrue(rl.isValid())
        # basic raster layer properties
        self.assertEqual(rl.width(), 0)
        self.assertEqual(rl.height(), 0)
        self.assertEqual(rl.bandCount(), 1)
        self.assertEqual(rl.bandName(1), "Band 1")
        self.assertIsNone(rl.attributeTable(1))
        self.assertFalse(rl.canCreateRasterAttributeTable())
        self.assertEqual(rl.providerType(), "wms")
        self.assertEqual(rl.rasterUnitsPerPixelX(), 1)
        self.assertEqual(rl.rasterUnitsPerPixelY(), 1)
        self.assertFalse(rl.subsetString())
        self.assertEqual(rl.subLayers(), ["bplan_stadtkarte"])
        self.assertEqual(rl.crs(), QgsCoordinateReferenceSystem("EPSG:25832"))
        self.assertEqual(rl.extent().xMinimum(), 462290)
        self.assertEqual(rl.extent().yMinimum(), 5540412)
        self.assertEqual(rl.extent().xMaximum(), 485746)
        self.assertEqual(rl.extent().yMaximum(), 5563928)

        self.assertEqual(rl.dataProvider().dataType(1), Qgis.DataType.ARGB32)
        self.assertEqual(rl.dataProvider().sourceDataType(1), Qgis.DataType.ARGB32)
        self.assertEqual(rl.dataProvider().bandScale(1), 1)
        self.assertEqual(rl.dataProvider().bandOffset(1), 0)
        self.assertEqual(rl.dataProvider().maximumTileSize().width(), 4000)
        self.assertEqual(rl.dataProvider().maximumTileSize().height(), 4000)

    def test_wms_utils(self):
        """
        Test WMS utils
        """
        rl = self.get_layer("basic")

        self.assertTrue(rl.isValid())
        self.assertTrue(QgsWmsUtils.isWmsLayer(rl))
        self.assertEqual(QgsWmsUtils.wmsVersion(rl), "1.3.0")

        # Test any other raster layer
        rl2 = QgsRasterLayer(
            os.path.join(self.TEST_DATA_DIR, "landsat_4326.tif"), "landsat", "gdal"
        )

        self.assertTrue(rl2.isValid())
        self.assertFalse(QgsWmsUtils.isWmsLayer(rl2))
        self.assertEqual(QgsWmsUtils.wmsVersion(rl2), "")


if __name__ == "__main__":
    unittest.main()
