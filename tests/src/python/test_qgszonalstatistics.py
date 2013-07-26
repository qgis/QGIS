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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis

import os
from PyQt4.QtCore import *
from qgis.core import *
from qgis.analysis import *
from utilities import (
    unitTestDataPath,
    getQgisTestApp,
    TestCase,
    unittest)

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsZonalStatistics(TestCase):
    """Tests for zonal stats class."""

    def testStatistics(self):
        """Test zonal stats"""
        sep = os.sep
        TEST_DATA_DIR = unitTestDataPath() + sep + "zonalstatistics" + sep
        myTempPath = QDir.tempPath() + sep
        testDir = QDir(TEST_DATA_DIR)
        for f in testDir.entryList(QDir.Files):
            QFile.remove(myTempPath + f)
            QFile.copy(TEST_DATA_DIR + f, myTempPath + f)

        myVector = QgsVectorLayer(myTempPath + "polys.shp", "poly", "ogr")
        myRasterPath = myTempPath + "edge_problem.asc"
        zs = QgsZonalStatistics(myVector, myRasterPath, "", 1)
        zs.calculateStatistics(None)

        feat = QgsFeature()
        # validate statistics for each feature
        request = QgsFeatureRequest().setFilterFid(0)
        feat = myVector.getFeatures(request).next()
        myMessage = ('Expected: %f\nGot: %f\n' % (12.0, feat[1]))
        assert feat[1] == 12.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (8.0, feat[2]))
        assert feat[2] == 8.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.666666666666667, feat[3]))
        assert abs(feat[3] - 0.666666666666667) < 0.00001, myMessage

        request.setFilterFid(1)
        feat = myVector.getFeatures(request).next()
        myMessage = ('Expected: %f\nGot: %f\n' % (9.0, feat[1]))
        assert feat[1] == 9.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (5.0, feat[2]))
        assert feat[2] == 5.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.555555555555556, feat[3]))
        assert abs(feat[3] - 0.555555555555556) < 0.00001, myMessage

        request.setFilterFid(2)
        feat = myVector.getFeatures(request).next()
        myMessage = ('Expected: %f\nGot: %f\n' % (6.0, feat[1]))
        assert feat[1] == 6.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (5.0, feat[2]))
        assert feat[2] == 5.0, myMessage
        myMessage = ('Expected: %f\nGot: %f\n' % (0.833333333333333, feat[3]))
        assert abs(feat[3] - 0.833333333333333) < 0.00001, myMessage

if __name__ == '__main__':
    unittest.main()
