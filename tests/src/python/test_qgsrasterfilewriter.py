# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterFileWriter.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Radim Blazek'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'

import qgis  # NOQA

import os
import glob
import tempfile

from osgeo import gdal
from qgis.PyQt.QtCore import QTemporaryFile, QDir
from qgis.core import (QgsContrastEnhancement,
                       QgsRaster,
                       QgsRasterLayer,
                       QgsRasterChecker,
                       QgsRasterPipe,
                       QgsRasterFileWriter,
                       QgsRasterProjector,
                       QgsRectangle)

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsRasterFileWriter(unittest.TestCase):

    def __init__(self, methodName):
        unittest.TestCase.__init__(self, methodName)
        self.testDataDir = unitTestDataPath()
        self.report = "<h1>Python Raster File Writer Tests</h1>\n"

    def write(self, theRasterName):
        print(theRasterName)

        path = "%s/%s" % (self.testDataDir, theRasterName)
        rasterLayer = QgsRasterLayer(path, "test")
        if not rasterLayer.isValid():
            return False
        provider = rasterLayer.dataProvider()

        tmpFile = QTemporaryFile()
        tmpFile.open()  # fileName is not available until open
        tmpName = tmpFile.fileName()
        tmpFile.close()
        # do not remove when class is destroyed so that we can read
        # the file and see difference
        tmpFile.setAutoRemove(False)

        fileWriter = QgsRasterFileWriter(tmpName)
        pipe = QgsRasterPipe()
        if not pipe.set(provider.clone()):
            print("Cannot set pipe provider")
            return False

        projector = QgsRasterProjector()
        projector.setCrs(provider.crs(), provider.crs())
        if not pipe.insert(2, projector):
            print("Cannot set pipe projector")
            return False

        fileWriter.writeRaster(
            pipe,
            provider.xSize(),
            provider.ySize(),
            provider.extent(),
            provider.crs())

        checker = QgsRasterChecker()
        ok = checker.runTest("gdal", tmpName, "gdal", path)
        self.report += checker.report()

        # All OK, we can delete the file
        tmpFile.setAutoRemove(ok)

        return ok

    def testWrite(self):
        for name in glob.glob("%s/raster/*.tif" % self.testDataDir):
            baseName = os.path.basename(name)
            allOk = True
            ok = self.write("raster/%s" % baseName)
            if not ok:
                allOk = False

        reportFilePath = "%s/qgistest.html" % QDir.tempPath()
        reportFile = open(reportFilePath, 'a')
        reportFile.write(self.report)
        reportFile.close()

        assert allOk, "Raster file writer test failed"

    def testDriverForExtension(self):
        self.assertEqual(QgsRasterFileWriter.driverForExtension('tif'), 'GTiff')
        self.assertEqual(QgsRasterFileWriter.driverForExtension('TIF'), 'GTiff')
        self.assertEqual(QgsRasterFileWriter.driverForExtension('tIf'), 'GTiff')
        self.assertEqual(QgsRasterFileWriter.driverForExtension('.tif'), 'GTiff')
        self.assertEqual(QgsRasterFileWriter.driverForExtension('img'), 'HFA')
        self.assertEqual(QgsRasterFileWriter.driverForExtension('.vrt'), 'VRT')
        self.assertEqual(QgsRasterFileWriter.driverForExtension('.jpg'), 'JPEG')
        self.assertEqual(QgsRasterFileWriter.driverForExtension('asc'), 'AAIGrid')
        self.assertEqual(QgsRasterFileWriter.driverForExtension('not a format'), '')
        self.assertEqual(QgsRasterFileWriter.driverForExtension(''), '')

    def testExtensionsForFormat(self):
        self.assertCountEqual(QgsRasterFileWriter.extensionsForFormat('not format'), [])
        self.assertCountEqual(QgsRasterFileWriter.extensionsForFormat('GTiff'), ['tiff', 'tif'])
        self.assertCountEqual(QgsRasterFileWriter.extensionsForFormat('GPKG'), ['gpkg'])
        self.assertCountEqual(QgsRasterFileWriter.extensionsForFormat('JPEG'), ['jpg', 'jpeg'])
        self.assertCountEqual(QgsRasterFileWriter.extensionsForFormat('AAIGrid'), ['asc'])

    def testSupportedFiltersAndFormat(self):
        # test with formats in recommended order
        formats = QgsRasterFileWriter.supportedFiltersAndFormats(QgsRasterFileWriter.SortRecommended)
        self.assertEqual(formats[0].filterString, 'GeoTIFF (*.tif *.TIF *.tiff *.TIFF)')
        self.assertEqual(formats[0].driverName, 'GTiff')
        self.assertTrue('netCDF' in [f.driverName for f in formats])

        # alphabetical sorting
        formats2 = QgsRasterFileWriter.supportedFiltersAndFormats(QgsRasterFileWriter.RasterFormatOptions())
        self.assertTrue(formats2[0].driverName < formats2[1].driverName)
        self.assertCountEqual([f.driverName for f in formats], [f.driverName for f in formats2])
        self.assertNotEqual(formats2[0].driverName, 'GTiff')

    def testSupportedFormatExtensions(self):
        formats = QgsRasterFileWriter.supportedFormatExtensions()
        self.assertTrue('tif' in formats)
        self.assertFalse('exe' in formats)
        self.assertEqual(formats[0], 'tif')
        self.assertTrue('nc' in formats)

        # alphabetical sorting
        formats2 = QgsRasterFileWriter.supportedFormatExtensions(QgsRasterFileWriter.RasterFormatOptions())
        self.assertTrue(formats2[1] < formats2[2])
        self.assertCountEqual(formats, formats2)
        self.assertNotEqual(formats2[0], 'tif')

    def testImportIntoGpkg(self):
        # init target file
        test_gpkg = tempfile.mktemp(suffix='.gpkg', dir=self.testDataDir)
        gdal.GetDriverByName('GPKG').Create(test_gpkg, 1, 1, 1)
        source = QgsRasterLayer(os.path.join(self.testDataDir, 'raster', 'band3_byte_noct_epsg4326.tif'), 'my', 'gdal')
        self.assertTrue(source.isValid())
        provider = source.dataProvider()
        fw = QgsRasterFileWriter(test_gpkg)
        fw.setOutputFormat('gpkg')
        fw.setCreateOptions(['RASTER_TABLE=imported_table', 'APPEND_SUBDATASET=YES'])

        pipe = QgsRasterPipe()
        self.assertTrue(pipe.set(provider.clone()))

        projector = QgsRasterProjector()
        projector.setCrs(provider.crs(), provider.crs())
        self.assertTrue(pipe.set(projector))

        self.assertEqual(fw.writeRaster(pipe,
                                        provider.xSize(),
                                        provider.ySize(),
                                        provider.extent(),
                                        provider.crs()), 0)

        # Check that the test geopackage contains the raster layer and compare
        rlayer = QgsRasterLayer('GPKG:%s:imported_table' % test_gpkg)
        self.assertTrue(rlayer.isValid())
        out_provider = rlayer.dataProvider()
        for i in range(3):
            src_data = provider.block(i + 1, provider.extent(), source.width(), source.height())
            out_data = out_provider.block(i + 1, out_provider.extent(), rlayer.width(), rlayer.height())
            self.assertEqual(src_data.data(), out_data.data())

        # remove result file
        os.unlink(test_gpkg)

    def testExportToGpkgWithExtraExtent(self):
        tmpName = tempfile.mktemp(suffix='.gpkg')
        source = QgsRasterLayer(os.path.join(self.testDataDir, 'raster', 'band3_byte_noct_epsg4326.tif'), 'my', 'gdal')
        self.assertTrue(source.isValid())
        provider = source.dataProvider()
        fw = QgsRasterFileWriter(tmpName)
        fw.setOutputFormat('gpkg')

        pipe = QgsRasterPipe()
        self.assertTrue(pipe.set(provider.clone()))

        self.assertEqual(fw.writeRaster(pipe,
                                        provider.xSize() + 4,
                                        provider.ySize() + 4,
                                        QgsRectangle(-3 - 2, -4 - 2, 7 + 2, 6 + 2),
                                        provider.crs()), 0)
        del fw

        # Check that the test geopackage contains the raster layer and compare
        rlayer = QgsRasterLayer(tmpName)
        self.assertTrue(rlayer.isValid())
        out_provider = rlayer.dataProvider()
        for i in range(3):
            src_data = provider.block(i + 1, provider.extent(), source.width(), source.height())
            out_data = out_provider.block(i + 1, provider.extent(), source.width(), source.height())
            self.assertEqual(src_data.data(), out_data.data())
        out_data = out_provider.block(1, QgsRectangle(7, -4, 7 + 2, 6), 2, 8)
        # band3_byte_noct_epsg4326 nodata is 255
        self.assertEqual(out_data.data().data(), b'\xff' * 2 * 8)
        del out_provider
        del rlayer

        # remove result file
        os.unlink(tmpName)

    def testExportToGpkgWithExtraExtentNoNoData(self):
        tmpName = tempfile.mktemp(suffix='.gpkg')
        # Remove nodata
        gdal.Translate('/vsimem/src.tif', os.path.join(self.testDataDir, 'raster', 'band3_byte_noct_epsg4326.tif'), options='-a_nodata none')
        source = QgsRasterLayer('/vsimem/src.tif', 'my', 'gdal')
        self.assertTrue(source.isValid())
        provider = source.dataProvider()
        fw = QgsRasterFileWriter(tmpName)
        fw.setOutputFormat('gpkg')

        pipe = QgsRasterPipe()
        self.assertTrue(pipe.set(provider.clone()))

        self.assertEqual(fw.writeRaster(pipe,
                                        provider.xSize() + 4,
                                        provider.ySize() + 4,
                                        QgsRectangle(-3 - 2, -4 - 2, 7 + 2, 6 + 2),
                                        provider.crs()), 0)
        del fw

        # Check that the test geopackage contains the raster layer and compare
        rlayer = QgsRasterLayer(tmpName)
        self.assertTrue(rlayer.isValid())
        out_provider = rlayer.dataProvider()
        for i in range(3):
            src_data = provider.block(i + 1, provider.extent(), source.width(), source.height())
            out_data = out_provider.block(i + 1, provider.extent(), source.width(), source.height())
            self.assertEqual(src_data.data(), out_data.data())
        out_data = out_provider.block(1, QgsRectangle(7, -4, 7 + 2, 6), 2, 8)
        # No nodata: defaults to zero
        self.assertEqual(out_data.data().data(), b'\x00' * 2 * 8)
        del out_provider
        del rlayer

        # remove result file
        gdal.Unlink('/vsimem/src.tif')
        os.unlink(tmpName)

    def _testGeneratePyramids(self, pyramidFormat):
        tmpName = tempfile.mktemp(suffix='.tif')
        source = QgsRasterLayer(os.path.join(self.testDataDir, 'raster', 'byte.tif'), 'my', 'gdal')
        self.assertTrue(source.isValid())
        provider = source.dataProvider()
        fw = QgsRasterFileWriter(tmpName)

        fw.setBuildPyramidsFlag(QgsRaster.PyramidsFlagYes)
        fw.setPyramidsFormat(pyramidFormat)
        fw.setPyramidsList([2])

        pipe = QgsRasterPipe()
        self.assertTrue(pipe.set(provider.clone()))

        projector = QgsRasterProjector()
        projector.setCrs(provider.crs(), provider.crs())
        self.assertTrue(pipe.set(projector))

        self.assertEqual(fw.writeRaster(pipe,
                                        provider.xSize(),
                                        provider.ySize(),
                                        provider.extent(),
                                        provider.crs()), 0)
        del fw
        ds = gdal.Open(tmpName)
        self.assertEqual(ds.RasterCount, 1)
        self.assertEqual(ds.GetRasterBand(1).Checksum(), 4672)
        self.assertEqual(ds.GetRasterBand(1).GetOverviewCount(), 1)
        fl = ds.GetFileList()
        if pyramidFormat == QgsRaster.PyramidsGTiff:
            self.assertEqual(len(fl), 2, fl)
            self.assertIn('.ovr', fl[1])
        elif pyramidFormat == QgsRaster.PyramidsInternal:
            self.assertEqual(len(fl), 1, fl)
        elif pyramidFormat == QgsRaster.PyramidsErdas:
            self.assertEqual(len(fl), 2, fl)
            self.assertIn('.aux', fl[1])
        os.unlink(tmpName)

    def testGeneratePyramidsExternal(self):
        return self._testGeneratePyramids(QgsRaster.PyramidsGTiff)

    def testGeneratePyramidsInternal(self):
        return self._testGeneratePyramids(QgsRaster.PyramidsInternal)

    def testGeneratePyramidsErdas(self):
        return self._testGeneratePyramids(QgsRaster.PyramidsErdas)

    def testWriteAsRawInvalidOutputFile(self):
        tmpName = "/this/is/invalid/file.tif"
        source = QgsRasterLayer(os.path.join(self.testDataDir, 'raster', 'byte.tif'), 'my', 'gdal')
        self.assertTrue(source.isValid())
        provider = source.dataProvider()
        fw = QgsRasterFileWriter(tmpName)

        pipe = QgsRasterPipe()
        self.assertTrue(pipe.set(provider.clone()))

        self.assertEqual(fw.writeRaster(pipe,
                                        provider.xSize(),
                                        provider.ySize(),
                                        provider.extent(),
                                        provider.crs()), QgsRasterFileWriter.CreateDatasourceError)
        del fw

    def testWriteAsImage(self):
        tmpName = tempfile.mktemp(suffix='.tif')
        source = QgsRasterLayer(os.path.join(self.testDataDir, 'raster', 'byte.tif'), 'my', 'gdal')
        source.setContrastEnhancement(algorithm=QgsContrastEnhancement.NoEnhancement)
        self.assertTrue(source.isValid())
        provider = source.dataProvider()
        fw = QgsRasterFileWriter(tmpName)

        self.assertEqual(fw.writeRaster(source.pipe(),
                                        provider.xSize(),
                                        provider.ySize(),
                                        provider.extent(),
                                        provider.crs()), QgsRasterFileWriter.NoError)
        ds = gdal.Open(tmpName)
        self.assertEqual(ds.RasterCount, 4)
        self.assertEqual(ds.GetRasterBand(1).Checksum(), 4672)
        self.assertEqual(ds.GetRasterBand(2).Checksum(), 4672)
        self.assertEqual(ds.GetRasterBand(3).Checksum(), 4672)
        self.assertEqual(ds.GetRasterBand(4).Checksum(), 4873)
        ds = None

        del fw
        os.unlink(tmpName)

    def testWriteAsImageInvalidOutputPath(self):
        tmpName = "/this/is/invalid/file.tif"
        source = QgsRasterLayer(os.path.join(self.testDataDir, 'raster', 'byte.tif'), 'my', 'gdal')
        source.setContrastEnhancement(algorithm=QgsContrastEnhancement.NoEnhancement)
        self.assertTrue(source.isValid())
        provider = source.dataProvider()
        fw = QgsRasterFileWriter(tmpName)

        self.assertEqual(fw.writeRaster(source.pipe(),
                                        provider.xSize(),
                                        provider.ySize(),
                                        provider.extent(),
                                        provider.crs()), QgsRasterFileWriter.CreateDatasourceError)
        del fw

    def testWriteAsRawGS7BG(self):
        ''' Test that despite writing a Byte raster, we correctly handle GS7BG creating a Float64 '''
        tmpName = tempfile.mktemp(suffix='.grd')
        source = QgsRasterLayer(os.path.join(self.testDataDir, 'raster', 'byte.tif'), 'my', 'gdal')
        self.assertTrue(source.isValid())
        provider = source.dataProvider()
        fw = QgsRasterFileWriter(tmpName)
        fw.setOutputFormat('GS7BG')

        pipe = QgsRasterPipe()
        self.assertTrue(pipe.set(provider.clone()))

        self.assertEqual(fw.writeRaster(pipe,
                                        provider.xSize(),
                                        provider.ySize(),
                                        provider.extent(),
                                        provider.crs()), QgsRasterFileWriter.NoError)
        del fw

        ds = gdal.Open(tmpName)
        self.assertEqual(ds.RasterCount, 1)
        self.assertEqual(ds.GetRasterBand(1).Checksum(), 4672)
        ds = None
        os.unlink(tmpName)


if __name__ == '__main__':
    unittest.main()
