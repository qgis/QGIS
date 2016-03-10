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
import shutil
import glob

from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsFeature, QgsProviderRegistry
from PyQt.QtCore import QSettings, QDate, QTime, QDateTime, QVariant
from qgis.testing import (
    start_app,
    unittest
)
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()

# Note - doesn't implement ProviderTestCase as OGR provider is tested by the shapefile provider test


class TestPyQgsTabfileProvider(unittest.TestCase):

    def testDateTimeFormats(self):
        # check that date and time formats are correctly interpreted
        basetestfile = os.path.join(TEST_DATA_DIR, 'tab_file.tab')
        vl = QgsVectorLayer(u'{}|layerid=0'.format(basetestfile), u'test', u'ogr')

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName('date')).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName('time')).type(), QVariant.Time)
        self.assertEqual(fields.at(fields.indexFromName('date_time')).type(), QVariant.DateTime)

        f = vl.getFeatures(QgsFeatureRequest()).next()

        date_idx = vl.fieldNameIndex('date')
        assert isinstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 5, 3))
        time_idx = vl.fieldNameIndex('time')
        assert isinstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 00))
        datetime_idx = vl.fieldNameIndex('date_time')
        assert isinstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(QDate(2004, 5, 3), QTime(13, 41, 00)))

if __name__ == '__main__':
    unittest.main()
