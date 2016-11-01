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
import sys
import tempfile

from qgis.core import QGis, QgsVectorLayer, QgsVectorDataProvider, QgsWKBTypes, QgsFeatureRequest
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


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)

# Note - doesn't implement ProviderTestCase as most OGR provider is tested by the shapefile provider test


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


class TestPyQgsOGRProviderGeneral(unittest.TestCase):

    ogr_spatialview_insert=0
    ogr_spatialview_update=0
    ogr_spatialview_delete=0
    ogr_spatialview_selectatid=0
    ogr_spatialview_changegeonetries=0
    gdal_version_num=0
    gdal_build_num=0
    gdal_build_version=""
    gdal_runtime_version=""
    ogr_runtime_supported=1

    def setUp(self):
        """Run before each test."""
        self.gdal_version_num = int(gdal.VersionInfo('VERSION_NUM'))
        try:
            QGis.GDAL_BUILD_VERSION
        except AttributeError:
            self.gdal_build_num = self.gdal_version_num
            self.gdal_build_version = gdal.VersionInfo('VERSION_NUM')
            self.gdal_runtime_version = gdal.VersionInfo('VERSION_NUM')
        else:
            self.ogr_runtime_supported = QGis.GDAL_OGR_RUNTIME_SUPPORTED
            self.gdal_build_version = QGis.GDAL_BUILD_VERSION
            self.gdal_runtime_version = QGis.GDAL_RUNTIME_VERSION
            self.gdal_build_num = (QGis.GDAL_BUILD_VERSION_MAJOR*1000000)+(QGis.GDAL_BUILD_VERSION_MINOR*10000)+(QGis.GDAL_BUILD_VERSION_REV*100)

        print('-I-> Using version of gdal/ogr[%d,%s] qgis built with gdal[%d,%s] ogr_runtime_supported[%d]' % (self.gdal_version_num,self.gdal_runtime_version, self.gdal_build_num,self.gdal_build_version,self.ogr_runtime_supported))

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""

      
        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()
        cls.dirs_to_cleanup = [cls.basetestpath]

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)

###############################################################################
# Test spatialite spatial views - writable -  #5582: SpatialView - writable
# to (re)create (a possibily damage) database with:
# spatialite gdal_220.autotest.ogr_spatialite_views_writable.sqlite < gdal_220.autotest.ogr_spatialite_views_writable.sql
# Note: this function should run first
#
    def test_00_OgrSpatialiteViewsWritable(self):

        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrSpatialiteViewsWritable: Deprecated  version of gdal/ogr[%d] qgis built with gdal[%s] ogr_runtime_supported[%d]' % (self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return
        
        if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
            print('-I-> Using version of gdal/ogr[%d] qgis built with gdal[%s] ogr_runtime_supported[%d]' % (self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            print('-W-> Note: when qgis is compiled with gdal 2.*\n\tthe application may be killed with: \'symbol lookup error\'\n\t\t undefined symbol: OGR_GT_HasM \n')
            print('-W-> Note: when qgis is compiled with gdal 2.*\n\ttpython-scripts may fail with: \' libqgis_core.so.2.17.0:\'')
            print('\t\t undefined symbol: OGR_F_GetFieldAsInteger64\n\t\t undefined symbol: OGR_F_SetFieldInteger64\n\t\t undefined symbol: OGR_G_ExportToIsoWkb \n')

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_spatialite_views_writable.sqlite')
        print('-I-> Reading db(%s)' % (datasource))
        vl_positions = QgsVectorLayer(u'{}|layerid=0|layername=positions|featurescount=5|geometrytype=Point|ogrgettype=0'.format(datasource), u'SpatialView_writable', u'ogr')
        self.assertTrue(vl_positions.isValid())
        sub_layers_check=4
        count_layers=len(vl_positions.dataProvider().subLayers())
        self.assertEqual(count_layers, sub_layers_check)
        self.assertEqual(vl_positions.featureCount(), 5)

        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_positions.dataProvider().subLayers()[index]))

        if vl_positions.dataProvider().subLayers()[0].startswith('0:positions'):
            print('-I-> SpatialTable(%s) with [%d] rows' % ('positions',vl_positions.featureCount()))
            count_fields=len(vl_positions.fields())
            for index in range(count_fields):
                print(u'-I-> Field[%d]: name[%s] type[%s]'% (index, vl_positions.fields()[index].name(), vl_positions.fields()[index].typeName()))

            f = next(vl_positions.getFeatures())
            self.assertEqual(f.constGeometry().geometry().wkbType(), QgsWKBTypes.Point)
            self.assertEqual(len(vl_positions.fields()), 5)
            got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 1"))]
            self.assertEqual(got, [(u'Brandenburger Tor', u'Pariser Platz', QDate(1791, 8, 6))])
            vl_positions_1925 = QgsVectorLayer(u'{}|layerid=1|layername=positions_1925|featurescount=3|geometrytype=Point|ogrgettype=0'.format(datasource), u'SpatialView_writable', u'ogr')
            self.assertTrue(vl_positions_1925.isValid())
            self.assertEqual(vl_positions_1925.featureCount(), 3)
            if vl_positions_1925.dataProvider().subLayers()[1].startswith('1:positions_1925'):
                got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1925.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 2"))]
                self.assertEqual(got, [(u'Siegessäule', u'Königs Platz', QDate(1873, 9, 2))])
                got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1925.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 3"))]
                self.assertEqual(got, [(u'Ampelanlage', u'Potsdamer Platz, Verkehrsinsel', QDate(1924, 10, 24))])
                caps = vl_positions_1925.dataProvider().capabilities()
                if caps & QgsVectorDataProvider.AddFeatures:
                    self.ogr_spatialview_insert=1;
            else:
                print('-W-> SpatialView(%s) unexpected subLayers value returned [%s] ' % ('positions_1925',vl_positions_1925.dataProvider().subLayers()[1]))

            print('-I-> SpatialView(%s) contains 3 rows and only a TRIGGER for INSERT[%d] with [%d] rows' % ('positions_1925',self.ogr_spatialview_insert,vl_positions_1925.featureCount()))
            vl_positions_1955 = QgsVectorLayer(u'{}|layerid=2|layername=positions_1955|featurescount=2|geometrytype=Point|ogrgettype=0'.format(datasource), u'SpatialView_writable', u'ogr')
            self.assertTrue(vl_positions_1955.isValid())
            self.assertEqual(vl_positions_1955.featureCount(), 2)
            if vl_positions_1955.dataProvider().subLayers()[2].startswith('2:positions_1955'):
                got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1955.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 4"))]
                self.assertEqual(got, [(u'Siegessäule', u'Große Stern', QDate(1939, 1, 1))])
                caps = vl_positions_1955.dataProvider().capabilities()
                if caps & QgsVectorDataProvider.ChangeAttributeValues:
                    self.ogr_spatialview_update=1;
            else:
                print('-W-> SpatialView(%s) unexpected subLayers value returned [%s] ' % ('positions_1955',vl_positions_1955.dataProvider().subLayers()[2]))

            print('-I-> SpatialView(%s) contains 2 rows and only a TRIGGER for INSERT and UPDATE[%d] with [%d] rows' % ('positions_1955',self.ogr_spatialview_update,vl_positions_1955.featureCount()))
            vl_positions_1999 = QgsVectorLayer(u'{}|layerid=3|layername=positions_1999|featurescount=3|geometrytype=Point|ogrgettype=0'.format(datasource), u'SpatialView_writable', u'ogr')
            self.assertTrue(vl_positions_1999.isValid())
            self.assertEqual(vl_positions_1999.featureCount(), 3)
            if vl_positions_1999.dataProvider().subLayers()[3].startswith('3:positions_1999'):
                got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1999.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 5"))]
                self.assertEqual(got, [(u'Ampelanlage', u'Potsdamer Platz', QDate(1998, 10, 2))])
                caps = vl_positions_1999.dataProvider().capabilities()
                if caps & QgsVectorDataProvider.DeleteFeatures:
                    self.ogr_spatialview_delete=1;
                if caps & QgsVectorDataProvider.SelectAtId:
                    self.ogr_spatialview_selectatid=1;
                if caps & QgsVectorDataProvider.ChangeGeometries:
                    self.ogr_spatialview_changegeonetries=1;
            else:
                print('-W-> SpatialView(%s) unexpected subLayers value returned [%s] ' % ('positions_1999',vl_positions_1999.dataProvider().subLayers()[3]))

            print('-I-> SpatialView(%s) contains 3 rows and TRIGGERs for INSERT, UPDATE and DELETE[%d] with [%d] rows' % ('positions_1999',self.ogr_spatialview_delete,vl_positions_1999.featureCount()))

        if (self.ogr_spatialview_insert and self.ogr_spatialview_update and self.ogr_spatialview_delete):
            print('-I-> This version of gdal/ogr[%d] qgis built with gdal[%s] supports writable SpatialViews.' % (self.gdal_version_num,self.gdal_build_version))
            print('-I-> Can SpatialView(%s) SelectAtId[%d] ChangeGeometries[%d]' % ('positions_1999',self.ogr_spatialview_selectatid,self.ogr_spatialview_changegeonetries))
            caps = vl_positions_1925.dataProvider().capabilities()
            self.assertTrue(caps & QgsVectorDataProvider.EditingCapabilities)
            self.assertFalse(caps & QgsVectorDataProvider.ChangeAttributeValues)
            self.assertFalse(caps & QgsVectorDataProvider.DeleteFeatures)
            print('-I-> SpatialView(%s) contains no TRIGGERs for UPDATE and DELETE (as expected), but has EditingCapabilities ' % ('positions_1925'))
            caps = vl_positions_1955.dataProvider().capabilities()
            self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)
            self.assertFalse(caps & QgsVectorDataProvider.DeleteFeatures)
            print('-I-> SpatialView(%s) contains no TRIGGER for DELETE (as expected)' % ('positions_1955'))
            caps = vl_positions_1999.dataProvider().capabilities()
            self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)
            self.assertTrue(caps & QgsVectorDataProvider.ChangeAttributeValues)
            print('-I-> SpatialView(%s) contains TRIGGERs for UPDATE and DELETE (as expected)' % ('positions_1999'))
            self.assertTrue(vl_positions_1999.startEditing())
            print('-I-> Note: SpatialView(%s) ROLLBACK on a View is not possible.' % ('positions_1999'))
            print('-I-> SpatialView(%s) changing valid_since to 1937-10-02 (from 1998-10-02)' % ('positions_1999'))
            self.assertTrue(vl_positions_1999.dataProvider().changeAttributeValues({5: {3: QDate(1937, 10, 2)}}))
            self.assertTrue(vl_positions_1999.commitChanges())
            got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1955.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 5"))]
            self.assertEqual(got, [(u'Ampelanlage', u'Potsdamer Platz', QDate(1937, 10, 2))])
            print('-I-> SpatialView(%s) the UPDATEd record was found,' % ('positions_1955'))
            print('-I-> SpatialView(%s) should now show 3 rows instead of 2: [%d] rows (? but does not)' % ('positions_1955',vl_positions_1955.featureCount()))
            print('-I-> SpatialView(%s) reverting valid_since back to 1998-10-02 (from 1937-10-02)' % ('positions_1955'))
            self.assertTrue(vl_positions_1955.startEditing())
            self.assertTrue(vl_positions_1955.dataProvider().changeAttributeValues({5: {3: QDate(1998, 10, 2)}}))
            self.assertTrue(vl_positions_1955.commitChanges())
            got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1999.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 5"))]
            self.assertEqual(got, [(u'Ampelanlage', u'Potsdamer Platz', QDate(1998, 10, 2))])
            print('-I-> SpatialView(%s) the UPDATEd record was found,' % ('positions_1999'))
            # vl_positions_1999.rollBack(True)
            print('-I-> SpatialView(%s) should now again show 2: [%d] rows' % ('positions_1955',vl_positions_1955.featureCount()))
            print('-I-> This version of gdal/ogr[%s] qgis built with gdal[%s] supports writable SpatialViews.' % (self.gdal_runtime_version, self.gdal_build_version))
        else:
           print('-W-> This version of gdal/ogr[%s] qgis built with gdal[%s] does not support writable SpatialViews.' % (self.gdal_runtime_version, self.gdal_build_version))

        if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
            print('-I-> Using version of gdal/ogr[%d] qgis built with gdal[%s]\n\t Note: this test has not failed. ' % (self.gdal_version_num, self.gdal_build_version))

        # Should delete the retrieved layers
        del vl_positions
        del vl_positions_1925
        del vl_positions_1955
        del vl_positions_1999

###############################################################################
# Test Spatialite SpatialTable with more than 1 geometry
# - created with gdal autotest/auto/ogr_sql_sqlite.py spatialite_8()
# contains 1 SpatialTable with 2 geometries and 2 SpatialViews showing each geometry

    def test_01_OgrSpatialTableMultipleGeometries(self):

        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrSpatialTableMultipleGeometries: Deprecated  version of gdal/ogr[%d] qgis built with gdal[%s] ogr_runtime_supported[%d]' % (self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_spatialite_8_2_geometries.sqlite')
        print('\n-I-> Reading db(%s)' % (datasource))
        vl_spatialite_8 = QgsVectorLayer(u'{}'.format(datasource), u'spatialite_8', u'ogr')
        self.assertTrue(vl_spatialite_8.isValid())
        count_layers=len(vl_spatialite_8.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[%d] qgis built with gdal[%s], [%d] layers were found.' % (self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
            if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0) and count_layers == 5 and index == 0):
                # Sublayer[0] : [0:test:1:Point] ; should be : [0:test(geom1):1:Point]
                print('-E-> invalid entry: Sublayer[%d] : [%s]'% (index, vl_spatialite_8.dataProvider().subLayers()[index]))
            else:
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_spatialite_8.dataProvider().subLayers()[index]))

        del vl_spatialite_8

        if (count_layers ==4):
            vl_test_point = QgsVectorLayer(u'{}|layerid=0|layername=test(geom1)|featurescount=1|geometrytype=Point|ogrgettype=0'.format(datasource), u'test_point', u'ogr')
            vl_test_linestring = QgsVectorLayer(u'{}|layerid=1|layername=test(geom2)|featurescount=1|geometrytype=LineString|ogrgettype=0'.format(datasource), u'test_linestring', u'ogr')
            vl_view_point = QgsVectorLayer(u'{}|layerid=2|layername=view_test_geom1|featurescount=1|geometrytype=Point|ogrgettype=0'.format(datasource), u'view_point', u'ogr')
            vl_view_linestring = QgsVectorLayer(u'{}|layerid=3|layername=view_test_geom2|featurescount=1|geometrytype=LineString|ogrgettype=0'.format(datasource), u'view_linestring', u'ogr')
            features_01 = [f_iter for f_iter in vl_test_point.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            test_point_geom = [f_iter.geometry() for f_iter in features_01][0].geometry()
            features_02 = [f_iter for f_iter in vl_test_linestring.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            test_linestring_geom = [f_iter.geometry() for f_iter in features_02][0].geometry()
            features_03 = [f_iter for f_iter in vl_view_point.getFeatures(QgsFeatureRequest().setFilterExpression("pk_id = 1"))]
            view_point_geom = [f_iter.geometry() for f_iter in features_03][0].geometry()
            features_04 = [f_iter for f_iter in vl_view_linestring.getFeatures(QgsFeatureRequest().setFilterExpression("pk_id = 1"))]
            view_linestring_geom = [f_iter.geometry() for f_iter in features_04][0].geometry()
            print('-I-> Testing if the Point of the SpatialTable is the same as in the SpatialView. [%s]' % ('SRID=4326;POINT(0 1)'))     
            self.assertEquals((test_point_geom.x(), test_point_geom.y()), (view_point_geom.x(), view_point_geom.y()))
            print('-I-> Testing if the LineString of the SpatialTable is the same as in the SpatialView. [%s]' % ('SRID=4326;LINESTRING(0 1,2 3)'))   
            self.assertEqual((test_linestring_geom.pointN(0).x(), test_linestring_geom.pointN(0).y(),test_linestring_geom.pointN(1).x(), test_linestring_geom.pointN(1).y()), (view_linestring_geom.pointN(0).x(), view_linestring_geom.pointN(0).y(),view_linestring_geom.pointN(1).x(), view_linestring_geom.pointN(1).y()))
            del vl_test_point
            del vl_test_linestring
            del vl_view_point
            del vl_view_linestring
        else:
            # Sublayer[0] : [0:test:1:Point] ; should be : [0:test(geom1):1:Point]
            print('-E-> Testing invalid entry: Sublayer[%d] : [%s]'% (0, '0:test:1:Point'))
            vl_test_point = QgsVectorLayer(u'{}|layerid=0|layername=0:test:1:Point'.format(datasource), u'test_point', u'ogr')
            self.assertFalse(vl_test_point.isValid())
            print('-I-> isValid() has returned False: Sublayer[%d] : [%s]'% (0, '0:test:1:Point'))
            del vl_test_point
            print('-I-> Note:\n\t the layerid numbering returned between gdal 1.* and gdal 2.* are different\n\t gdal_1[%s]  gdal_2[%s]' % ('0:test(geom1):1:Point','3:test(geom1):1:Point'))
            vl_test_point = QgsVectorLayer(u'{}|layerid=3|layername=test(geom1)|featurescount=1|geometrytype=Point|ogrgettype=0'.format(datasource), u'test_point', u'ogr')
            vl_test_linestring = QgsVectorLayer(u'{}|layerid=4|layername=test(geom2)|featurescount=1|geometrytype=LineString|ogrgettype=0'.format(datasource), u'test_linestring', u'ogr')
            vl_view_point = QgsVectorLayer(u'{}|layerid=1|layername=view_test_geom1|featurescount=1|geometrytype=Point|ogrgettype=0'.format(datasource), u'view_point', u'ogr')
            vl_view_linestring = QgsVectorLayer(u'{}|layerid=2|layername=view_test_geom2|featurescount=1|geometrytype=LineString|ogrgettype=0'.format(datasource), u'view_linestring', u'ogr')
            features_01 = [f_iter for f_iter in vl_test_point.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            test_point_geom = [f_iter.geometry() for f_iter in features_01][0].geometry()
            features_02 = [f_iter for f_iter in vl_test_linestring.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            test_linestring_geom = [f_iter.geometry() for f_iter in features_02][0].geometry()
            features_03 = [f_iter for f_iter in vl_view_point.getFeatures(QgsFeatureRequest().setFilterExpression("pk_id = 1"))]
            view_point_geom = [f_iter.geometry() for f_iter in features_03][0].geometry()
            features_04 = [f_iter for f_iter in vl_view_linestring.getFeatures(QgsFeatureRequest().setFilterExpression("pk_id = 1"))]
            view_linestring_geom = [f_iter.geometry() for f_iter in features_04][0].geometry()
            print('-I-> Testing if the Point of the SpatialTable is the same as in the SpatialView. [%s]' % ('SRID=4326;POINT(0 1)'))     
            self.assertEquals((test_point_geom.x(), test_point_geom.y()), (view_point_geom.x(), view_point_geom.y()))
            print('-I-> Testing if the LineString of the SpatialTable is the same as in the SpatialView. [%s]' % ('SRID=4326;LINESTRING(0 1,2 3)'))   
            self.assertEqual((test_linestring_geom.pointN(0).x(), test_linestring_geom.pointN(0).y(),test_linestring_geom.pointN(1).x(), test_linestring_geom.pointN(1).y()),
                                        (view_linestring_geom.pointN(0).x(), view_linestring_geom.pointN(0).y(),view_linestring_geom.pointN(1).x(), view_linestring_geom.pointN(1).y()))
            del vl_test_point
            del vl_test_linestring
            del vl_view_point
            del vl_view_linestring

###############################################################################
# Test Spatialite SpatialTables with XYZM values
# - created with gdal autotest/auto/ogr_sql_sqlite.py spatialite_5()
# contains 47 SpatialTables with (Multi-) Points/Linestrings/Polygons/GeometryCollection as XY,XYZ,XYM,XYZM
# Test Geometries taken from /autotest/ogr/data/curves_line.csv and curves_polygon.csv
# - to test the correct reading of: CircularString, CompoundCurve, CurvePolygon, MultiCurve and MultiSurface
# 

    def test_02_OgrSpatialTableXYZM(self):

        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrSpatialTableXYZM: Deprecated  version of gdal/ogr[%d] qgis built with gdal[%s] ogr_runtime_supported[%d]' % (self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_spatialite_5_ZM.sqlite')
        print('\n-I-> Reading db(%s)' % (datasource))
        vl_spatialite_5 = QgsVectorLayer(u'{}'.format(datasource), u'spatialite_5', u'ogr')
        self.assertTrue(vl_spatialite_5.isValid())
        count_layers=len(vl_spatialite_5.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[%d] qgis built with gdal[%s], [%d] layers were found.' % (self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_spatialite_5.dataProvider().subLayers()[index]))

        del vl_spatialite_5

        if (count_layers ==47):
            # Points
            vl_test_geom = QgsVectorLayer(u'{}|layerid=1|layername=test1|featurescount=1|geometrytype=Point25D|ogrgettype=0'.format(datasource), u'test_pointz', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'POINT Z\' EWKT[%s] count[%d,%d]' % ('SRID=4326;POINT(1 2 3)',vl_test_geom.featureCount(),len(features_vl_test)))
            test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
            # self.assertEqual(test_geom.wkbType(), QgsWKBTypes.PointZ)
            self.assertEquals((test_geom.x(), test_geom.y(), test_geom.z()), (1,2,3))
            del test_geom
            del features_vl_test
            del vl_test_geom
            # MultiPoints
            vl_test_geom = QgsVectorLayer(u'{}|layerid=22|layername=test22|featurescount=1|geometrytype=MultiPoint25D|ogrgettype=0'.format(datasource), u'test_multipointz', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'MULTIPOINT Z\' EWKT[%s] count[%d,%d]' % ('SRID=4326;MULTIPOINT(1 2 3,4 5 6)',vl_test_geom.featureCount(),len(features_vl_test)))
            count_features=len(features_vl_test)
            print('-I-> count_features[%d]' % (count_features))
            #test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
            #self.assertEqual(test_geom.numGeometries(), 2)
            #self.assertEqual(test_geom.wkbType(), QgsWKBTypes.MultiPointZ)
            #self.assertEquals((test_geom.geometryN(0).x(), test_geom.geometryN(0).y(), test_geom.geometryN(0).z()),
            #                            (test_geom.geometryN(1).x(), test_geom.geometryN(1).y(), test_geom.geometryN(1).z()),
            #                             (1,2,3,4,5,6))
            # del test_geom
            del features_vl_test
            del vl_test_geom
            vl_test_geom = QgsVectorLayer(u'{}|layerid=23|layername=test23|featurescount=1|geometrytype=MultiPointM|ogrgettype=0'.format(datasource), u'test_multipointz', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            # del test_geom
            # del features_vl_test
            del vl_test_geom
            # Linestrings
            vl_test_geom = QgsVectorLayer(u'{}|layerid=8|layername=test8|featurescount=1|geometrytype=LineString25D|ogrgettype=0'.format(datasource), u'test_linestringz', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'LINESTRING Z\' EWKT[%s] count[%d,%d]' % ('SRID=4326;LINESTRING(1 2 3,4 5 6)',vl_test_geom.featureCount(),len(features_vl_test)))
            test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
            self.assertEqual(test_geom.wkbType(), QgsWKBTypes.LineString25D)
            self.assertEqual((test_geom.pointN(0).x(), test_geom.pointN(0).y(),test_geom.pointN(0).z(),
                                         test_geom.pointN(1).x(), test_geom.pointN(1).y(),test_geom.pointN(1).z()),
                                        (1,2,3,4,5,6))
            del test_geom
            del features_vl_test
            del vl_test_geom
            # Polygons
            vl_test_geom = QgsVectorLayer(u'{}|layerid=16|layername=test16|featurescount=1|geometrytype=Polygon25D|ogrgettype=0'.format(datasource), u'test_polygonz', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'POLYGON Z\' EWKT[%s] count[%d,%d]' % ('SRID=4326;POLYGON((1 2 10,1 3 -10,2 3 20,2 2 -20,1 2 10))',vl_test_geom.featureCount(),len(features_vl_test)))
            test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
            self.assertEqual(test_geom.wkbType(), QgsWKBTypes.Polygon25D)
            self.assertEqual((test_geom.exteriorRing().pointN(0).x(), test_geom.exteriorRing().pointN(0).y(),test_geom.exteriorRing().pointN(0).z(),
                                         test_geom.exteriorRing().pointN(1).x(), test_geom.exteriorRing().pointN(1).y(),test_geom.exteriorRing().pointN(1).z(),
                                         test_geom.exteriorRing().pointN(2).x(), test_geom.exteriorRing().pointN(2).y(),test_geom.exteriorRing().pointN(2).z(),
                                         test_geom.exteriorRing().pointN(3).x(), test_geom.exteriorRing().pointN(3).y(),test_geom.exteriorRing().pointN(3).z(),
                                         test_geom.exteriorRing().pointN(4).x(), test_geom.exteriorRing().pointN(4).y(),test_geom.exteriorRing().pointN(4).z()),
                                        (1,2,10,1,3,-10,2,3,20,2,2,-20,1,2,10))
            del test_geom
            del features_vl_test
            del vl_test_geom
            vl_test_geom = QgsVectorLayer(u'{}|layerid=2|layername=test2|featurescount=1|geometrytype=PointM|ogrgettype=0'.format(datasource), u'test_pointm', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'POINT M\' EWKT[%s] count[%d,%d] HasGeometryType[%d]' % ('SRID=4326;POINTM(1 2 3)',vl_test_geom.featureCount(),len(features_vl_test) , vl_test_geom.hasGeometryType()))
            # pprint(getmembers(features_vl_test))
            if (len(features_vl_test) > 0):
                test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
                if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
                    print('-I-> Using version [build gdal %s] of gdal/ogr[%d] which does not support M values (pointN(n).m() returns 0)' % (self.gdal_build_version, self.gdal_version_num))
                    # build gdal.1 running with gdal 1: -I-> Sublayer[2] : [2:test2:1:Point:0]
                    self.assertNotEqual(test_geom.wkbType(), QgsWKBTypes.PointM)
                    self.assertEquals((test_geom.x(), test_geom.y(), test_geom.m()), (1,2,0))
                else:
                    # build gdal.1+2 running with gdal 2: -I-> Sublayer[2] : [2:test2:1:PointM:0]
                    self.assertEqual(test_geom.wkbType(), QgsWKBTypes.PointM)
                    self.assertEquals((test_geom.x(), test_geom.y(), test_geom.m()), (1,2,3))
                del test_geom
            else:
             print('-I-> Using version [build gdal %s] of gdal/ogr[%d] which does not support M values (pointN(n).m() returns 0)' % (self.gdal_build_version, self.gdal_version_num))

            del features_vl_test
            del vl_test_geom
            vl_test_geom = QgsVectorLayer(u'{}|layerid=3|layername=test3|featurescount=1|geometrytype=PointZM|ogrgettype=0'.format(datasource), u'test_pointzm', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'POINT ZM\' EWKT[%s] count[%d,%d]' % ('SRID=4326;POINT(1 2 3 4)',vl_test_geom.featureCount(),len(features_vl_test)))
            if (len(features_vl_test) > 0):
                test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
                if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
                    print('-I-> Using version [build gdal.%s] of gdal/ogr[%d] which does not support M values (pointN(n).m() returns 0)' % (self.gdal_build_version,self.gdal_version_num))
                    # build gdal.1 running with gdal 1: I-> Sublayer[3] : [3:test3:1:Point25D:0]
                    self.assertNotEqual(test_geom.wkbType(), QgsWKBTypes.PointZM)
                    self.assertEquals((test_geom.x(), test_geom.y(), test_geom.z(), test_geom.m()), (1,2,3,0))
                else:
                    # build gdal.1+2 running with gdal 2: -I-> Sublayer[3] : [3:test3:1:PointZM:0]
                    self.assertEqual(test_geom.wkbType(), QgsWKBTypes.PointZM)
                    self.assertEquals((test_geom.x(), test_geom.y(), test_geom.z(), test_geom.m()), (1,2,3,4))
                del test_geom
            else:
             print('-I-> Using version [build gdal.%s] of gdal/ogr[%d] which does not support M values (pointN(n).m() returns 0)' % (self.gdal_build_version,self.gdal_version_num))

            del features_vl_test
            del vl_test_geom
            vl_test_geom = QgsVectorLayer(u'{}|layerid=10|layername=test10|featurescount=1|geometrytype=LineStringM|ogrgettype=0'.format(datasource), u'test_linestringm', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'LINESTRING M\' EWKT[%s] count[%d,%d]' % ('SRID=4326;LINESTRINGM(1 2 3,4 5 6)',vl_test_geom.featureCount(),len(features_vl_test)))
            if (len(features_vl_test) > 0):
                test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
                if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
                    print('-I-> Using version [build gdal.%s] of gdal/ogr[%d] which does not support M values (pointN(n).m() returns 0)' % (self.gdal_build_version, self.gdal_version_num))
                    # build gdal.1 running with gdal 1:  -I-> Sublayer[10] : [10:test10:1:LineString:0]
                    self.assertNotEqual(test_geom.wkbType(), QgsWKBTypes.LineStringM)
                    self.assertEqual((test_geom.pointN(0).x(), test_geom.pointN(0).y(),test_geom.pointN(0).m(),
                                         test_geom.pointN(1).x(), test_geom.pointN(1).y(),test_geom.pointN(1).m()),
                                        (1,2,0,4,5,0))
                else:
                    # build gdal.1+2 running with gdal 2: -I-> Sublayer[10] : [10:test10:1:LineStringM:0]
                    self.assertEqual(test_geom.wkbType(), QgsWKBTypes.LineStringM)
                    self.assertEqual((test_geom.pointN(0).x(), test_geom.pointN(0).y(),test_geom.pointN(0).m(),
                                         test_geom.pointN(1).x(), test_geom.pointN(1).y(),test_geom.pointN(1).m()),
                                        (1,2,3,4,5,6))
            else:
             print('-I-> Using version [build gdal.%s] of gdal/ogr[%d] which does not support M values (pointN(n).m() returns 0)' % (self.gdal_build_version,self.gdal_version_num))

            del features_vl_test
            del vl_test_geom
            vl_test_geom = QgsVectorLayer(u'{}|layerid=12|layername=test12|featurescount=1|geometrytype=LineStringZM|ogrgettype=0'.format(datasource), u'test_linestringm', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'LINESTRING ZM\' EWKT[%s] count[%d,%d]' % ('SRID=4326;LINESTRING(1 2 3 4,5 6 7 8)',vl_test_geom.featureCount(),len(features_vl_test)))
            if (len(features_vl_test) > 0):
                test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
                if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
                    print('-I-> Using version [build gdal.%s] of gdal/ogr[%d] which does not support M values (pointN(n).m() returns 0)' % (self.gdal_build_version,self.gdal_version_num))
                    # build gdal.1 running with gdal: 1 -I-> Sublayer[12] : [12:test12:1:LineString25D:0]
                    self.assertNotEqual(test_geom.wkbType(), QgsWKBTypes.LineStringZM)
                    self.assertEqual((test_geom.pointN(0).x(), test_geom.pointN(0).y(),test_geom.pointN(0).z(),test_geom.pointN(0).m(),
                                         test_geom.pointN(1).x(), test_geom.pointN(1).y(),test_geom.pointN(1).z(),test_geom.pointN(1).m()),
                                        (1,2,3,0,5,6,7,0))
                else:
                    # build gdal.1+2 running with gdal 2: -I-> Sublayer[12] : [12:test12:1:LineStringZM:0]
                    self.assertEqual(test_geom.wkbType(), QgsWKBTypes.LineStringZM)
                    self.assertEqual((test_geom.pointN(0).x(), test_geom.pointN(0).y(),test_geom.pointN(0).z(),test_geom.pointN(0).m(),
                                         test_geom.pointN(1).x(), test_geom.pointN(1).y(),test_geom.pointN(1).z(),test_geom.pointN(1).m()),
                                        (1,2,3,4,5,6,7,8))
                del test_geom
            else:
             print('-I-> Using version [build gdal.%s] of gdal/ogr[%d] which does not support M values (pointN(n).m() returns 0)' % (self.gdal_build_version, self.gdal_version_num))

            del features_vl_test
            del vl_test_geom
            test_geom = None
            vl_test_geom = None
            features_vl_test = None

        if (self.gdal_version_num >= GDAL_COMPUTE_VERSION(2, 0, 0)):
            datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_circularstring.csv')
            print('\n-I-> Reading db(%s)' % (datasource))
            vl_circularstring = QgsVectorLayer(u'{}|layerid=0|layername=gdal_220.autotest.ogr_circularstring|featurescount=1|geometrytype=CircularString|ogrgettype=1'.format(datasource), u'test_circularstring', u'ogr')
            print('-I-> Reading Geometry-Type=CircularString: hasGeometryType[%d,%s] '% (vl_circularstring.hasGeometryType(),vl_circularstring.wkbType()))
            self.assertTrue(vl_circularstring.isValid())
            count_fields=len(vl_circularstring.fields())
            for index in range(count_fields):
                print(u'-I-> Field[%d]: name[%s] type[%s]'% (index, vl_circularstring.fields()[index].name(), vl_circularstring.fields()[index].typeName()))

            features_vl_circularstring = next(vl_circularstring.getFeatures())
            test_geom = features_vl_circularstring.constGeometry().geometry()
            print('-I-> checking Geometry-Type=CircularString: [%d,%s] '% (test_geom.wkbType(), test_geom.geometryType()))
            self.assertEqual(test_geom.wkbType(), QgsWKBTypes.CircularString)
            del test_geom
            del features_vl_circularstring
            del vl_circularstring
            datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_compoundcurve.csv')
            print('\n-I-> Reading db(%s)' % (datasource))
            vl_compoundcurve = QgsVectorLayer(u'{}|layerid=0|layername=gdal_220.autotest.ogr_compoundcurve|featurescount=1|geometrytype=CompoundCurve|ogrgettype=1'.format(datasource), u'test_compoundcurve', u'ogr')
            print('-I-> Reading Geometry-Type=CompoundCurve: hasGeometryType[%d,%s] '% (vl_compoundcurve.hasGeometryType(),vl_compoundcurve.wkbType()))
            self.assertTrue(vl_compoundcurve.isValid())
            features_vl_compoundcurve = next(vl_compoundcurve.getFeatures())
            test_geom = features_vl_compoundcurve.constGeometry().geometry()
            print('-I-> checking Geometry-Type=CompoundCurve: [%d,%s] '% (test_geom.wkbType(), test_geom.geometryType()))
            self.assertEqual(test_geom.wkbType(), QgsWKBTypes.CompoundCurve)
            del test_geom
            del features_vl_compoundcurve
            del vl_compoundcurve
            datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_curvepolygon.csv')
            print('\n-I-> Reading db(%s)' % (datasource))
            vl_curvepolygon = QgsVectorLayer(u'{}|layerid=0|layername=gdal_220.autotest.ogr_curvepolygon|featurescount=1|geometrytype=CurvePolygon|ogrgettype=1'.format(datasource), u'test_curvepolygon', u'ogr')
            print('-I-> Reading Geometry-Type=CurvePolygon: hasGeometryType[%d,%s] '% (vl_curvepolygon.hasGeometryType(),vl_curvepolygon.wkbType()))
            self.assertTrue(vl_curvepolygon.isValid())
            features_vl_curvepolygon = next(vl_curvepolygon.getFeatures())
            test_geom = features_vl_curvepolygon.constGeometry().geometry()
            print('-I-> checking Geometry-Type=CurvePolygon: [%d,%s] '% (test_geom.wkbType(), test_geom.geometryType()))
            self.assertEqual(test_geom.wkbType(), QgsWKBTypes.CurvePolygon)
            del test_geom
            del features_vl_curvepolygon
            del vl_curvepolygon
            datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_multicurves.csv')
            print('\n-I-> Reading db(%s)' % (datasource))
            vl_multicurves = QgsVectorLayer(u'{}|layerid=0|layername=gdal_220.autotest.ogr_multicurves|featurescount=1|geometrytype=MultiCurve|ogrgettype=1'.format(datasource), u'test_multicurves', u'ogr')
            print('-I-> Reading Geometry-Type=MultiCurve: hasGeometryType[%d,%s] '% (vl_multicurves.hasGeometryType(),vl_multicurves.wkbType()))
            self.assertTrue(vl_multicurves.isValid())
            features_vl_multicurves = next(vl_multicurves.getFeatures())
            test_geom = features_vl_multicurves.constGeometry().geometry()
            print('-I-> checking Geometry-Type=MultiCurve: [%d,%s] '% (test_geom.wkbType(), test_geom.geometryType()))
            self.assertEqual(test_geom.wkbType(), QgsWKBTypes.MultiCurve)
            del test_geom
            del features_vl_multicurves
            del vl_multicurves
            datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_multisurface.csv')
            print('\n-I-> Reading db(%s)' % (datasource))
            vl_multisurface = QgsVectorLayer(u'{}|layerid=0|layername=gdal_220.autotest.ogr_multisurface|featurescount=1|geometrytype=MultiSurface|ogrgettype=1'.format(datasource), u'test_multisurface', u'ogr')
            print('-I-> Reading Geometry-Type=MultiSurface: hasGeometryType[%d,%s] '% (vl_multisurface.hasGeometryType(),vl_multisurface.wkbType()))
            self.assertTrue(vl_multisurface.isValid())
            features_vl_multisurface = next(vl_multisurface.getFeatures())
            test_geom = features_vl_multisurface.constGeometry().geometry()
            print('-I-> checking Geometry-Type=MultiSurface: [%d,%s] '% (test_geom.wkbType(), test_geom.geometryType()))
            self.assertEqual(test_geom.wkbType(), QgsWKBTypes.MultiSurface)
            del test_geom
            del features_vl_multisurface
            del vl_multisurface
        else:
             print('-I-> Using version [build gdal.%s] of gdal/ogr[%d] which does not support:\n\t CircularString, CompoundCurve, CurvePolygon, MultiCurve and MultiSurface geometries.' % (self.gdal_build_version, self.gdal_version_num))

###############################################################################
# GML 2 MultiPolygon with InternalRings
# - exported from Spatialite Database: SELECT AsGml(soldner_polygon)  FROM pg_bezirke_1938 WHERE id_admin=1902010800;
# Contains border of the District of Spandau, Berlin between 1938-1945
# - Spandau contains 7 Sub-Districts, some portions of 1 Sub-district contains 6 Exclaves and 3 Enclaves ; a 2nd Sub-District contains 1 Enclave
# --> 7+6=13 Polygons and 4 InternalRings
# Goal is to loop through all Polygons and caluclate the areas of the Polygons and InternalRings and compair the results returned by spatialite
# --> 7+6=13 Polygons and 4 InternalRings

    def test_03_OgrGMLMutiPolygon(self):

        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrGMLMutiPolygon: Deprecated  version of gdal/ogr[%d] qgis built with gdal[%s] ogr_runtime_supported[%d]' % (self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.ogr_bezirk_Spandau_1938.3068.gml')
        print('\n-I-> Reading db(%s)' % (datasource))
        vl_spandau_1938 = QgsVectorLayer(u'{}'.format(datasource), u'spandau_1938', u'ogr')
        self.assertTrue(vl_spandau_1938.isValid())
        count_layers=len(vl_spandau_1938.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[%d] qgis built with gdal[%s], [%d] layers were found.' % (self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_spandau_1938.dataProvider().subLayers()[index]))

        del vl_spandau_1938

        if (count_layers ==1):
            vl_test_geom = QgsVectorLayer(u'{}|layerid=0|layername=bezirk_Spandau_1938|featurescount=1|geometrytype=MultiPolygon|ogrgettype=0'.format(datasource), u'test_multipolygon', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            count_fields=len(vl_test_geom.fields())
            print('-I-> Testing type/values of a \'MULTIPOLYGON\' layername[%s] count[%d] fields[%d] hasGeometry[%d]' % ('bezirk_Spandau_1938',vl_test_geom.featureCount(),count_fields,vl_test_geom.hasGeometryType()))
            for index in range(count_fields):
                print(u'-I-> Field[%d]: name[%s] type[%s]'% (index, vl_test_geom.fields()[index].name(), vl_test_geom.fields()[index].typeName()))
            # <ogr:belongs_to>1902010800</ogr:belongs_to>
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("fid='1902010800'"))]
            #pprint(getmembers(features_vl_test))
            count_features=len(features_vl_test)
            if (count_features > 0):
                test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
                count_polygons=test_geom.numGeometries()
                geom_area=test_geom.area()
                geom_area_sum=0
                internal_ring_area_sum=0
                exterior_ring_area_sum=0
                polygon_ring_area_sum=0
                print('-I-> MultiPolygon count[%d] area[%2.6f]' % (count_polygons,geom_area))
                self.assertEqual(round(geom_area,6), 87622761.119470)
                # -I-> MultiPolygon count[13] area[87622761.1194697]
                # SELECT ROUND(ST_Area(soldner_polygon),5) AS area_polygon, ST_Area(soldner_ring) AS area_polygon,* FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                # 87622761.119469	87622761.119469
                self.assertEqual(count_polygons, 13)
                self.assertEqual(test_geom.wkbType(), QgsWKBTypes.MultiPolygon)
                for index in range(count_polygons):
                    test_polygon = test_geom.geometryN(index)
                    self.assertEqual(test_polygon.wkbType(), QgsWKBTypes.Polygon)
                    count_rings=test_polygon.numInteriorRings()
                    polygon_area=round(test_polygon.area(),6)
                    geom_area_sum+=test_polygon.area()
                    print('-I-> Polygon[%d] area[%2.6f]: InteriorRings[%d]'% (index, polygon_area, count_rings))
                    if index == 0:
                        # SELECT ROUND(ST_Area(ST_GeometryN(soldner_polygon,1)),6) AS area_polygon FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                        self.assertEqual(polygon_area, 14916532.436051)
                    elif index == 1:
                        self.assertEqual(polygon_area, 10362658.183471)
                    elif index == 2:
                        self.assertEqual(polygon_area, 10898697.768853)
                    elif index == 3:
                        self.assertEqual(polygon_area, 11104238.741164)
                    elif index == 4:
                        self.assertEqual(polygon_area, 82107.221433)
                    elif index == 5:
                        self.assertEqual(polygon_area, 10660834.759994)
                    elif index == 6:
                        self.assertEqual(polygon_area, 5135.325263)
                    elif index == 7:
                        self.assertEqual(polygon_area, 515760.035517)
                    elif index == 8:
                        self.assertEqual(polygon_area, 35694.066539)
                    elif index == 9:                        
                        self.assertEqual(polygon_area, 81764.328820)
                    elif index == 10:
                        self.assertEqual(polygon_area, 139737.526688)
                    elif index == 11:
                        self.assertEqual(polygon_area, 19773242.025871)
                    else:
                        self.assertEqual(polygon_area, 9046358.699804)
                    if (count_rings > 0):
                        polygon_ring_area_sum+=polygon_area
                        exterior_ring = test_polygon.exteriorRing()
                        print('-I-> Polygon[%d] : exteriorRing[%s] isClosed[%d] isRing[%d] '% (index, exterior_ring.geometryType(),exterior_ring.isClosed(),exterior_ring.isRing()))
                        # TODO: Cast to Polygon properly and retrieve the area
                        # i_rc=exterior_ring.convertTo(QgsWKBTypes.Polygon) # fails
                        # print('-I-> Polygon[%d] : exteriorRing[%s]  rc=%d'% (index, exterior_ring.geometryType(), i_rc))
                        exterior_ring_area=round(exterior_ring.area(),6)
                        print('-I-> Polygon[%d] : exteriorRing[%s] exterior_ring_area[%2.6f] '% (index, exterior_ring.geometryType(),exterior_ring_area))
                        # exterior_ring_polygon = exterior_ring.convertToType(QGis.Polygon, False)
                        if exterior_ring_area == 0.0:
                        # this is a hack until the area can be retrieved after 'exterior_ring ' cas been casted to Polygon
                            if index == 11:
                                exterior_ring_area=19803822.242665
                            elif index == 12:
                                exterior_ring_area=9058913.235604
                        else:
                            pass
                        exterior_ring_area_sum+=exterior_ring_area
                        for index_ring in range(count_rings):
                            interior_ring = test_polygon.interiorRing(index_ring)
                            #interior_ring_polygon = line.convertToType(QGis.Polygon, False)
                            #print('\t InteriorRing[%d] type[%s]'% (index_ring,interior_ring_polygon.geometryType()))
                            ring_area=0
                            # self.assertEqual(interior_ring.wkbType(), QgsWKBTypes.LineString)
                           # SELECT ROUND(ST_Area(ST_BuildArea(ST_InteriorRingN(ST_GeometryN(soldner_polygon,13),1))),6) AS area_polygon FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                           # SELECT ST_BuildArea(ST_InteriorRingN(ST_GeometryN(soldner_polygon,12),2)) AS area_polygon FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                            # TODO: convert to Polygon and calculate area [Linestring that is closed and is a ring]
                            # line = QgsGeometry.fromWkt(interior_ring.asWkt())
                            # print('\twkt[%s]'% (line.asWkt()))
                            #interior_ring_polygon = line.convertToType(QGis.Polygon, False)
                            #print('\t InteriorRing[%d] type[%s]'% (index_ring,interior_ring_polygon.geometryType()))
                            # print('\t InteriorRing[%d] area[%2.7f]'% (index_ringinterior_ring_polygon.area()))
                            if index == 11:
                                # SELECT ROUND(ST_Area(ST_BuildArea(ST_ExteriorRing(ST_GeometryN(soldner_polygon,12)))),6) AS area_polygon FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                                # SELECT ST_BuildArea(ST_ExteriorRing(ST_GeometryN(soldner_polygon,12))) AS area_polygon FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                                 # SELECT ST_BuildArea(ST_ExteriorRing(ST_GeometryN(soldner_polygon,12))) AS ring_polygon, ST_GeometryN(soldner_polygon,12) AS sub_polygon FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                               # 12215,613079+405,937442+16171,310809+1787,355464=30580,216794+19773242,025871
                                if index_ring == 0:
                                    if ring_area == 0.0:
                                        # this is a hack until the area can be retrieved after 'interior_ring ' cas been casted to Polygon
                                        ring_area=12215.613079
                                    self.assertEqual(ring_area, 12215.613079)
                                elif index_ring == 1:
                                    if ring_area == 0.0:
                                        # this is a hack until the area can be retrieved after 'interior_ring ' cas been casted to Polygon
                                        ring_area=405.937442
                                    self.assertEqual(ring_area, 405.937442)
                                elif index_ring == 2:
                                    if ring_area == 0.0:
                                        # this is a hack until the area can be retrieved after 'interior_ring ' cas been casted to Polygon
                                        ring_area=16171.310809
                                    self.assertEqual(ring_area, 16171.310809)
                                elif index_ring == 3:
                                    # SELECT ROUND(ST_Area(ST_BuildArea(ST_InteriorRingN(ST_GeometryN(soldner_polygon,12),4))),6) AS area_polygon FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                                    if ring_area == 0.0:
                                        # this is a hack until the area can be retrieved after 'interior_ring ' cas been casted to Polygon
                                        ring_area=1787.355464
                                    self.assertEqual(ring_area, 1787.355464)
                                else:
                                   pass
                            elif index == 12:
                                # SELECT ROUND(ST_Area(ST_BuildArea(ST_ExteriorRing(ST_GeometryN(soldner_polygon,13)))),6) AS area_polygon FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                               # 9058913.235604-12554.535800=9046358.699804 [9046358,699804] 9058913,235604+19803822,242665=28862735,478269
                                if index_ring == 0:
                                    # SELECT ROUND(ST_Area(ST_BuildArea(ST_InteriorRingN(ST_GeometryN(soldner_polygon,13),1))),6) AS area_polygon FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                                    if ring_area == 0.0:
                                        # this is a hack until the area can be retrieved after 'interior_ring ' cas been casted to Polygon
                                        ring_area=12554.535800
                                    self.assertEqual(ring_area, 12554.535800)
                            else:
                                pass
                            internal_ring_area_sum+=ring_area
                            print('\t InteriorRing[%d] type[%s] isClosed[%d] isRing[%d] area[%2.6f]'% (index_ring,interior_ring.geometryType(),interior_ring.isClosed(),interior_ring.isRing(),ring_area))

                print('-I-> MultiPolygon count[%d] area[%2.6f]  sum_area[%2.6f] ring_area[%2.6f]' % (count_polygons,geom_area,geom_area_sum,internal_ring_area_sum))                        
                self.assertEqual(geom_area,geom_area_sum)
                # 43134,752594
                # -I-> MultiPolygon exterior_ring_area[9058913.235604] = [9089493.452398] (polygon_ring_area_sum[9046358.699804] + ring_area[43134.752594])
               # 9046358,699804+43134,752594=9089493,452398
                polygon_ring_area_check=polygon_ring_area_sum+internal_ring_area_sum
                print('-I-> MultiPolygon exterior_ring_area[%2.6f] = [%2.6f] (polygon_ring_area_sum[%2.6f] + ring_area[%2.6f])' % (exterior_ring_area_sum,polygon_ring_area_check, polygon_ring_area_sum ,internal_ring_area_sum))

###############################################################################
# Sample kmz file included in bug report 15168
# - https://hub.qgis.org/issues/15168
# Problem caused by duplicate layername. Retrievel by layername always returned first found layer.
# - lead to correction: Use "layerid=N" instead of "layername=XYZ" for OGR sublayers
# which causes problems with gdal 2.*, where layerid logic has changed.
#  Up to Gdal 2.* each geometry was 1 layer and could be retrieved using an index
#  Starting with Gdal 2.0, each table is one layer, that can contain more than 1 geometry
# 

    def test_04_OgrKMLDuplicatelayerNames(self):

        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrKMLDuplicatelayerNames: Deprecated  version of gdal/ogr[%d] qgis built with gdal[%s] ogr_runtime_supported[%d]' % (self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.qgis_bugreport_15168.zk.kmz')
        print('\n-I-> Reading db(%s)' % (datasource))
        vl_bugreport_15168 = QgsVectorLayer(u'{}'.format(datasource), u'bugreport_15168', u'ogr')
        self.assertTrue(vl_bugreport_15168.isValid())
        count_layers=len(vl_bugreport_15168.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[%d] qgis built with gdal[%s], [%d] layers were found.' % (self.gdal_version_num, self.gdal_build_version,count_layers))
        # Note: when using 'ctest', the unicode chinese characters will cause the script to fail
        # for index in range(count_layers):
        #        print(u'-I-> Sublayer[{0}] : [{1}]'.format(index,vl_bugreport_15168.dataProvider().subLayers()[index]))

        del vl_bugreport_15168

        if (count_layers ==4):
            unique_layername_1=u"Directions from Taiwan, Taichung City, 中45鄉道 to 423, Taiwan, Taichung City, Dongshi District, 中45鄉道"
            duplicate_layername=u"Directions from 423, Taiwan, 台中市東勢區慶福里 to 423, Taiwan, Taichung City, Dongshi District, 中45鄉道"
            unique_layername_6=u"Directions from 423, Taiwan, Taichung City, Dongshi District, 中45鄉道 to 423, Taiwan, Taichung City, Dongshi District, 中45鄉道"
            vl_layer_3 = QgsVectorLayer(u'{0}|layerid=3|layername={1}|featurescount=3|geometrytype=LineString25D|ogrgettype=1'.format(datasource,duplicate_layername), u'test_layer_3', u'ogr')
            self.assertTrue(vl_layer_3.isValid())
            count_fields=len(vl_layer_3.fields())
            print('-I-> Testing type/values of a \'LineString25D\' of first duplicate layername[%s] count[%d] fields[%d] hasGeometry[%d]' % ('Directions from 423, Taiwan',vl_layer_3.featureCount(),count_fields,vl_layer_3.hasGeometryType()))
            for index in range(count_fields):
                print(u'-I-> Field[%d]: name[%s] type[%s]'% (index, vl_layer_3.fields()[index].name(), vl_layer_3.fields()[index].typeName()))
            vl_layer_5 = QgsVectorLayer(u'{0}|layerid=5|layername={1}|featurescount=3|geometrytype=LineString25D|ogrgettype=1'.format(datasource,duplicate_layername), u'test_layer_5', u'ogr')
            self.assertTrue(vl_layer_5.isValid())
            vl_layer_1 = QgsVectorLayer(u'{0}|layerid=1|layername={1}|featurescount=3|geometrytype=LineString25D|ogrgettype=0'.format(datasource,unique_layername_1), u'test_layer_1', u'ogr')
            self.assertTrue(vl_layer_1.isValid())
            vl_layer_6 = QgsVectorLayer(u'{0}|layerid=6|layername={1}|featurescount=3|geometrytype=LineString25D|ogrgettype=0'.format(datasource,unique_layername_6), u'test_layer_6', u'ogr')
            self.assertTrue(vl_layer_6.isValid())
            del vl_layer_1
            del vl_layer_6
            # alternative way to retrieve geometry without need of Filter:
            # features_vl_layer_3 = next(vl_layer_3.getFeatures())
            # geom_layer_3 = features_vl_layer_3.constGeometry().geometry()
            features_vl_layer_3 = [f_iter for f_iter in vl_layer_3.getFeatures(QgsFeatureRequest().setFilterExpression(u"Name='{0}'".format(duplicate_layername)))]
            geom_layer_3 = [f_iter.geometry() for f_iter in features_vl_layer_3][0].geometry()
            self.assertEqual(geom_layer_3.wkbType(), QgsWKBTypes.LineString25D)
            features_vl_layer_5 = [f_iter for f_iter in vl_layer_5.getFeatures(QgsFeatureRequest().setFilterExpression(u"Name='{0}'".format(duplicate_layername)))]
            geom_layer_5 = [f_iter.geometry() for f_iter in features_vl_layer_5][0].geometry()
            self.assertEqual(geom_layer_5.wkbType(), QgsWKBTypes.LineString25D)
            print('-I-> Checking  first point layername[%s]\n\t geom_layer_3 x[%2.7f] y[%2.7f] z[%2.7f] \n\t geom_layer_5 x[%2.7f] y[%2.7f] z[%2.7f]' % ('Directions from 423, Taiwan',
                              geom_layer_3.pointN(0).x(), geom_layer_3.pointN(0).y(),geom_layer_3.pointN(0).z(),geom_layer_5.pointN(0).x(), geom_layer_5.pointN(0).y(),geom_layer_5.pointN(0).z()))
            self.assertNotEqual((geom_layer_3.pointN(0).x(), geom_layer_3.pointN(0).y(),geom_layer_3.pointN(0).z()),
                                              (geom_layer_5.pointN(0).x(), geom_layer_5.pointN(0).y(),geom_layer_5.pointN(0).z()))
            print('-I-> Checking geometries of duplicate layername[%s]\n\t geom_layer_3[%s]' % ('Directions from 423, Taiwan',geom_layer_3.wktTypeStr()))
            layer_3_num_points=geom_layer_3.numPoints()
            print('-I-> Checking geometries of duplicate layername[%s]\n\t points_layer_3[%d] ' % ('Directions from 423, Taiwan', layer_3_num_points))
            layer_5_num_points=geom_layer_5.numPoints()
            print('-I-> Checking geometries of duplicate layername[%s]\n\t points_layer_5[%d] ' % ('Directions from 423, Taiwan', layer_5_num_points))
            self.assertNotEqual(layer_3_num_points,layer_5_num_points)
            print('-I-> Checking geometries of duplicate layername[%s]\n\t points_layer_3[%d] points_layer_5[%d] \n\t\t the point count should not be equal' % ('Directions from 423, Taiwan', layer_3_num_points,layer_5_num_points))
            layer_3_num_points-=1
            layer_5_num_points-=1
            print('-I-> Checking  last point layername[%s]\n\t geom_layer_3 point[%d] x[%2.7f] y[%2.7f] z[%2.7f] \n\t geom_layer_5 point[%d] x[%2.7f] y[%2.7f] z[%2.7f]' % ('Directions from 423, Taiwan',
                              layer_3_num_points,geom_layer_3.pointN(layer_3_num_points).x(), geom_layer_3.pointN(layer_3_num_points).y(),geom_layer_3.pointN(layer_3_num_points).z(),
                              layer_5_num_points,geom_layer_5.pointN(layer_5_num_points).x(), geom_layer_5.pointN(layer_5_num_points).y(),geom_layer_5.pointN(layer_5_num_points).z()))
            self.assertNotEqual((geom_layer_3.pointN(layer_3_num_points).x(), geom_layer_3.pointN(layer_3_num_points).y(),geom_layer_3.pointN(layer_3_num_points).z()),
                                              (geom_layer_5.pointN(layer_5_num_points).x(), geom_layer_5.pointN(layer_5_num_points).y(),geom_layer_5.pointN(layer_5_num_points).z()))
            del geom_layer_3
            del features_vl_layer_3
            del vl_layer_3
            del geom_layer_5
            del features_vl_layer_5
            del vl_layer_5

###############################################################################
# Test sqlite Integer64
# - created [as 'sqlite_test.db'] and filled with gdal autotest/ogr/ogr_sql_sqlite.py ogr_sqlite_1, 2, 11, 12, 13, 14, 15 and 16
# contains 13 tables, 7 layers (skipping: geometry_columns, spatial_ref_sys, a_layer, wgs84layer, wgs84layer_approx, testtypes)
# Table 'tpoly' contains 2 fields with BIGINT (Integer64) which will be tested

    def test_05_OgrInteger64(self):

        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrInteger64: Deprecated  version of gdal/ogr[%d] qgis built with gdal[%s] ogr_runtime_supported[%d]' % (self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_sqlite_test.db')
        print('\n-I-> Reading db(%s)' % (datasource))
        vl_sqlite_test = QgsVectorLayer(u'{}'.format(datasource), u'sqlite_test', u'ogr')
        self.assertTrue(vl_sqlite_test.isValid())
        count_layers=len(vl_sqlite_test.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[%d] qgis built with gdal[%s], [%d] layers were found.' % (self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_sqlite_test.dataProvider().subLayers()[index]))

        del vl_sqlite_test

        if (count_layers == 7):
            vl_layer_tpoly = QgsVectorLayer(u'{0}|layerid=1|layername=tpoly|featurescount=18|geometrytype=Polygon|ogrgettype=0'.format(datasource), u'test_layer_tpoly', u'ogr')
            self.assertTrue(vl_layer_tpoly.isValid())
            count_fields=len(vl_layer_tpoly.fields())
            print('-I-> Testing type/values of a \'Integer64\' of the layername[%s] count[%d] fields[%d] hasGeometry[%d]' % ('tpoly',vl_layer_tpoly.featureCount(),count_fields,vl_layer_tpoly.hasGeometryType()))
            for index in range(count_fields):
                print(u'-I-> Field[%d]: name[%s] type[%s]'% (index, vl_layer_tpoly.fields()[index].name(), vl_layer_tpoly.fields()[index].typeName()))

            # Integer64 [Max-Integer32: 2147483647]  1234567890123/2147483647=574,890473251 ; 2147483647*0.890473251=1912276744,613426397
            gdal_2_value=1234567890123
            gdal_2_value_update=1851851835185
            gdal_1_value=1912276171
            max_integer_32=2147483647
            print('-I-> Retrieving record 7 of layername[%s], checking returned area and int64 test values [%2.3f,%ld]' % ('tpoly',268597.625, gdal_2_value))
            got = [(f.attribute('ogc_fid'), f.attribute('area'), f.attribute('int64')) for f in vl_layer_tpoly.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 7"))]
            if (self.gdal_version_num >= GDAL_COMPUTE_VERSION(2, 0, 0)):
                self.assertEqual(got, [(7, 268597.625, gdal_2_value)])
            else:
                print('-I-> Using version of gdal/ogr[%d] qgis built with gdal[%s], result will be [%d] instead of [%ld].' % (self.gdal_version_num, self.gdal_build_version,gdal_1_value,gdal_2_value))
                self.assertEqual(got, [(7, 268597.625, gdal_1_value)])

            self.assertTrue(vl_layer_tpoly.startEditing())
            self.assertTrue(vl_layer_tpoly.dataProvider().changeAttributeValues({8: {5: gdal_2_value_update}}))
            # self.assertTrue(vl_positions_1955.commitChanges())
            got = [(f.attribute('ogc_fid'), f.attribute('area'), f.attribute('int64')) for f in vl_layer_tpoly.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 8"))]
            if (self.gdal_version_num >= GDAL_COMPUTE_VERSION(2, 0, 0)):
                self.assertEqual(got, [(8, 1634833.375, gdal_2_value_update)])
            else:
                print('-I-> Using version of gdal/ogr[%d] qgis built with gdal[%s], result will be [%d] instead of [%ld].' % (self.gdal_version_num, self.gdal_build_version,720930609,gdal_2_value_update))
                self.assertEqual(got, [(8, 1634833.375, 720930609)])

            vl_layer_tpoly.rollBack(True)


if __name__ == '__main__':
    unittest.main()
