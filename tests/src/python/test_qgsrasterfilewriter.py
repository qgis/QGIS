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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
import glob
import tempfile

from osgeo import gdal
from qgis.PyQt.QtCore import QTemporaryFile, QDir
from qgis.core import (QgsRaster,
                       QgsRasterLayer,
                       QgsRasterChecker,
                       QgsRasterPipe,
                       QgsRasterFileWriter,
                       QgsRasterProjector)

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
        tmpFile.open()  # fileName is no avialable until open
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
        self.assertTrue(pipe.insert(2, projector))

        self.assertEqual(fw.writeRaster(pipe,
                                        provider.xSize(),
                                        provider.ySize(),
                                        provider.extent(),
                                        provider.crs()), 0)

        # Check that the test geopackage contains the raster layer and compare
        rlayer = QgsRasterLayer('GPKG:%s:imported_table' % test_gpkg)
        self.assertTrue(rlayer.isValid())
        out_provider = rlayer.dataProvider()
        self.assertEqual(provider.block(1, provider.extent(), source.width(), source.height()).data(),
                         out_provider.block(1, out_provider.extent(), rlayer.width(), rlayer.height()).data())

        # remove result file
        os.unlink(test_gpkg)

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
        self.assertTrue(pipe.insert(2, projector))

        self.assertEqual(fw.writeRaster(pipe,
                                        provider.xSize(),
                                        provider.ySize(),
                                        provider.extent(),
                                        provider.crs()), 0)
        del fw
        ds = gdal.Open(tmpName)
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


if __name__ == '__main__':
    unittest.main()
