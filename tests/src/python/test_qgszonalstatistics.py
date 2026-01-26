"""QGIS Unit tests for QgsZonalStatistics.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alexander Bruy"
__date__ = "15/07/2013"
__copyright__ = "Copyright 2013, The QGIS Project"

import os
import shutil

from qgis.PyQt.QtCore import QDir, QFile, QTemporaryDir
from qgis.analysis import QgsZonalStatistics
from qgis.core import (
    QgsFeature,
    QgsFeatureRequest,
    QgsGeometry,
    QgsRasterLayer,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()

TEST_DATA_DIR = unitTestDataPath()


class TestQgsZonalStatistics(QgisTestCase):
    """Tests for zonal stats class."""

    def testStatistics(self):
        """Test zonal stats"""
        TEST_DATA_DIR = unitTestDataPath() + "/zonalstatistics/"
        myTempPath = QDir.tempPath() + "/"
        testDir = QDir(TEST_DATA_DIR)
        for f in testDir.entryList(QDir.Filter.Files):
            QFile.remove(myTempPath + f)
            QFile.copy(TEST_DATA_DIR + f, myTempPath + f)

        myVector = QgsVectorLayer(myTempPath + "polys.shp", "poly", "ogr")
        myRaster = QgsRasterLayer(myTempPath + "edge_problem.asc", "raster", "gdal")
        zs = QgsZonalStatistics(
            myVector, myRaster, "", 1, QgsZonalStatistics.Statistic.All
        )
        zs.calculateStatistics(None)

        feat = QgsFeature()
        # validate statistics for each feature
        request = QgsFeatureRequest().setFilterFid(0)
        feat = next(myVector.getFeatures(request))
        myMessage = f"Expected: {12.0:f}\nGot: {feat[1]:f}\n"
        assert feat[1] == 12.0, myMessage
        myMessage = f"Expected: {8.0:f}\nGot: {feat[2]:f}\n"
        assert feat[2] == 8.0, myMessage
        myMessage = f"Expected: {0.666666666666667:f}\nGot: {feat[3]:f}\n"
        assert abs(feat[3] - 0.666666666666667) < 0.00001, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[4]:f}\n"
        assert feat[4] == 1.0, myMessage
        myMessage = f"Expected: {0.47140452079103201:f}\nGot: {feat[5]:f}\n"
        assert abs(feat[5] - 0.47140452079103201) < 0.00001, myMessage
        myMessage = f"Expected: {0.0:f}\nGot: {feat[6]:f}\n"
        assert feat[6] == 0.0, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[7]:f}\n"
        assert feat[7] == 1.0, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[8]:f}\n"
        assert feat[8] == 1.0, myMessage
        myMessage = f"Expected: {0.0:f}\nGot: {feat[9]:f}\n"
        assert feat[9] == 0.0, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[10]:f}\n"
        assert feat[10] == 1.0, myMessage
        myMessage = f"Expected: {2.0:f}\nGot: {feat[11]:f}\n"
        assert feat[11] == 2.0, myMessage

        request.setFilterFid(1)
        feat = next(myVector.getFeatures(request))
        myMessage = f"Expected: {9.0:f}\nGot: {feat[1]:f}\n"
        assert feat[1] == 9.0, myMessage
        myMessage = f"Expected: {5.0:f}\nGot: {feat[2]:f}\n"
        assert feat[2] == 5.0, myMessage
        myMessage = f"Expected: {0.555555555555556:f}\nGot: {feat[3]:f}\n"
        assert abs(feat[3] - 0.555555555555556) < 0.00001, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[4]:f}\n"
        assert feat[4] == 1.0, myMessage
        myMessage = f"Expected: {0.49690399499995302:f}\nGot: {feat[5]:f}\n"
        assert abs(feat[5] - 0.49690399499995302) < 0.00001, myMessage
        myMessage = f"Expected: {0.0:f}\nGot: {feat[6]:f}\n"
        assert feat[6] == 0.0, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[7]:f}\n"
        assert feat[7] == 1.0, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[8]:f}\n"
        assert feat[8] == 1.0, myMessage
        myMessage = f"Expected: {0.0:f}\nGot: {feat[9]:f}\n"
        assert feat[9] == 0.0, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[10]:f}\n"
        assert feat[10] == 1.0, myMessage
        myMessage = f"Expected: {2.0:f}\nGot: {feat[11]:f}\n"
        assert feat[11] == 2.0, myMessage

        request.setFilterFid(2)
        feat = next(myVector.getFeatures(request))
        myMessage = f"Expected: {6.0:f}\nGot: {feat[1]:f}\n"
        assert feat[1] == 6.0, myMessage
        myMessage = f"Expected: {5.0:f}\nGot: {feat[2]:f}\n"
        assert feat[2] == 5.0, myMessage
        myMessage = f"Expected: {0.833333333333333:f}\nGot: {feat[3]:f}\n"
        assert abs(feat[3] - 0.833333333333333) < 0.00001, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[4]:f}\n"
        assert feat[4] == 1.0, myMessage
        myMessage = f"Expected: {0.372677996249965:f}\nGot: {feat[5]:f}\n"
        assert abs(feat[5] - 0.372677996249965) < 0.00001, myMessage
        myMessage = f"Expected: {0.0:f}\nGot: {feat[6]:f}\n"
        assert feat[6] == 0.0, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[7]:f}\n"
        assert feat[7] == 1.0, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[8]:f}\n"
        assert feat[8] == 1.0, myMessage
        myMessage = f"Expected: {0.0:f}\nGot: {feat[9]:f}\n"
        assert feat[9] == 0.0, myMessage
        myMessage = f"Expected: {1.0:f}\nGot: {feat[10]:f}\n"
        assert feat[10] == 1.0, myMessage
        myMessage = f"Expected: {2.0:f}\nGot: {feat[11]:f}\n"
        assert feat[11] == 2.0, myMessage

    def test_enum_conversion(self):
        """Test regression GH #43245"""

        tmp = QTemporaryDir()
        origin = os.path.join(TEST_DATA_DIR, "raster", "band1_byte_ct_epsg4326.tif")
        dest = os.path.join(tmp.path(), "band1_byte_ct_epsg4326.tif")
        shutil.copyfile(origin, dest)

        layer = QgsRasterLayer(dest, "rast", "gdal")

        stats = QgsZonalStatistics.calculateStatistics(
            layer.dataProvider(),
            QgsGeometry.fromWkt(layer.extent().asWktPolygon()),
            layer.rasterUnitsPerPixelX(),
            layer.rasterUnitsPerPixelY(),
            1,
            QgsZonalStatistics.Statistic.Max | QgsZonalStatistics.Statistic.Median,
        )

        self.assertEqual(
            sorted(list(stats.keys())),
            [QgsZonalStatistics.Statistic.Median, QgsZonalStatistics.Statistic.Max],
        )
        self.assertEqual(stats[QgsZonalStatistics.Statistic.Median], 142.0)
        self.assertEqual(stats[QgsZonalStatistics.Statistic.Max], 254.0)


if __name__ == "__main__":
    unittest.main()
