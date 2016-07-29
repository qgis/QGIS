# -*- coding: utf-8 -*-
"""QGIS Unit tests for the OGR/MapInfo tab provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2016-01-28'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import tempfile
from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsVectorDataProvider
from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant
from qgis.testing import (
    start_app,
    unittest
)
import osgeo.gdal
from utilities import unitTestDataPath
import tempfile
import shutil
import glob

start_app()
TEST_DATA_DIR = unitTestDataPath()

# Note - doesn't implement ProviderTestCase as OGR provider is tested by the shapefile provider test


class TestPyQgsTabfileProvider(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.basetestpath = tempfile.mkdtemp()
        cls.dirs_to_cleanup = [cls.basetestpath]

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)

    def testDateTimeFormats(self):
        # check that date and time formats are correctly interpreted
        basetestfile = os.path.join(TEST_DATA_DIR, 'tab_file.tab')
        vl = QgsVectorLayer(u'{}|layerid=0'.format(basetestfile), u'test', u'ogr')

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('date')).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName('time')).type(), QVariant.Time)
        self.assertEqual(fields.at(fields.indexFromName('date_time')).type(), QVariant.DateTime)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        date_idx = vl.fieldNameIndex('date')
        assert isinstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 5, 3))
        time_idx = vl.fieldNameIndex('time')
        assert isinstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 00))
        datetime_idx = vl.fieldNameIndex('date_time')
        assert isinstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(QDate(2004, 5, 3), QTime(13, 41, 00)))

    # This test fails with GDAL version < 2 because tab update is new in GDAL 2.0
    @unittest.expectedFailure(int(osgeo.gdal.VersionInfo()[:1]) < 2)
    def testUpdateMode(self):
        """ Test that on-the-fly re-opening in update/read-only mode works """

        basetestfile = os.path.join(TEST_DATA_DIR, 'tab_file.tab')
        vl = QgsVectorLayer(u'{}|layerid=0'.format(basetestfile), u'test', u'ogr')
        caps = vl.dataProvider().capabilities()
        self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)

        # We should be really opened in read-only mode even if write capabilities are declared
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-only")

        # Test that startEditing() / commitChanges() plays with enterUpdateMode() / leaveUpdateMode()
        self.assertTrue(vl.startEditing())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-write")
        self.assertTrue(vl.dataProvider().isValid())

        self.assertTrue(vl.commitChanges())
        self.assertEquals(vl.dataProvider().property("_debug_open_mode"), "read-only")
        self.assertTrue(vl.dataProvider().isValid())


if __name__ == '__main__':
    unittest.main()
