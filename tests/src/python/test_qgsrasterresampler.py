# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterResampler.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from builtins import str

__author__ = 'Nyall Dawson'
__date__ = '14/11/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

from osgeo import gdal
import struct
from contextlib import contextmanager
import tempfile

import qgis  # NOQA

import os
import shutil

from qgis.PyQt.QtGui import qRed

from qgis.core import (QgsRasterLayer,
                       QgsRectangle,
                       QgsRasterResampleFilter,
                       QgsSingleBandGrayRenderer,
                       QgsCubicRasterResampler,
                       QgsBilinearRasterResampler,
                       QgsRasterDataProvider
                       )
from utilities import unitTestDataPath
from qgis.testing import start_app, unittest

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsRasterResampler(unittest.TestCase):

    def checkBlockContents(self, block, expected):
        res = []
        for r in range(block.height()):
            res.append([qRed(block.color(r, c)) for c in range(block.width())])
        self.assertEqual(res, expected)

    def testBilinearResample(self):
        with tempfile.TemporaryDirectory() as dest_dir:
            path = os.path.join(unitTestDataPath(), 'landsat.tif')
            dest_path = shutil.copy(path, os.path.join(dest_dir, 'landsat.tif'))

            raster_layer = QgsRasterLayer(dest_path, 'test')
            raster_layer.dataProvider().setNoDataValue(1, -9999)
            self.assertTrue(raster_layer.isValid())

            extent = QgsRectangle(785994.37761193525511771,
                                  3346249.2209800467826426,
                                  786108.49096253968309611,
                                  3346362.94137834152206779)

            renderer = QgsSingleBandGrayRenderer(raster_layer.dataProvider(), 1)
            filter = QgsRasterResampleFilter(renderer)

            # default (nearest neighbour) resampling
            block = filter.block(1, extent, 2, 2)
            self.checkBlockContents(block, [[124, 127], [125, 126]])

            block = filter.block(1, extent, 4, 4)
            self.checkBlockContents(block, [[124, 124, 127, 127],
                                            [124, 124, 127, 127],
                                            [125, 125, 126, 126],
                                            [125, 125, 126, 126]]
                                    )

            block = filter.block(1, extent, 8, 8)
            self.checkBlockContents(block, [[124, 124, 124, 124, 127, 127, 127, 127],
                                            [124, 124, 124, 124, 127, 127, 127, 127],
                                            [124, 124, 124, 124, 127, 127, 127, 127],
                                            [124, 124, 124, 124, 127, 127, 127, 127],
                                            [125, 125, 125, 125, 126, 126, 126, 126],
                                            [125, 125, 125, 125, 126, 126, 126, 126],
                                            [125, 125, 125, 125, 126, 126, 126, 126],
                                            [125, 125, 125, 125, 126, 126, 126, 126]])

            # with resampling
            filter.setZoomedInResampler(QgsBilinearRasterResampler())
            block = filter.block(1, extent, 2, 2)
            self.checkBlockContents(block, [[124, 127], [125, 126]])

            block = filter.block(1, extent, 4, 4)
            self.checkBlockContents(block,
                                    [[124, 124, 126, 126],
                                     [124, 124, 125, 126],
                                     [124, 124, 125, 126],
                                     [125, 125, 125, 126]]
                                    )

            block = filter.block(1, extent, 8, 8)
            self.checkBlockContents(block,
                                    [[124, 124, 124, 125, 125, 126, 126, 126],
                                     [124, 124, 124, 125, 125, 126, 126, 126],
                                     [124, 124, 124, 124, 125, 125, 126, 126],
                                     [124, 124, 124, 124, 125, 125, 126, 126],
                                     [124, 124, 124, 124, 125, 125, 126, 126],
                                     [124, 124, 124, 124, 125, 125, 126, 126],
                                     [125, 125, 125, 125, 125, 125, 126, 126],
                                     [125, 125, 125, 125, 125, 125, 126, 126]]
                                    )

            # with oversampling
            extent = QgsRectangle(785878.92593475803732872, 3346136.27493690419942141, 786223.56509550288319588, 3346477.7564090033993125)
            block = filter.block(1, extent, 2, 2)
            self.checkBlockContents(block, [[127, 126], [125, 126]])

            block = filter.block(1, extent, 4, 4)
            self.checkBlockContents(block,
                                    [[125, 127, 127, 127],
                                     [126, 127, 127, 126],
                                     [125, 126, 126, 126],
                                     [127, 125, 125, 125]]
                                    )

            block = filter.block(1, extent, 8, 8)
            self.checkBlockContents(block,
                                    [[126, 126, 126, 126, 125, 125, 125, 126],
                                     [126, 126, 125, 125, 125, 126, 126, 126],
                                     [126, 125, 124, 124, 125, 126, 126, 126],
                                     [126, 125, 124, 124, 125, 126, 126, 126],
                                     [125, 125, 124, 124, 124, 126, 126, 126],
                                     [125, 125, 125, 125, 125, 126, 126, 126],
                                     [125, 125, 125, 125, 125, 126, 126, 126],
                                     [125, 125, 126, 126, 125, 125, 125, 125]]
                                    )

            filter.setMaxOversampling(2)
            block = filter.block(1, extent, 2, 2)
            self.checkBlockContents(block, [[127, 126], [125, 126]])

            block = filter.block(1, extent, 4, 4)
            self.checkBlockContents(block,
                                    [[125, 127, 127, 127],
                                     [126, 127, 127, 126],
                                     [125, 126, 126, 126],
                                     [127, 125, 125, 125]]
                                    )

            block = filter.block(1, extent, 8, 8)
            self.checkBlockContents(block,
                                    [[126, 126, 126, 126, 125, 125, 125, 126],
                                     [126, 126, 125, 125, 125, 126, 126, 126],
                                     [126, 125, 124, 124, 125, 126, 126, 126],
                                     [126, 125, 124, 124, 125, 126, 126, 126],
                                     [125, 125, 124, 124, 124, 126, 126, 126],
                                     [125, 125, 125, 125, 125, 126, 126, 126],
                                     [125, 125, 125, 125, 125, 126, 126, 126],
                                     [125, 125, 126, 126, 125, 125, 125, 125]]
                                    )

            filter.setMaxOversampling(4)
            block = filter.block(1, extent, 2, 2)
            self.checkBlockContents(block, [[127, 126], [125, 126]])

            block = filter.block(1, extent, 4, 4)
            self.checkBlockContents(block,
                                    [[125, 127, 127, 127],
                                     [126, 127, 127, 126],
                                     [125, 126, 126, 126],
                                     [127, 125, 125, 125]]
                                    )

            block = filter.block(1, extent, 8, 8)
            self.checkBlockContents(block,
                                    [[126, 126, 126, 126, 125, 125, 125, 126],
                                     [126, 126, 125, 125, 125, 126, 126, 126],
                                     [126, 125, 124, 124, 125, 126, 126, 126],
                                     [126, 125, 124, 124, 125, 126, 126, 126],
                                     [125, 125, 124, 124, 124, 126, 126, 126],
                                     [125, 125, 125, 125, 125, 126, 126, 126],
                                     [125, 125, 125, 125, 125, 126, 126, 126],
                                     [125, 125, 126, 126, 125, 125, 125, 125]]
                                    )

    def testCubicResample(self):
        with tempfile.TemporaryDirectory() as dest_dir:
            path = os.path.join(unitTestDataPath(), 'landsat.tif')
            dest_path = shutil.copy(path, os.path.join(dest_dir, 'landsat.tif'))

            raster_layer = QgsRasterLayer(dest_path, 'test')
            raster_layer.dataProvider().setNoDataValue(1, -9999)
            self.assertTrue(raster_layer.isValid())

            extent = QgsRectangle(785994.37761193525511771,
                                  3346249.2209800467826426,
                                  786108.49096253968309611,
                                  3346362.94137834152206779)

            renderer = QgsSingleBandGrayRenderer(raster_layer.dataProvider(), 1)
            filter = QgsRasterResampleFilter(renderer)

            # default (nearest neighbour) resampling
            block = filter.block(1, extent, 2, 2)
            self.checkBlockContents(block, [[124, 127], [125, 126]])

            block = filter.block(1, extent, 4, 4)
            self.checkBlockContents(block, [[124, 124, 127, 127],
                                            [124, 124, 127, 127],
                                            [125, 125, 126, 126],
                                            [125, 125, 126, 126]]
                                    )

            block = filter.block(1, extent, 8, 8)
            self.checkBlockContents(block, [[124, 124, 124, 124, 127, 127, 127, 127],
                                            [124, 124, 124, 124, 127, 127, 127, 127],
                                            [124, 124, 124, 124, 127, 127, 127, 127],
                                            [124, 124, 124, 124, 127, 127, 127, 127],
                                            [125, 125, 125, 125, 126, 126, 126, 126],
                                            [125, 125, 125, 125, 126, 126, 126, 126],
                                            [125, 125, 125, 125, 126, 126, 126, 126],
                                            [125, 125, 125, 125, 126, 126, 126, 126]])

            # with resampling
            filter.setZoomedInResampler(QgsCubicRasterResampler())
            block = filter.block(1, extent, 2, 2)
            self.checkBlockContents(block, [[124, 127], [125, 126]])

            block = filter.block(1, extent, 4, 4)
            self.checkBlockContents(block,
                                    [[124, 125, 127, 127],
                                     [124, 125, 126, 127],
                                     [125, 125, 126, 126],
                                     [125, 125, 126, 126]]
                                    )

            block = filter.block(1, extent, 8, 8)
            self.checkBlockContents(block,
                                    [[125, 124, 124, 125, 126, 127, 127, 127],
                                     [125, 124, 124, 125, 126, 127, 127, 127],
                                     [125, 124, 124, 125, 126, 127, 127, 127],
                                     [125, 124, 124, 125, 126, 126, 127, 127],
                                     [125, 125, 125, 125, 126, 126, 126, 126],
                                     [125, 125, 125, 125, 126, 126, 126, 126],
                                     [125, 125, 125, 125, 126, 126, 126, 126],
                                     [125, 125, 125, 125, 126, 126, 126, 126]]
                                    )

            # with oversampling
            extent = QgsRectangle(785878.92593475803732872, 3346136.27493690419942141, 786223.56509550288319588, 3346477.7564090033993125)
            block = filter.block(1, extent, 2, 2)
            self.checkBlockContents(block, [[127, 126], [125, 126]])

            block = filter.block(1, extent, 4, 4)
            self.checkBlockContents(block,
                                    [[125, 127, 127, 127],
                                     [126, 127, 127, 126],
                                     [125, 126, 126, 126],
                                     [127, 125, 125, 125]]
                                    )

            block = filter.block(1, extent, 8, 8)
            self.checkBlockContents(block,
                                    [[127, 126, 127, 126, 126, 125, 126, 126],
                                     [127, 127, 127, 126, 126, 126, 126, 126],
                                     [126, 127, 125, 125, 126, 127, 127, 127],
                                     [126, 126, 126, 124, 126, 127, 126, 127],
                                     [126, 126, 125, 124, 125, 126, 126, 127],
                                     [126, 126, 125, 125, 125, 126, 127, 127],
                                     [126, 125, 127, 125, 125, 127, 128, 126],
                                     [126, 125, 127, 127, 126, 125, 125, 126]]
                                    )

            filter.setMaxOversampling(2)
            block = filter.block(1, extent, 2, 2)
            self.checkBlockContents(block, [[127, 126], [125, 126]])

            block = filter.block(1, extent, 4, 4)
            self.checkBlockContents(block,
                                    [[125, 127, 127, 127],
                                     [126, 127, 127, 126],
                                     [125, 126, 126, 126],
                                     [127, 125, 125, 125]]
                                    )

            block = filter.block(1, extent, 8, 8)
            self.checkBlockContents(block,
                                    [[127, 126, 127, 126, 126, 125, 126, 126],
                                     [127, 127, 127, 126, 126, 126, 126, 126],
                                     [126, 127, 125, 125, 126, 127, 127, 127],
                                     [126, 126, 126, 124, 126, 127, 126, 127],
                                     [126, 126, 125, 124, 125, 126, 126, 127],
                                     [126, 126, 125, 125, 125, 126, 127, 127],
                                     [126, 125, 127, 125, 125, 127, 128, 126],
                                     [126, 125, 127, 127, 126, 125, 125, 126]]
                                    )

            filter.setMaxOversampling(4)
            block = filter.block(1, extent, 2, 2)
            self.checkBlockContents(block, [[127, 126], [125, 126]])

            block = filter.block(1, extent, 4, 4)
            self.checkBlockContents(block,
                                    [[125, 127, 127, 127],
                                     [126, 127, 127, 126],
                                     [125, 126, 126, 126],
                                     [127, 125, 125, 125]]
                                    )

            block = filter.block(1, extent, 8, 8)
            self.checkBlockContents(block,
                                    [[127, 126, 127, 126, 126, 125, 126, 126],
                                     [127, 127, 127, 126, 126, 126, 126, 126],
                                     [126, 127, 125, 125, 126, 127, 127, 127],
                                     [126, 126, 126, 124, 126, 127, 126, 127],
                                     [126, 126, 125, 124, 125, 126, 126, 127],
                                     [126, 126, 125, 125, 125, 126, 127, 127],
                                     [126, 125, 127, 125, 125, 127, 128, 126],
                                     [126, 125, 127, 127, 126, 125, 125, 126]]
                                    )

    @contextmanager
    def setupGDALResampling(self):

        temp_folder = tempfile.mkdtemp()
        tmpfilename = os.path.join(temp_folder, 'test.tif')

        ds = gdal.GetDriverByName('GTiff').Create(tmpfilename, 5, 5)
        xmin = 100
        ymax = 1000
        xres = 5
        yres = 5
        ds.SetGeoTransform([xmin, xres, 0, ymax, 0, -yres])
        ds.WriteRaster(0, 0, 5, 5,
                       struct.pack('B' * 5 * 5,
                                   10, 20, 30, 40, 150,
                                   20, 30, 40, 50, 160,
                                   30, 40, 50, 60, 170,
                                   40, 50, 60, 70, 180,
                                   50, 60, 70, 80, 190))
        ds = None

        raster_layer = QgsRasterLayer(tmpfilename, 'test')
        self.assertTrue(raster_layer.isValid())

        raster_layer.dataProvider().enableProviderResampling(True)

        try:
            yield raster_layer.dataProvider()
        finally:
            gdal.Unlink(tmpfilename)

    # Return an extent, fully inside raster, intersecting 2x2 pixels,
    # with a shift of 25% of a cell
    def _getExtentRequestInside(self):
        xmin = 100
        ymax = 1000
        xres = 5
        yres = 5

        extent = QgsRectangle(xmin + 2.25 * xres,
                              ymax - 4.25 * yres,
                              xmin + 4.25 * xres,
                              ymax - 2.25 * yres)
        return extent

    def checkRawBlockContents(self, block, expected):
        res = []
        for r in range(block.height()):
            res.append([(0 if block.isNoData(r, c) else block.value(r, c)) for c in range(block.width())])
        self.assertEqual(res, expected)

    def testGDALResampling_nearest(self):

        with self.setupGDALResampling() as provider:
            # default (nearest neighbour) resampling
            block = provider.block(1, self._getExtentRequestInside(), 2, 2)
            self.checkRawBlockContents(block, [[50, 60], [60, 70]])

    def testGDALResampling_nominal_resolution_zoomed_in_bilinear(self):

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            block = provider.block(1, self._getExtentRequestInside(), 2, 2)
            self.checkRawBlockContents(block, [[55, 90], [65, 100]])

    def testGDALResampling_nominal_resolution_zoomed_in_cubic(self):

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Cubic)
            block = provider.block(1, self._getExtentRequestInside(), 2, 2)
            self.checkRawBlockContents(block, [[53, 88], [63, 98]])

    def testGDALResampling_nominal_resolution_zoomed_out_bilinear(self):

        with self.setupGDALResampling() as provider:
            provider.setZoomedOutResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            block = provider.block(1, self._getExtentRequestInside(), 2, 2)
            self.checkRawBlockContents(block, [[55, 90], [65, 100]])

    def testGDALResampling_nominal_resolution_zoomed_out_cubic(self):

        with self.setupGDALResampling() as provider:
            provider.setZoomedOutResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Cubic)
            block = provider.block(1, self._getExtentRequestInside(), 2, 2)
            self.checkRawBlockContents(block, [[53, 88], [63, 98]])

    def testGDALResampling_oversampling_bilinear(self):

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            provider.setZoomedOutResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Cubic)  # ignored
            block = provider.block(1, self._getExtentRequestInside(), 4, 4)
            self.checkRawBlockContents(block, [[50, 55, 60, 115], [55, 60, 65, 120], [60, 65, 70, 125], [65, 70, 75, 130]])

    def testGDALResampling_oversampling_cubic(self):

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Cubic)
            provider.setZoomedOutResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)  # ignored
            block = provider.block(1, self._getExtentRequestInside(), 4, 4)
            self.checkRawBlockContents(block, [[50, 49, 60, 119], [55, 54, 65, 124], [60, 59, 70, 129], [66, 65, 76, 135]])

    def testGDALResampling_downsampling_bilinear(self):

        with self.setupGDALResampling() as provider:
            provider.setZoomedOutResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Cubic)  # ignored
            block = provider.block(1, self._getExtentRequestInside(), 1, 1)
            self.checkRawBlockContents(block, [[84]])

    def testGDALResampling_downsampling_cubic(self):

        with self.setupGDALResampling() as provider:
            provider.setZoomedOutResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Cubic)
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)  # ignored
            block = provider.block(1, self._getExtentRequestInside(), 1, 1)
            self.checkRawBlockContents(block, [[86]])

    def testGDALResampling_downsampling_bilinear_beyond_max_oversampling_factor(self):

        with self.setupGDALResampling() as provider:
            provider.setMaxOversampling(1.5)
            provider.setZoomedOutResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Cubic)  # ignored
            block = provider.block(1, self._getExtentRequestInside(), 1, 1)
            # as we request at a x2 oversampling factor and the limit is 1.5
            # fallback to an alternate method using first nearest resampling
            # and then bilinear
            self.checkRawBlockContents(block, [[120]])

    def testGDALResampling_downsampling_bilinear_beyond_max_oversampling_factor_containing_raster_extent(self):

        xmin = 100
        ymax = 1000
        xres = 5
        yres = 5

        extent = QgsRectangle(xmin - 10 * xres,
                              ymax - (5 + 10) * yres,
                              xmin + (5 + 10) * xres,
                              ymax + 10 * yres)

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            provider.setMaxOversampling(1)
            block = provider.block(1, extent, 3, 3)
            self.checkRawBlockContents(block, [[0, 0, 0], [0, 50.0, 0], [0, 0, 0]])

    def testGDALResampling_nominal_resolution_containing_raster_extent(self):

        xmin = 100
        ymax = 1000
        xres = 5
        yres = 5

        extent = QgsRectangle(xmin - 2.25 * xres,
                              ymax - (5 + 2.75) * yres,
                              xmin + (5 + 2.75) * xres,
                              ymax + 2.25 * yres)

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            block = provider.block(1, extent, 10, 10)
            self.checkRawBlockContents(block, [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                                               [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                                               [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                                               [0, 0, 0, 25, 35, 45, 130, 0, 0, 0],
                                               [0, 0, 0, 35, 45, 55, 140, 0, 0, 0],
                                               [0, 0, 0, 45, 55, 65, 150, 0, 0, 0],
                                               [0, 0, 0, 55, 65, 75, 160, 0, 0, 0],
                                               [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                                               [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                                               [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]])

    def testGDALResampling_nominal_resolution_aligned_containing_raster_extent(self):

        xmin = 100
        ymax = 1000
        xres = 5
        yres = 5

        extent = QgsRectangle(xmin - 2 * xres,
                              ymax - (5 + 3) * yres,
                              xmin + (5 + 3) * xres,
                              ymax + 2 * yres)

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            block = provider.block(1, extent, 10, 10)
            self.checkRawBlockContents(block, [[0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                                               [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                                               [0, 0, 10, 20, 30, 40, 150, 0, 0, 0],
                                               [0, 0, 20, 30, 40, 50, 160, 0, 0, 0],
                                               [0, 0, 30, 40, 50, 60, 170, 0, 0, 0],
                                               [0, 0, 40, 50, 60, 70, 180, 0, 0, 0],
                                               [0, 0, 50, 60, 70, 80, 190, 0, 0, 0],
                                               [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                                               [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                                               [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]])

    def testGDALResampling_nominal_resolution_slightly_overlapping_left_edge(self):

        xmin = 100
        ymax = 1000
        xres = 5
        yres = 5

        extent = QgsRectangle(xmin - 0.2 * xres,
                              ymax - 4.25 * yres,
                              xmin + 1.8 * xres,
                              ymax - 2.25 * yres)

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            block = provider.block(1, extent, 2, 2)
            # The values in the left column are not subject to resampling currently
            self.checkRawBlockContents(block, [[30, 41], [50, 51]])

    def testGDALResampling_nominal_resolution_slightly_overlapping_top_edge(self):

        xmin = 100
        ymax = 1000
        xres = 5
        yres = 5

        extent = QgsRectangle(xmin + 2.25 * xres,
                              ymax - 1.8 * yres,
                              xmin + 4.25 * xres,
                              ymax + 0.2 * yres)

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            block = provider.block(1, extent, 2, 2)
            # The values in the top line are not subject to resampling currently
            self.checkRawBlockContents(block, [[30, 150], [41, 76]])

    def testGDALResampling_nominal_resolution_slightly_overlapping_right_edge(self):

        xmin = 100
        ymax = 1000
        xres = 5
        yres = 5

        extent = QgsRectangle(xmin + (5 - 2 + 0.2) * xres,
                              ymax - 4.25 * yres,
                              xmin + (5 + 0.2) * xres,
                              ymax - 2.25 * yres)

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            block = provider.block(1, extent, 2, 2)
            # The values in the right column are not subject to resampling currently
            self.checkRawBlockContents(block, [[85, 170], [95, 190]])

    def testGDALResampling_nominal_resolution_slightly_overlapping_bottom_edge(self):

        xmin = 100
        ymax = 1000
        xres = 5
        yres = 5

        extent = QgsRectangle(xmin + 2.25 * xres,
                              ymax - (5 + 0.2) * yres,
                              xmin + 4.25 * xres,
                              ymax - (5 - 2 + 0.2) * yres)

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            block = provider.block(1, extent, 2, 2)
            # The values in the bottom line are not subject to resampling currently
            self.checkRawBlockContents(block, [[65, 100], [70, 190]])

    def testGDALResampling_less_than_one_pixel(self):

        xmin = 100
        ymax = 1000
        xres = 5
        yres = 5

        # Extent is less than one pixel. Simulates pixel identification
        extent = QgsRectangle(xmin + 2.25 * xres,
                              ymax - 4.25 * yres,
                              xmin + 2.5 * xres,
                              ymax - 4.5 * yres)

        with self.setupGDALResampling() as provider:
            provider.setZoomedInResamplingMethod(QgsRasterDataProvider.ResamplingMethod.Bilinear)
            block = provider.block(1, extent, 1, 1)
            self.checkRawBlockContents(block, [[68]])


if __name__ == '__main__':
    unittest.main()
