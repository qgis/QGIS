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

import qgis  # NOQA

import os

from qgis.PyQt.QtGui import qRed

from qgis.core import (QgsRasterLayer,
                       QgsRectangle,
                       QgsRasterResampleFilter,
                       QgsSingleBandGrayRenderer,
                       QgsCubicRasterResampler,
                       QgsBilinearRasterResampler
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
        path = os.path.join(unitTestDataPath(), 'landsat.tif')
        raster_layer = QgsRasterLayer(path, 'test')
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
        path = os.path.join(unitTestDataPath(), 'landsat.tif')
        raster_layer = QgsRasterLayer(path, 'test')
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


if __name__ == '__main__':
    unittest.main()
