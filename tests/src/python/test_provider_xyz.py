"""QGIS Unit tests for the XYZ provider.

From build dir, run: ctest -R PyQgsXyzProvider -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2025-04-23"
__copyright__ = "Copyright 2025, Nyall Dawson"

import unittest

from qgis.PyQt.QtCore import QCoreApplication, QUrl, QSize
from qgis.core import (
    Qgis,
    QgsCoordinateReferenceSystem,
    QgsSettings,
    QgsRasterLayer,
    QgsMapSettings,
    QgsRectangle,
    QgsMapRendererSequentialJob,
)
from qgis.testing import start_app, QgisTestCase


class TestPyQgsXyzProvider(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "xyz"

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsXyzProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsXyzProvider")
        QgsSettings().clear()
        start_app()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        cls.vl = (
            None
            # so as to properly close the provider and remove any temporary file
        )
        super().tearDownClass()

    def get_layer(self, test_id: str) -> QgsRasterLayer:
        data_path = self.get_test_data_path("raster/osm_tiles")
        local_path = QUrl.fromLocalFile(data_path.as_posix()).toString()

        return QgsRasterLayer(
            f"tilePixelRatio=1&type=xyz&url={local_path}/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0",
            "test",
            "wms",
        )

    def test_basic(self):
        """
        Test basic provider capabilities
        """
        data_path = self.get_test_data_path("raster/osm_tiles")
        local_path = QUrl.fromLocalFile(data_path.as_posix()).toString()

        rl = QgsRasterLayer(
            f"tilePixelRatio=1&type=xyz&url={local_path}/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0",
            "test",
            "wms",
        )

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
        self.assertEqual(rl.subLayers(), ["xyz"])
        self.assertEqual(rl.crs(), QgsCoordinateReferenceSystem("EPSG:3857"))
        self.assertAlmostEqual(rl.extent().xMinimum(), -20037508.342789244, 2)
        self.assertAlmostEqual(rl.extent().yMinimum(), -20037508.342789248, 2)
        self.assertAlmostEqual(rl.extent().xMaximum(), 20037508.342789244, 2)
        self.assertAlmostEqual(rl.extent().yMaximum(), 20037508.342789244, 2)

        self.assertEqual(rl.dataProvider().dataType(1), Qgis.DataType.ARGB32)
        self.assertEqual(rl.dataProvider().sourceDataType(1), Qgis.DataType.ARGB32)
        self.assertEqual(rl.dataProvider().bandScale(1), 1)
        self.assertEqual(rl.dataProvider().bandOffset(1), 0)

        self.assertEqual(
            rl.dataProvider().capabilities(), Qgis.RasterInterfaceCapability.Prefetch
        )

    def test_render_block(self):
        data_path = self.get_test_data_path("raster/osm_tiles")
        local_path = QUrl.fromLocalFile(data_path.as_posix()).toString()

        rl = QgsRasterLayer(
            f"tilePixelRatio=1&type=xyz&url={local_path}/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0",
            "test",
            "wms",
        )

        ms = QgsMapSettings()
        ms.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:4326"))
        ms.setExtent(QgsRectangle(11.25, 45.0, 22.5, 56.25))
        ms.setOutputSize(QSize(512, 512))
        ms.setLayers([rl])
        ms.setFlag(Qgis.MapSettingsFlag.RenderPartialOutput)
        ms.setFlag(Qgis.MapSettingsFlag.HighQualityImageTransforms)
        ms.setFlag(Qgis.MapSettingsFlag.RenderMapTile, False)
        ms.setFlag(Qgis.MapSettingsFlag.Antialiasing)
        job = QgsMapRendererSequentialJob(ms)
        job.start()
        job.waitForFinished()
        image = job.renderedImage()

        self.assertTrue(self.image_check("osm", "osm", image))


if __name__ == "__main__":
    unittest.main()
