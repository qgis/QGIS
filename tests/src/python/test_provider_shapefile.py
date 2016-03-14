# -*- coding: utf-8 -*-
"""QGIS Unit tests for the OGR/Shapefile provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '2015-04-23'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os
import tempfile
import shutil
import glob

from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsFeature, QgsProviderRegistry
from PyQt4.QtCore import QSettings
from qgis.testing import (start_app,
                          unittest
                          )
from utilities import unitTestDataPath
from providertestbase import ProviderTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsShapefileProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()
        cls.repackfilepath = tempfile.mkdtemp()

        srcpath = os.path.join(TEST_DATA_DIR, 'provider')
        for file in glob.glob(os.path.join(srcpath, 'shapefile.*')):
            shutil.copy(os.path.join(srcpath, file), cls.basetestpath)
            shutil.copy(os.path.join(srcpath, file), cls.repackfilepath)
        for file in glob.glob(os.path.join(srcpath, 'shapefile_poly.*')):
            shutil.copy(os.path.join(srcpath, file), cls.basetestpath)
        cls.basetestfile = os.path.join(cls.basetestpath, 'shapefile.shp')
        cls.repackfile = os.path.join(cls.repackfilepath, 'shapefile.shp')
        cls.basetestpolyfile = os.path.join(cls.basetestpath, 'shapefile_poly.shp')
        cls.vl = QgsVectorLayer(u'{}|layerid=0'.format(cls.basetestfile), u'test', u'ogr')
        assert(cls.vl.isValid())
        cls.provider = cls.vl.dataProvider()
        cls.vl_poly = QgsVectorLayer(u'{}|layerid=0'.format(cls.basetestpolyfile), u'test', u'ogr')
        assert (cls.vl_poly.isValid())
        cls.poly_provider = cls.vl_poly.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        shutil.rmtree(cls.basetestpath, True)
        shutil.rmtree(cls.repackfilepath, True)

    def enableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', True)

    def disableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', False)

    def testRepack(self):
        vl = QgsVectorLayer(u'{}|layerid=0'.format(self.repackfile), u'test', u'ogr')

        ids = [f.id() for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk=1'))]
        vl.setSelectedFeatures(ids)
        self.assertEquals(vl.selectedFeaturesIds(), ids)
        self.assertEquals(vl.pendingFeatureCount(), 5)
        self.assertTrue(vl.startEditing())
        self.assertTrue(vl.deleteFeature(3))
        self.assertTrue(vl.commitChanges())
        self.assertTrue(vl.selectedFeatureCount() == 0 or vl.selectedFeatures()[0]['pk'] == 1)


if __name__ == '__main__':
    unittest.main()
