# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsZonalStatistics.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alexander Bruy'
__date__ = '15/07/2013'
__copyright__ = 'Copyright 2013, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import QDir, QFile
from qgis.core import QgsVectorLayer, QgsRasterLayer, QgsFeature, QgsFeatureRequest
from qgis.analysis import QgsZonalStatistics

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class TestQgsZonalStatistics(unittest.TestCase):

    """Tests for zonal stats class."""

    def testStatistics(self):
        """Test zonal stats"""
        TEST_DATA_DIR = unitTestDataPath() + "/zonalstatistics/"
        myTempPath = QDir.tempPath() + "/"
        testDir = QDir(TEST_DATA_DIR)
        for f in testDir.entryList(QDir.Files):
            QFile.remove(myTempPath + f)
            QFile.copy(TEST_DATA_DIR + f, myTempPath + f)

        myVector = QgsVectorLayer(myTempPath + "polys.shp", "poly", "ogr")
        myRaster = QgsRasterLayer(myTempPath + "edge_problem.asc", "raster", "gdal")
        zs = QgsZonalStatistics(myVector, myRaster, "", 1, QgsZonalStatistics.All)
        zs.calculateStatistics(None)

        feat = QgsFeature()
        # validate statistics for each feature
        request = QgsFeatureRequest().setFilterFid(0)
        feat = next(myVector.getFeatures(request))
        myMessage = ('Expected: %f\nGot: %f\n' % (12.0, feat[1]))
        assert feat[1] == 12.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (8.0, feat[2]))
        assert feat[2] == 8.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.666666666666667, feat[3]))
        assert abs(feat[3] - 0.666666666666667) < 0.00001, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[4]))
        assert feat[4] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.47140452079103201, feat[5]))
        assert abs(feat[5] - 0.47140452079103201) < 0.00001, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.0, feat[6]))
        assert feat[6] == 0.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[7]))
        assert feat[7] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[8]))
        assert feat[8] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.0, feat[9]))
        assert feat[9] == 0.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[10]))
        assert feat[10] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (2.0, feat[11]))
        assert feat[11] == 2.0, myMessage

        request.setFilterFid(1)
        feat = next(myVector.getFeatures(request))
        myMessage = ('Expected: %f\nGot: %f\n' % (9.0, feat[1]))
        assert feat[1] == 9.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (5.0, feat[2]))
        assert feat[2] == 5.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.555555555555556, feat[3]))
        assert abs(feat[3] - 0.555555555555556) < 0.00001, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[4]))
        assert feat[4] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.49690399499995302, feat[5]))
        assert abs(feat[5] - 0.49690399499995302) < 0.00001, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.0, feat[6]))
        assert feat[6] == 0.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[7]))
        assert feat[7] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[8]))
        assert feat[8] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.0, feat[9]))
        assert feat[9] == 0.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[10]))
        assert feat[10] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (2.0, feat[11]))
        assert feat[11] == 2.0, myMessage

        request.setFilterFid(2)
        feat = next(myVector.getFeatures(request))
        myMessage = ('Expected: %f\nGot: %f\n' % (6.0, feat[1]))
        assert feat[1] == 6.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (5.0, feat[2]))
        assert feat[2] == 5.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.833333333333333, feat[3]))
        assert abs(feat[3] - 0.833333333333333) < 0.00001, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[4]))
        assert feat[4] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.372677996249965, feat[5]))
        assert abs(feat[5] - 0.372677996249965) < 0.00001, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.0, feat[6]))
        assert feat[6] == 0.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[7]))
        assert feat[7] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[8]))
        assert feat[8] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.0, feat[9]))
        assert feat[9] == 0.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (1.0, feat[10]))
        assert feat[10] == 1.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (2.0, feat[11]))
        assert feat[11] == 2.0, myMessage


if __name__ == '__main__':
    unittest.main()
