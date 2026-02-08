"""QGIS Unit tests for QgsApplication.

From build dir: ctest -R PyQgsAppInteractWithGDAL -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Even Rouault"
__date__ = "31/01/2026"
__copyright__ = "Copyright 2026, The QGIS Project"

import os
from osgeo import gdal

from qgis.testing import unittest, start_app

from utilities import unitTestDataPath

start_app()

global_dataset = None


class TestPyQgsAppInteractWithGDAL(unittest.TestCase):

    def test1(self):

        path = os.path.join(unitTestDataPath("raster"), "rgb_with_mask.tif")
        ds = gdal.Open(path)
        self.assertIsNotNone(ds)

        # make sure this is a global variable so that the dataset is not closed
        # immediately
        global global_dataset
        global_dataset = ds


if __name__ == "__main__":
    unittest.main()
