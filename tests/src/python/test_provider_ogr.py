# -*- coding: utf-8 -*-
"""QGIS Unit tests for the non-shapefile, non-tabfile datasources handled by OGR provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '2016-04-11'
__copyright__ = 'Copyright 2016, Even Rouault'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import shutil
import tempfile

from qgis.core import QgsVectorLayer, QgsVectorDataProvider
from qgis.testing import (
    start_app,
    unittest
)
from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()

# Note - doesn't implement ProviderTestCase as most OGR provider is tested by the shapefile provider test


class PyQgsOGRProvider(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()
        cls.datasource = os.path.join(cls.basetestpath, 'test.csv')
        with open(cls.datasource, 'wt') as f:
            f.write('id,WKT\n')
            f.write('1,POINT(2 49)\n')

        cls.dirs_to_cleanup = [cls.basetestpath]

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)

    def testUpdateMode(self):

        vl = QgsVectorLayer(u'{}|layerid=0'.format(self.datasource), u'test', u'ogr')
        self.assertTrue(vl.isValid())
        caps = vl.dataProvider().capabilities()
        self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)

        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")

        # No-op
        self.assertTrue(vl.dataProvider().enterUpdateMode())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")

        # No-op
        self.assertTrue(vl.dataProvider().leaveUpdateMode())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")


if __name__ == '__main__':
    unittest.main()
