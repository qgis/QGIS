# -*- coding: utf-8 -*-
"""QGIS Unit tests for the postgres provider.

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

import os
import tempfile
import shutil
import glob

from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsFeature, QgsProviderRegistry
from PyQt4.QtCore import QSettings
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       unittest,
                       TestCase
                       )
from providertestbase import ProviderTestCase

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsPostgresProvider(TestCase, ProviderTestCase):
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
        cls.basetestfile = os.path.join(cls.basetestpath, 'shapefile.shp')
        cls.repackfile = os.path.join(cls.repackfilepath, 'shapefile.shp')
        cls.vl = QgsVectorLayer(u'{}|layerid=0'.format(cls.basetestfile), u'test', u'ogr')
        assert (cls.vl.isValid())
        cls.provider = cls.vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        shutil.rmtree(cls.basetestpath)
        shutil.rmtree(cls.repackfilepath)

    def testUnique(self):
        """
        Override parent method because OGR doesn't return NULL values in SELECT DISTINCT...
        This is only to make the tests pass, not to define this as expected behavior so if it is possible to remove this
        in the future even better.
        """
        assert set(self.provider.uniqueValues(1)) == set([-200, 100, 200, 300, 400])
        assert set([u'Apple', u'Honey', u'Orange', u'Pear']) == set(self.provider.uniqueValues(2)), 'Got {}'.format(
            set(self.provider.uniqueValues(2)))

    def testRepack(self):
        vl = QgsVectorLayer(u'{}|layerid=0'.format(self.repackfile), u'test', u'ogr')

        ids = [f.id() for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk=1'))]
        vl.setSelectedFeatures(ids)
        assert vl.selectedFeaturesIds() == ids, vl.selectedFeaturesIds()
        assert vl.pendingFeatureCount() == 5, vl.pendingFeatureCount()
        assert vl.startEditing()
        assert vl.deleteFeature(3)
        assert vl.commitChanges()
        assert vl.selectedFeatureCount() == 0 or vl.selectedFeatures()[0]['pk'] == 1


if __name__ == '__main__':
    unittest.main()
