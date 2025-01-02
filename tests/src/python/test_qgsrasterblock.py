"""QGIS Unit tests for QgsRasterBlock.

From build dir, run:
ctest -R PyQgsRasterBlock -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Till Frankenbach"
__date__ = "24/07/2024"
__copyright__ = "Copyright 2024, The QGIS Project"

import numpy

from qgis.core import (
    QgsRasterLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase
from qgis.PyQt.QtCore import QTemporaryDir

from test_qgsrasterattributetable import createTestRasters

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsRasterBlock(QgisTestCase):
    def setUp(self):
        self.temp_dir = QTemporaryDir()
        self.temp_path = self.temp_dir.path()

        createTestRasters(self, self.temp_path)

    def testQgsRasterBlock(self):
        raster = QgsRasterLayer(self.uri_2x2_2_BANDS_INT16)
        self.assertTrue(raster.isValid())
        provider = raster.dataProvider()

        # test without noDataValue set
        block = provider.block(1, raster.extent(), raster.width(), raster.height())
        expected_array = numpy.array([[0, 2], [4, 4]])
        self.assertTrue((block.as_numpy() == expected_array).all())

        # test with noDataValue set
        block.setNoDataValue(0)
        data = numpy.array([[numpy.nan, 2], [4, 4]])
        expected_masked_array = numpy.ma.masked_array(data, mask=numpy.isnan(data))
        self.assertTrue((block.as_numpy() == expected_masked_array).all())

        # test with noDataValue set and use_masking == False
        self.assertTrue((block.as_numpy(use_masking=False) == expected_array).all())


if __name__ == "__main__":
    unittest.main()
