"""
***************************************************************************
    test_analysis_interpolation.py
    ---------------------
    Date                 : January 2025
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
__date__ = "January 2025"
__copyright__ = "(C) 2025, Alexander Bruy"

import os
import math
import tempfile

from qgis.core import (
    Qgis,
    QgsCoordinateTransformContext,
    QgsVectorLayer,
    QgsRasterChecker,
)
from qgis.analysis import (
    QgsInterpolator,
    QgsIDWInterpolator,
    QgsTinInterpolator,
    QgsGridFileWriter,
)

import unittest
from qgis.testing import QgisTestCase

from utilities import unitTestDataPath, start_app

TEST_DATA_DIR = unitTestDataPath()
start_app()


class TestInterpolation(QgisTestCase):

    def __init__(self, methodName):
        QgisTestCase.__init__(self, methodName)
        self.report = "<h1>Python Raster Analysis Interpolation Tests</h1>\n"

    def test_idw_interpolator(self):
        layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "points.shp"), "points", "ogr"
        )
        self.assertTrue(layer.isValid())

        pixel_size = 5
        extent = layer.extent()
        context = QgsCoordinateTransformContext()

        cols = max(math.ceil(extent.width() / pixel_size), 1)
        rows = max(math.ceil(extent.height() / pixel_size), 1)

        data = QgsInterpolator.LayerData()
        data.source = layer
        data.sourceType = QgsInterpolator.SourceType.SourcePoints
        data.transformContext = context
        data.valueSource = QgsInterpolator.ValueSource.ValueAttribute
        data.interpolationAttribute = 3

        interpolator = QgsIDWInterpolator([data])
        interpolator.setDistanceCoefficient(2.0)

        output_file = os.path.join(tempfile.gettempdir(), "idw_interpolation.tif")

        writer = QgsGridFileWriter(interpolator, output_file, extent, cols, rows)
        writer.writeFile()

        checker = QgsRasterChecker()
        ok = checker.runTest(
            "gdal",
            output_file,
            "gdal",
            os.path.join(TEST_DATA_DIR, "analysis", "idw_interpolation.tif"),
        )
        self.report += checker.report()

        report_file = os.path.join(tempfile.gettempdir(), "idw_interpolation_test.html")
        with open(report_file, "w", encoding="utf-8") as f:
            f.write(self.report)

        self.assertTrue(ok)

    def test_tin_interpolator(self):
        layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "points.shp"), "points", "ogr"
        )
        self.assertTrue(layer.isValid())

        pixel_size = 5
        extent = layer.extent()
        context = QgsCoordinateTransformContext()

        cols = max(math.ceil(extent.width() / pixel_size), 1)
        rows = max(math.ceil(extent.height() / pixel_size), 1)

        data = QgsInterpolator.LayerData()
        data.source = layer
        data.sourceType = QgsInterpolator.SourceType.SourcePoints
        data.transformContext = context
        data.valueSource = QgsInterpolator.ValueSource.ValueAttribute
        data.interpolationAttribute = 3

        interpolator = QgsTinInterpolator(
            [data], QgsTinInterpolator.TinInterpolation.Linear
        )

        output_file = os.path.join(tempfile.gettempdir(), "tin_interpolation.tif")

        writer = QgsGridFileWriter(interpolator, output_file, extent, cols, rows)
        writer.writeFile()

        checker = QgsRasterChecker()
        ok = checker.runTest(
            "gdal",
            output_file,
            "gdal",
            os.path.join(TEST_DATA_DIR, "analysis", "tin_interpolation.tif"),
        )
        self.report += checker.report()

        report_file = os.path.join(tempfile.gettempdir(), "tin_interpolation_test.html")
        with open(report_file, "w", encoding="utf-8") as f:
            f.write(self.report)

        self.assertTrue(ok)


if __name__ == "__main__":
    unittest.main()
