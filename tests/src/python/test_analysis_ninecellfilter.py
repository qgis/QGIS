"""
***************************************************************************
    test_analysis_ninecellfilter.py
    ---------------------
    Date                 : April 2025
    Copyright            : (C) 2025 by Alexander Bruy
    Email                : alexander dot bruy at gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Alexander Bruy"
__date__ = "April 2025"
__copyright__ = "(C) 2025, Alexander Bruy"

import os
import tempfile

from qgis.core import (
    QgsRasterChecker,
)
from qgis.analysis import (
    QgsAspectFilter,
)

import unittest
from qgis.testing import QgisTestCase

from utilities import unitTestDataPath, start_app

TEST_DATA_DIR = unitTestDataPath()
start_app()


class TestInterpolation(QgisTestCase):

    def __init__(self, methodName):
        QgisTestCase.__init__(self, methodName)
        self.report = "<h1>Python Raster Analysis nine cell filter Tests</h1>\n"

    def test_aspect(self):
        input_file = os.path.join(TEST_DATA_DIR, "analysis", "dem.tif")
        output_file = os.path.join(tempfile.gettempdir(), "aspect.tif")

        aspect = QgsAspectFilter(input_file, output_file, "GTiff")
        aspect.setZFactor(1.0)
        result = aspect.processRaster()
        self.assertEqual(result, 0)


if __name__ == "__main__":
    unittest.main()
