"""QGIS Unit tests for the WCS provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alexander Bruy"
__date__ = "2026-10-11"
__copyright__ = "Copyright 2026, Alexander Bruy"

import os
import unittest
from http.server import BaseHTTPRequestHandler
from pathlib import Path
from urllib.parse import parse_qs, urlparse

import mockedwebserver
from osgeo import gdal
from qgis.core import QgsDataSourceUri, QgsRasterLayer
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class WcsTestHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        parsed_query = urlparse(self.path)

        query_string = {
            k.upper(): v[0] for k, v in parse_qs(parsed_query.query).items()
        }
        request_type = query_string.get("REQUEST", "").upper()

        if request_type == "GETCAPABILITIES":
            self.send_response(200)
            self.send_header("Content-Type", "text/xml")
            self.end_headers()
            self.wfile.write(self.server.capabilities_xml)
        elif request_type == "DESCRIBECOVERAGE":
            self.send_response(200)
            self.send_header("Content-Type", "text/xml")
            self.end_headers()
            self.wfile.write(self.server.describecoverage_xml)
        elif request_type == "GETCOVERAGE":
            self.server.coverage_requests.append(query_string)
            self.send_response(200)
            self.send_header("Content-Type", "image/tiff")
            self.end_headers()
            self.wfile.write(self.server.dummy_tiff)
        else:
            self.send_response(404)
            self.end_headers()


class PyQgsWCSProvider(QgisTestCase):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        driver = gdal.GetDriverByName("GTiff")
        ds = driver.Create("/vsimem/dummy_wcs.tif", 100, 100, 1, gdal.GDT_Byte)
        ds = None

        vsi_file = gdal.VSIFOpenL("/vsimem/dummy_wcs.tif", "rb")
        file_size = gdal.VSIStatL("/vsimem/dummy_wcs.tif").size
        cls.dummy_tiff = gdal.VSIFReadL(1, file_size, vsi_file)
        gdal.VSIFCloseL(vsi_file)
        gdal.Unlink("/vsimem/dummy_wcs.tif")

        file_path = os.path.join(
            TEST_DATA_DIR, "provider", "wcs", "getcapabilities.xml"
        )
        with open(file_path, "rb") as f:
            cls.capabilities_xml = f.read()

        file_path = os.path.join(
            TEST_DATA_DIR, "provider", "wcs", "describecoverage.xml"
        )
        with open(file_path, "rb") as f:
            cls.describecoverage_xml = f.read()

    def test_getcache_grid_parameters(self):
        # Ensure that GRIDORIGIN parameter uses correctly swapped extent
        # when using CRS with inverted axis order.
        # See https://github.com/qgis/QGIS/issues/59445
        server, port = mockedwebserver.launch(WcsTestHandler)
        server.server.capabilities_xml = self.capabilities_xml.replace(
            b"$$PORT$$", bytes(str(port), "ascii")
        )
        server.server.describecoverage_xml = self.describecoverage_xml
        server.server.dummy_tiff = self.dummy_tiff
        server.server.coverage_requests = []

        try:
            uri = QgsDataSourceUri()
            uri.setParam("identifier", "dtm")
            uri.setParam("crs", "EPSG:4326")
            uri.setParam("format", "image/tiff")
            uri.setParam("url", f"http://localhost:{port}/wcs")
            uri.setParam("IgnoreGetMapUrl", "1")
            layer = QgsRasterLayer(uri.encodedUri().data().decode(), "test", "wcs")
            self.assertTrue(layer.isValid())
            provider = layer.dataProvider()

            request_width = 100
            request_height = 100
            extent = provider.extent()
            block = provider.block(1, extent, request_width, request_height)
            self.assertTrue(block.isValid())

            requests = server.server.coverage_requests
            block_request = requests[-1]
            self.assertIn("GRIDORIGIN", block_request)
            self.assertIn("GRIDOFFSETS", block_request)
            self.assertIn("BOUNDINGBOX", block_request)

            parts = block_request["BOUNDINGBOX"].split(",")
            self.assertGreaterEqual(len(parts), 4)
            bbox_xmin, bbox_ymin, bbox_xmax, bbox_ymax = (float(i) for i in parts[:4])
            self.assertLess(bbox_xmin, bbox_xmax)
            self.assertLess(bbox_ymin, bbox_ymax)

            parts = block_request["GRIDORIGIN"].split(",")
            self.assertEqual(len(parts), 2)
            grid_origin_x, grid_origin_y = (float(i) for i in parts)
            self.assertAlmostEqual(grid_origin_x, bbox_xmax, places=6)
            self.assertAlmostEqual(grid_origin_y, bbox_ymin, places=6)

            x_res = extent.width() / request_width
            y_res = extent.height() / request_height
            parts = [float(i) for i in block_request["GRIDOFFSETS"].split(",")]
            self.assertEqual(len(parts), 2)
            self.assertAlmostEqual(parts[0], -y_res, places=6)
            self.assertAlmostEqual(parts[1], x_res, places=6)
        finally:
            server.stop()


if __name__ == "__main__":
    unittest.main()
