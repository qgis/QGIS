# -*- coding: utf-8 -*-
"""QGIS Unit tests for the non-shapefile, non-tabfile datasources handled by OGR provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Mark Johnson'
__date__ = '2016-09-30'
__copyright__ = 'Copyright 2016, Mark Johnson'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import shutil
import sys
import tempfile

from qgis.core import Qgis, QgsVectorLayer, QgsVectorDataProvider, QgsWkbTypes, QgsFeatureRequest, QgsGeometry
from qgis.PyQt.QtCore import QDate
from qgis.testing import (
    start_app,
    unittest
)
from utilities import unitTestDataPath
from osgeo import gdal, osr, ogr
from inspect import getmembers
from pprint import pprint

start_app()
TEST_DATA_DIR = unitTestDataPath()


def count_opened_filedescriptors(filename_to_test):
    count = -1
    if sys.platform.startswith('linux'):
        count = 0
        open_files_dirname = '/proc/%d/fd' % os.getpid()
        filenames = os.listdir(open_files_dirname)
        for filename in filenames:
            full_filename = open_files_dirname + '/' + filename
            if os.path.exists(full_filename):
                link = os.readlink(full_filename)
                if os.path.basename(link) == os.path.basename(filename_to_test):
                    count += 1
    return count


class TestPyQgsSpatialiteProviderGeneral(unittest.TestCase):

    def setUp(self):
        """Run before each test."""

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

###############################################################################
# Test spatialite spatial views - writable -  checks if Capabilities are correctly set
# View positions_1925: contains only an INSERT trigger
# View positions_1955: contains an INSERT and UPDATE trigger
# View positions_1995: contains an INSERT, UPDATE and DELETE trigger
# to (re)create (a possibily damage) database with:
# spatialite gdal_220.autotest.ogr_spatialite_views_writable.sqlite < gdal_220.autotest.ogr_spatialite_views_writable.sql
#
    def test_01_SpatialiteViewsWritable(self):

        datasource = os.path.join(TEST_DATA_DIR, 'provider/spatialite_views_writable.sqlite')
        vl_positions = QgsVectorLayer(u'dbname={} table={} ({})'.format(datasource, "positions", "wsg84_center"), u'SpatialView_writable', u'spatialite')
        # vl_positions = QgsVectorLayer(u'dbname={} '.format(datasource), u'SpatialView_writable', u'spatialite')
        self.assertTrue(vl_positions.isValid())
        trigger_spatialview_insert=0;
        trigger_spatialview_update=0;
        trigger_spatialview_delete=0;
        spatialview_selectatid=0;
        spatialview_changegeonetries=0;
        count_fields=len(vl_positions.fields())
        if count_fields == 5:
            f = next(vl_positions.getFeatures())
            self.assertEqual(f.geometry().wkbType(), QgsWkbTypes.Point)
            # AttributeError: 'QgsFeature' object has no attribute 'constGeometry'
            self.assertEqual(len(vl_positions.fields()), 5)
            got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 1"))]
            self.assertEqual(got, [(u'Brandenburger Tor', u'Pariser Platz', QDate(1791, 8, 6).toString("yyyy-MM-dd"))])
            del vl_positions
            vl_positions_1925 = QgsVectorLayer(u'dbname={} table={} ({})'.format(datasource, "positions_1925", "wsg84_center"), u'SpatialView_writable', u'spatialite')
            count_fields=len(vl_positions_1925.fields())
            self.assertTrue(vl_positions_1925.isValid())
            self.assertEqual(vl_positions_1925.featureCount(), 3)
            if count_fields == 5:
                got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1925.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 2"))]
                self.assertEqual(got, [(u'Siegessäule', u'Königs Platz', u'1873-09-02')])
                got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1925.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 3"))]
                self.assertEqual(got, [(u'Ampelanlage', u'Potsdamer Platz, Verkehrsinsel', QDate(1924, 10, 24).toString("yyyy-MM-dd"))])
                caps = vl_positions_1925.dataProvider().capabilities()
                if caps & QgsVectorDataProvider.AddFeatures:
                    trigger_spatialview_insert=1;

            vl_positions_1955 = QgsVectorLayer(u'dbname={} table={} ({})'.format(datasource, "positions_1955", "wsg84_center"), u'SpatialView_writable', u'spatialite')
            self.assertTrue(vl_positions_1955.isValid())
            self.assertEqual(vl_positions_1955.featureCount(), 2)
            count_fields=len(vl_positions_1955.fields())
            if count_fields == 5:
                got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1955.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 4"))]
                self.assertEqual(got, [(u'Siegessäule', u'Große Stern', QDate(1939, 1, 1).toString("yyyy-MM-dd"))])
                caps = vl_positions_1955.dataProvider().capabilities()
                if caps & QgsVectorDataProvider.ChangeAttributeValues:
                    trigger_spatialview_update=1;

            vl_positions_1999 = QgsVectorLayer(u'dbname={} table={} ({})'.format(datasource, "positions_1999", "wsg84_center"), u'SpatialView_writable', u'spatialite')
            self.assertTrue(vl_positions_1999.isValid())
            self.assertEqual(vl_positions_1999.featureCount(), 3)
            count_fields=len(vl_positions_1999.fields())
            if count_fields == 5:
                got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1999.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 5"))]
                self.assertEqual(got, [(u'Ampelanlage', u'Potsdamer Platz', QDate(1998, 10, 2).toString("yyyy-MM-dd"))])
                caps = vl_positions_1999.dataProvider().capabilities()
                if caps & QgsVectorDataProvider.DeleteFeatures:
                    trigger_spatialview_delete=1;
                if caps & QgsVectorDataProvider.SelectAtId:
                    spatialview_selectatid=1;
                if caps & QgsVectorDataProvider.ChangeGeometries:
                    spatialview_changegeonetries=1;

        if (trigger_spatialview_insert and trigger_spatialview_update and trigger_spatialview_delete):
            # print('-I-> Can SpatialView({1}) SelectAtId[{0}] ChangeGeometries[{2}]'.format('positions_1999',spatialview_selectatid,spatialview_changegeonetries))
            caps = vl_positions_1925.dataProvider().capabilities()
            self.assertTrue(caps & QgsVectorDataProvider.EditingCapabilities)
            self.assertFalse(caps & QgsVectorDataProvider.ChangeAttributeValues)
            self.assertFalse(caps & QgsVectorDataProvider.DeleteFeatures)
            # print('-I-> SpatialView({0}) contains no TRIGGERs for UPDATE and DELETE (as expected), but has EditingCapabilities '.format('positions_1925'))
            caps = vl_positions_1955.dataProvider().capabilities()
            self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)
            self.assertFalse(caps & QgsVectorDataProvider.DeleteFeatures)
            # print('-I-> SpatialView({0}) contains no TRIGGER for DELETE (as expected)'.format('positions_1955'))
            caps = vl_positions_1999.dataProvider().capabilities()
            self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)
            self.assertTrue(caps & QgsVectorDataProvider.ChangeAttributeValues)
            # print('-I-> SpatialView({0}) contains TRIGGERs for UPDATE and DELETE (as expected)'.format('positions_1999'))
            self.assertTrue(vl_positions_1999.startEditing())
            # print('-I-> SpatialView({0}) changing valid_since to 1937-10-02 (from 1998-10-02)'.format ('positions_1999'))
            self.assertTrue(vl_positions_1999.dataProvider().changeAttributeValues({5: {3: QDate(1937, 10, 2)}}))
            self.assertTrue(vl_positions_1999.commitChanges())
            got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1955.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 5"))]
            self.assertEqual(got, [(u'Ampelanlage', u'Potsdamer Platz', QDate(1937, 10, 2).toString("yyyy-MM-dd"))])
            # print('-I-> SpatialView({0}) should now show 3 rows instead of 2: [{1}] rows (? but does not)'.format('positions_1955',vl_positions_1955.featureCount()))
            self.assertTrue(vl_positions_1955.startEditing())
            self.assertTrue(vl_positions_1955.dataProvider().changeAttributeValues({5: {3: QDate(1998, 10, 2)}}))
            self.assertTrue(vl_positions_1955.commitChanges())
            got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1999.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 5"))]
            self.assertEqual(got, [(u'Ampelanlage', u'Potsdamer Platz', QDate(1998, 10, 2).toString("yyyy-MM-dd"))])
            vl_positions_1999.rollBack(True)
            self.assertEqual(vl_positions_1955.featureCount(), 2)
            # Should delete the retrieved layers
            del vl_positions_1925
            del vl_positions_1955
            del vl_positions_1999

if __name__ == '__main__':
    unittest.main()
