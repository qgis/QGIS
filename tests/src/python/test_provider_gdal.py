# -*- coding: utf-8 -*-
"""Generic Unit tests for the GDAL provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2018-30-10'
__copyright__ = 'Copyright 2018, Nyall Dawson'

import math
import os
import struct

from osgeo import gdal
from qgis.core import (
    Qgis,
    QgsPointXY,
    QgsProviderRegistry,
    QgsRasterLayer,
    QgsRectangle,
)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QTemporaryDir

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


class PyQgsGdalProvider(unittest.TestCase):

    def checkBlockContents(self, block, expected):
        res = []
        for r in range(block.height()):
            res.extend([block.value(r, c) for c in range(block.width())])
        self.assertEqual(res, expected)

    def testRasterBlock(self):
        """Test raster block with extent"""

        path = os.path.join(unitTestDataPath(), 'landsat_4326.tif')
        raster_layer = QgsRasterLayer(path, 'test')
        self.assertTrue(raster_layer.isValid())

        extent = QgsRectangle(17.94284482577178252, 30.23021770271909503, 17.94407867909909626, 30.23154272264058307)
        block = raster_layer.dataProvider().block(1, extent, 2, 3)
        self.checkBlockContents(block, [
            125.0, 125.0,
            125.0, 125.0,
            125.0, 124.0,
        ])

        full_content = [
            125.0, 125.0, 125.0,
            125.0, 125.0, 125.0,
            125.0, 124.0, 125.0,
            126.0, 127.0, 127.0,
        ]

        extent = raster_layer.extent()
        block = raster_layer.dataProvider().block(1, extent, 3, 4)
        self.checkBlockContents(block, full_content)

        extent = raster_layer.extent()
        extent.grow(-0.0001)
        block = raster_layer.dataProvider().block(1, extent, 3, 4)
        self.checkBlockContents(block, full_content)

        row_height = raster_layer.extent().height() / raster_layer.height()

        for row in range(raster_layer.height()):
            extent = raster_layer.extent()
            extent.setYMaximum(extent.yMaximum() - row_height * row)
            extent.setYMinimum(extent.yMaximum() - row_height)
            block = raster_layer.dataProvider().block(1, extent, 3, 1)
            self.checkBlockContents(block, full_content[row * 3:row * 3 + 3])

    def testDecodeEncodeUriGpkg(self):
        """Test decodeUri/encodeUri geopackage support"""

        uri = '/my/raster.gpkg'
        parts = QgsProviderRegistry.instance().decodeUri('gdal', uri)
        self.assertEqual(parts, {'path': '/my/raster.gpkg', 'layerName': None})
        encodedUri = QgsProviderRegistry.instance().encodeUri('gdal', parts)
        self.assertEqual(encodedUri, uri)

        uri = 'GPKG:/my/raster.gpkg'
        parts = QgsProviderRegistry.instance().decodeUri('gdal', uri)
        self.assertEqual(parts, {'path': '/my/raster.gpkg', 'layerName': None})
        encodedUri = QgsProviderRegistry.instance().encodeUri('gdal', parts)
        self.assertEqual(encodedUri, '/my/raster.gpkg')

        uri = 'GPKG:/my/raster.gpkg:mylayer'
        parts = QgsProviderRegistry.instance().decodeUri('gdal', uri)
        self.assertEqual(parts, {'path': '/my/raster.gpkg', 'layerName': 'mylayer'})
        encodedUri = QgsProviderRegistry.instance().encodeUri('gdal', parts)
        self.assertEqual(encodedUri, uri)

    def testDecodeEncodeUriOptions(self):
        """Test decodeUri/encodeUri options support"""

        uri = '/my/raster.pdf|option:DPI=300|option:GIVEME=TWO'
        parts = QgsProviderRegistry.instance().decodeUri('gdal', uri)
        self.assertEqual(parts, {'path': '/my/raster.pdf', 'layerName': None, 'openOptions': ['DPI=300', 'GIVEME=TWO']})
        encodedUri = QgsProviderRegistry.instance().encodeUri('gdal', parts)
        self.assertEqual(encodedUri, uri)

    def testDecodeEncodeUriVsizip(self):
        """Test decodeUri/encodeUri for /vsizip/ prefixed URIs"""

        uri = '/vsizip//my/file.zip/image.tif'
        parts = QgsProviderRegistry.instance().decodeUri('gdal', uri)
        self.assertEqual(parts, {'path': '/my/file.zip', 'layerName': None, 'vsiPrefix': '/vsizip/',
                                 'vsiSuffix': '/image.tif'})
        encodedUri = QgsProviderRegistry.instance().encodeUri('gdal', parts)
        self.assertEqual(encodedUri, uri)

    def test_provider_sidecar_files_for_uri(self):
        """
        Test retrieving sidecar files for uris
        """
        metadata = QgsProviderRegistry.instance().providerMetadata('gdal')

        self.assertEqual(metadata.sidecarFilesForUri(''), [])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/some_file.asc'),
                         ['/home/me/some_file.aux.xml', '/home/me/some_file.asc.aux.xml', '/home/me/some_file.vat.dbf',
                          '/home/me/some_file.asc.vat.dbf', '/home/me/some_file.ovr', '/home/me/some_file.asc.ovr',
                          '/home/me/some_file.wld', '/home/me/some_file.asc.wld'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.jpg'),
                         ['/home/me/special.jpw', '/home/me/special.jgw', '/home/me/special.jpgw',
                          '/home/me/special.jpegw', '/home/me/special.aux.xml', '/home/me/special.jpg.aux.xml',
                          '/home/me/special.vat.dbf', '/home/me/special.jpg.vat.dbf', '/home/me/special.ovr',
                          '/home/me/special.jpg.ovr', '/home/me/special.wld', '/home/me/special.jpg.wld'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.img'),
                         ['/home/me/special.ige', '/home/me/special.aux.xml', '/home/me/special.img.aux.xml',
                          '/home/me/special.vat.dbf', '/home/me/special.img.vat.dbf', '/home/me/special.ovr',
                          '/home/me/special.img.ovr', '/home/me/special.wld', '/home/me/special.img.wld'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.sid'),
                         ['/home/me/special.j2w', '/home/me/special.aux.xml', '/home/me/special.sid.aux.xml',
                          '/home/me/special.vat.dbf', '/home/me/special.sid.vat.dbf', '/home/me/special.ovr',
                          '/home/me/special.sid.ovr', '/home/me/special.wld', '/home/me/special.sid.wld'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.tif'),
                         ['/home/me/special.tifw', '/home/me/special.tfw', '/home/me/special.aux.xml',
                          '/home/me/special.tif.aux.xml', '/home/me/special.vat.dbf', '/home/me/special.tif.vat.dbf',
                          '/home/me/special.ovr', '/home/me/special.tif.ovr', '/home/me/special.wld',
                          '/home/me/special.tif.wld'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.bil'),
                         ['/home/me/special.bilw', '/home/me/special.blw', '/home/me/special.aux.xml',
                          '/home/me/special.bil.aux.xml', '/home/me/special.vat.dbf', '/home/me/special.bil.vat.dbf',
                          '/home/me/special.ovr', '/home/me/special.bil.ovr', '/home/me/special.wld',
                          '/home/me/special.bil.wld'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.raster'),
                         ['/home/me/special.rasterw', '/home/me/special.aux.xml', '/home/me/special.raster.aux.xml',
                          '/home/me/special.vat.dbf', '/home/me/special.raster.vat.dbf', '/home/me/special.ovr',
                          '/home/me/special.raster.ovr', '/home/me/special.wld', '/home/me/special.raster.wld'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.bt'),
                         ['/home/me/special.btw', '/home/me/special.aux.xml', '/home/me/special.bt.aux.xml',
                          '/home/me/special.vat.dbf', '/home/me/special.bt.vat.dbf', '/home/me/special.ovr',
                          '/home/me/special.bt.ovr', '/home/me/special.wld', '/home/me/special.bt.wld'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.rst'),
                         ['/home/me/special.rdc', '/home/me/special.smp', '/home/me/special.ref',
                          '/home/me/special.vct', '/home/me/special.vdc', '/home/me/special.avl',
                          '/home/me/special.aux.xml', '/home/me/special.rst.aux.xml', '/home/me/special.vat.dbf',
                          '/home/me/special.rst.vat.dbf', '/home/me/special.ovr', '/home/me/special.rst.ovr',
                          '/home/me/special.wld', '/home/me/special.rst.wld'])
        self.assertEqual(metadata.sidecarFilesForUri('/home/me/special.sdat'),
                         ['/home/me/special.sgrd', '/home/me/special.mgrd', '/home/me/special.prj',
                          '/home/me/special.aux.xml', '/home/me/special.sdat.aux.xml', '/home/me/special.vat.dbf',
                          '/home/me/special.sdat.vat.dbf', '/home/me/special.ovr', '/home/me/special.sdat.ovr',
                          '/home/me/special.wld', '/home/me/special.sdat.wld'])

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 5, 0), "GDAL 3.5.0 required")
    def testInt64(self):
        """Test Int64 support"""

        tmp_dir = QTemporaryDir()
        tmpfile = os.path.join(tmp_dir.path(), 'testInt64.tif')
        ds = gdal.GetDriverByName('GTiff').Create(tmpfile, 2, 2, 1, gdal.GDT_Int64)
        ds.WriteRaster(0, 0, 2, 2, struct.pack('q' * 4, -1234567890123, 1234567890123, -(1 << 63), (1 << 63) - 1))
        ds = None

        raster_layer = QgsRasterLayer(tmpfile, 'test')
        self.assertTrue(raster_layer.isValid())
        self.assertEqual(raster_layer.dataProvider().dataType(1), Qgis.Float64)

        extent = raster_layer.extent()
        block = raster_layer.dataProvider().block(1, extent, 2, 2)

        full_content = [
            -1234567890123, 1234567890123, float(-(1 << 63)), float((1 << 63) - 1)
        ]
        self.checkBlockContents(block, full_content)

        pos = QgsPointXY(0, 0)
        value_sample = raster_layer.dataProvider().sample(pos, 1)[0]
        self.assertEqual(value_sample, full_content[0])

        pos = QgsPointXY(1, 0)
        value_sample = raster_layer.dataProvider().sample(pos, 1)[0]
        self.assertEqual(value_sample, full_content[1])

        pos = QgsPointXY(0, -1)
        value_sample = raster_layer.dataProvider().sample(pos, 1)[0]
        self.assertTrue(value_sample, full_content[2])

        pos = QgsPointXY(1, -1)
        value_sample = raster_layer.dataProvider().sample(pos, 1)[0]
        self.assertTrue(math.isnan(value_sample))  # (1 << 63) - 1 not exactly representable as double

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 5, 0), "GDAL 3.5.0 required")
    def testUInt64(self):
        """Test Int64 support"""

        tmp_dir = QTemporaryDir()
        tmpfile = os.path.join(tmp_dir.path(), 'testUInt64.tif')
        ds = gdal.GetDriverByName('GTiff').Create(tmpfile, 2, 2, 1, gdal.GDT_UInt64)
        ds.WriteRaster(0, 0, 2, 2, struct.pack('Q' * 4, 1, 1234567890123, 0, (1 << 64) - 1))
        ds = None

        raster_layer = QgsRasterLayer(tmpfile, 'test')
        self.assertTrue(raster_layer.isValid())
        self.assertEqual(raster_layer.dataProvider().dataType(1), Qgis.Float64)

        extent = raster_layer.extent()
        block = raster_layer.dataProvider().block(1, extent, 2, 2)

        full_content = [
            1, 1234567890123, 0, float((1 << 64) - 1)
        ]
        self.checkBlockContents(block, full_content)

        pos = QgsPointXY(0, 0)
        value_sample = raster_layer.dataProvider().sample(pos, 1)[0]
        self.assertEqual(value_sample, full_content[0])

        pos = QgsPointXY(1, 0)
        value_sample = raster_layer.dataProvider().sample(pos, 1)[0]
        self.assertEqual(value_sample, full_content[1])

        pos = QgsPointXY(0, -1)
        value_sample = raster_layer.dataProvider().sample(pos, 1)[0]
        self.assertEqual(value_sample, full_content[2])

        pos = QgsPointXY(1, -1)
        value_sample = raster_layer.dataProvider().sample(pos, 1)[0]
        self.assertTrue(math.isnan(value_sample))

    @unittest.skipIf(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(3, 2, 0) or int(gdal.VersionInfo('VERSION_NUM')) >= GDAL_COMPUTE_VERSION(3, 5, 2), "Test only relevant on GDAL >= 3.2.0 and < 3.5.2")
    def testSanitizeVRT(self):
        """Test qgsgdalprovider.cpp sanitizeVRTFile() / workaround for https://github.com/qgis/QGIS/issues/49285 """

        tmp_dir = QTemporaryDir()
        tmpfilename = os.path.join(tmp_dir.path(), 'tmp.tif')
        path = os.path.join(unitTestDataPath(), 'landsat_4326.tif')
        tmp_ds = gdal.Translate(tmpfilename, path, options='-outsize 1024 0')
        tmp_ds.BuildOverviews('NEAR', [2])
        tmp_ds = None
        vrtfilename = os.path.join(tmp_dir.path(), 'out.vrt')
        ds = gdal.BuildVRT(vrtfilename, [tmpfilename])
        ds = None
        assert 'OverviewList' in open(vrtfilename, 'rt').read()

        raster_layer = QgsRasterLayer(vrtfilename, 'test')
        del raster_layer

        assert 'OverviewList' not in open(vrtfilename, 'rt').read()


if __name__ == '__main__':
    unittest.main()
