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
    ogr_runtime_supported=1

    def setUp(self):
        """Run before each test."""
        self.gdal_version_num = int(gdal.VersionInfo('VERSION_NUM'))
        try:
            Qgis.GDAL_BUILD_VERSION
        except AttributeError:
            self.gdal_build_num = self.gdal_version_num
            self.gdal_build_version = gdal.VersionInfo('VERSION_NUM')
        else:
            self.ogr_runtime_supported = Qgis.GDAL_OGR_RUNTIME_SUPPORTED
            self.gdal_build_version = Qgis.GDAL_BUILD_VERSION
            self.gdal_build_num = (Qgis.GDAL_BUILD_VERSION_MAJOR*1000000)+(Qgis.GDAL_BUILD_VERSION_MINOR*10000)+(Qgis.GDAL_BUILD_VERSION_REV*100)


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

        test_name='OgrSpatialiteViewsWritable'
        print('-I-> test_00_{0} [{1}]'.format(test_name,'start of test'))
        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrSpatialiteViewsWritable: Deprecated  version of gdal/ogr[{0}] qgis built with gdal[{0}] ogr_runtime_supported[{0}]'.format(self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
            print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{0}] ogr_runtime_supported[{0}]'.format(self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            print('-W-> Note: when qgis is compiled with gdal 2.*\n\tthe application may be killed with: \'symbol lookup error\'\n\t\t undefined symbol: OGR_GT_HasM \n')
            print('-W-> Note: when qgis is compiled with gdal 2.*\n\ttpython-scripts may fail with: \' libqgis_core.so.2.17.0:\'')
            print('\t\t undefined symbol: OGR_F_GetFieldAsInteger64\n\t\t undefined symbol: OGR_F_SetFieldInteger64\n\t\t undefined symbol: OGR_G_ExportToIsoWkb \n')

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_spatialite_views_writable.sqlite')
        print('-I-> Reading db({0})'.format(datasource))
        vl_positions = QgsVectorLayer(u'{}|layerid=0|layername=positions|featurescounted=5|geometrytype=Point|ogrtype=0'.format(datasource), u'SpatialView_writable', u'ogr')
        self.assertTrue(vl_positions.isValid())
        sub_layers_check=4
        count_layers=len(vl_positions.dataProvider().subLayers())
        self.assertEqual(count_layers, sub_layers_check)
        self.assertEqual(vl_positions.featureCount(), 5)

        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_positions.dataProvider().subLayers()[index]))

        if vl_positions.dataProvider().subLayers()[0].startswith('0:positions'):
            print('-I-> SpatialTable({0}) with [{1}] rows'.format('positions',vl_positions.featureCount()))
            count_fields=len(vl_positions.fields())
            for index in range(count_fields):
                print(u'-I-> Field[{0}]: name[{1}] type[{2}]'.format(index, vl_positions.fields()[index].name(), vl_positions.fields()[index].typeName()))

            f = next(vl_positions.getFeatures())
            self.assertEqual(f.geometry().wkbType(), QgsWkbTypes.Point)
            # AttributeError: 'QgsFeature' object has no attribute 'constGeometry'
            self.assertEqual(len(vl_positions.fields()), 5)
            got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 1"))]
            self.assertEqual(got, [(u'Brandenburger Tor', u'Pariser Platz', QDate(1791, 8, 6))])
            vl_positions_1925 = QgsVectorLayer(u'{}|layerid=1|layername=positions_1925|featurescounted=3|geometrytype=Point|ogrtype=0'.format(datasource), u'SpatialView_writable', u'ogr')
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
                print('-W-> SpatialView({0}) unexpected subLayers value returned [{0}] '.format('positions_1925',vl_positions_1925.dataProvider().subLayers()[1]))

            print('-I-> SpatialView({0}) contains 3 rows and only a TRIGGER for INSERT[{1}] with [{1}] rows'.format('positions_1925',self.ogr_spatialview_insert,vl_positions_1925.featureCount()))
            vl_positions_1955 = QgsVectorLayer(u'{}|layerid=2|layername=positions_1955|featurescounted=2|geometrytype=Point|ogrtype=0'.format(datasource), u'SpatialView_writable', u'ogr')
            self.assertTrue(vl_positions_1955.isValid())
            self.assertEqual(vl_positions_1955.featureCount(), 2)
            if vl_positions_1955.dataProvider().subLayers()[2].startswith('2:positions_1955'):
                got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1955.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 4"))]
                self.assertEqual(got, [(u'Siegessäule', u'Große Stern', QDate(1939, 1, 1))])
                caps = vl_positions_1955.dataProvider().capabilities()
                if caps & QgsVectorDataProvider.ChangeAttributeValues:
                    self.ogr_spatialview_update=1;
            else:
                print('-W-> SpatialView({0}) unexpected subLayers value returned [{1}] '.format('positions_1955',vl_positions_1955.dataProvider().subLayers()[2]))

            print('-I-> SpatialView({0}) contains 2 rows and only a TRIGGER for INSERT and UPDATE[{1}] with [{2}] rows'.format('positions_1955',self.ogr_spatialview_update,vl_positions_1955.featureCount()))
            vl_positions_1999 = QgsVectorLayer(u'{}|layerid=3|layername=positions_1999|featurescounted=3|geometrytype=Point|ogrtype=0'.format(datasource), u'SpatialView_writable', u'ogr')
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
                print('-W-> SpatialView({0}) unexpected subLayers value returned [{1}] '.format('positions_1999',vl_positions_1999.dataProvider().subLayers()[3]))

            print('-I-> SpatialView({0}) contains 3 rows and TRIGGERs for INSERT, UPDATE and DELETE[{1}] with [{2}] rows'.format('positions_1999',self.ogr_spatialview_delete,vl_positions_1999.featureCount()))

        if (self.ogr_spatialview_insert and self.ogr_spatialview_update and self.ogr_spatialview_delete):
            print('-I-> This version of gdal/ogr[{0}] qgis built with gdal[{1}] supports writable SpatialViews.'.format(self.gdal_version_num,self.gdal_build_version))
            print('-I-> Can SpatialView({1}) SelectAtId[{0}] ChangeGeometries[{2}]'.format('positions_1999',self.ogr_spatialview_selectatid,self.ogr_spatialview_changegeonetries))
            caps = vl_positions_1925.dataProvider().capabilities()
            self.assertTrue(caps & QgsVectorDataProvider.EditingCapabilities)
            self.assertFalse(caps & QgsVectorDataProvider.ChangeAttributeValues)
            self.assertFalse(caps & QgsVectorDataProvider.DeleteFeatures)
            print('-I-> SpatialView({0}) contains no TRIGGERs for UPDATE and DELETE (as expected), but has EditingCapabilities '.format('positions_1925'))
            caps = vl_positions_1955.dataProvider().capabilities()
            self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)
            self.assertFalse(caps & QgsVectorDataProvider.DeleteFeatures)
            print('-I-> SpatialView({0}) contains no TRIGGER for DELETE (as expected)'.format('positions_1955'))
            caps = vl_positions_1999.dataProvider().capabilities()
            self.assertTrue(caps & QgsVectorDataProvider.AddFeatures)
            self.assertTrue(caps & QgsVectorDataProvider.ChangeAttributeValues)
            print('-I-> SpatialView({0}) contains TRIGGERs for UPDATE and DELETE (as expected)'.format('positions_1999'))
            self.assertTrue(vl_positions_1999.startEditing())
            print('-I-> Note: SpatialView({0) ROLLBACK on a View is not possible.'.format('positions_1999'))
            print('-I-> SpatialView({0}) changing valid_since to 1937-10-02 (from 1998-10-02)'.format ('positions_1999'))
            self.assertTrue(vl_positions_1999.dataProvider().changeAttributeValues({5: {3: QDate(1937, 10, 2)}}))
            self.assertTrue(vl_positions_1999.commitChanges())
            got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1955.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 5"))]
            self.assertEqual(got, [(u'Ampelanlage', u'Potsdamer Platz', QDate(1937, 10, 2))])
            print('-I-> SpatialView({0}) the UPDATEd record was found,'.format('positions_1955'))
            print('-I-> SpatialView({0}) should now show 3 rows instead of 2: [{1}] rows (? but does not)'.format('positions_1955',vl_positions_1955.featureCount()))
            print('-I-> SpatialView({0}) reverting valid_since back to 1998-10-02 (from 1937-10-02)'.format('positions_1955'))
            self.assertTrue(vl_positions_1955.startEditing())
            self.assertTrue(vl_positions_1955.dataProvider().changeAttributeValues({5: {3: QDate(1998, 10, 2)}}))
            self.assertTrue(vl_positions_1955.commitChanges())
            got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_since')) for f in vl_positions_1999.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 5"))]
            self.assertEqual(got, [(u'Ampelanlage', u'Potsdamer Platz', QDate(1998, 10, 2))])
            print('-I-> SpatialView({0}) the UPDATEd record was found,'.format('positions_1999'))
            # vl_positions_1999.rollBack(True)
            print('-I-> SpatialView({0}) should now again show 2: [{1}] rows'.format('positions_1955',vl_positions_1955.featureCount()))
        else:
           print('-W-> This version of gdal[{0}] does not support writable SpatialViews.'.format(self.gdal_build_version))

        if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
            print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{0}]\n\t Note: this test has not failed. '.format(self.gdal_version_num, self.gdal_build_version))

        # Should delete the retrieved layers
        del vl_positions
        del vl_positions_1925
        del vl_positions_1955
        del vl_positions_1999
        print('-I-> test_00_{0} [{1}]'.format(test_name,'end of test'))

###############################################################################
# Test Spatialite SpatialTable with more than 1 geometry
# - created with gdal autotest/auto/ogr_sql_sqlite.py spatialite_8()
# contains 1 SpatialTable with 2 geometries and 2 SpatialViews showing each geometry

    def test_01_OgrSpatialTableMultipleGeometries(self):

        test_name='OgrSpatialTableMultipleGeometries'
        print('\n-I-> test_01_{0} [{1}]'.format(test_name,'start of test'))
        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrSpatialTableMultipleGeometries: Deprecated  version of gdal/ogr[{0}] qgis built with gdal[{0}] ogr_runtime_supported[{0}]'.format(self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_spatialite_8_2_geometries.sqlite')
        print('\n-I-> Reading db({0})'.format(datasource))
        vl_spatialite_8 = QgsVectorLayer(u'{}'.format(datasource), u'spatialite_8', u'ogr')
        self.assertTrue(vl_spatialite_8.isValid())
        count_layers=len(vl_spatialite_8.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{1}], [{2}] layers were found.'.format(self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
            if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0) and count_layers == 5 and index == 0):
                # Sublayer[0] : [0:test:1:Point] ; should be : [0:test(geom1):1:Point]
                print('-E-> invalid entry: Sublayer[{0}] : [{1}]'.format(index, vl_spatialite_8.dataProvider().subLayers()[index]))
            else:
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_spatialite_8.dataProvider().subLayers()[index]))

        del vl_spatialite_8

        if (count_layers ==4):
            vl_test_point = QgsVectorLayer(u'{}|layerid=0|layername=test(geom1)|featurescounted=1|geometrytype=Point|ogrtype=0'.format(datasource), u'test_point', u'ogr')
            vl_test_linestring = QgsVectorLayer(u'{}|layerid=1|layername=test(geom2)|featurescounted=1|geometrytype=LineString|ogrtype=0'.format(datasource), u'test_linestring', u'ogr')
            vl_view_point = QgsVectorLayer(u'{}|layerid=2|layername=view_test_geom1|featurescounted=1|geometrytype=Point|ogrtype=0'.format(datasource), u'view_point', u'ogr')
            vl_view_linestring = QgsVectorLayer(u'{}|layerid=3|layername=view_test_geom2|featurescounted=1|geometrytype=LineString|ogrtype=0'.format(datasource), u'view_linestring', u'ogr')
            features_01 = [f_iter for f_iter in vl_test_point.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            test_point_geom = [f_iter.geometry() for f_iter in features_01][0].geometry()
            features_02 = [f_iter for f_iter in vl_test_linestring.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            test_linestring_geom = [f_iter.geometry() for f_iter in features_02][0].geometry()
            features_03 = [f_iter for f_iter in vl_view_point.getFeatures(QgsFeatureRequest().setFilterExpression("pk_id = 1"))]
            view_point_geom = [f_iter.geometry() for f_iter in features_03][0].geometry()
            features_04 = [f_iter for f_iter in vl_view_linestring.getFeatures(QgsFeatureRequest().setFilterExpression("pk_id = 1"))]
            view_linestring_geom = [f_iter.geometry() for f_iter in features_04][0].geometry()
            print('-I-> Testing if the Point of the SpatialTable is the same as in the SpatialView. [{0}]'.format('SRID=4326;POINT(0 1)'))
            self.assertEquals((test_point_geom.x(), test_point_geom.y()), (view_point_geom.x(), view_point_geom.y()))
            print('-I-> Testing if the LineString of the SpatialTable is the same as in the SpatialView. [{0}]'.format('SRID=4326;LINESTRING(0 1,2 3)'))
            self.assertEqual((test_linestring_geom.pointN(0).x(), test_linestring_geom.pointN(0).y(),test_linestring_geom.pointN(1).x(), test_linestring_geom.pointN(1).y()), (view_linestring_geom.pointN(0).x(), view_linestring_geom.pointN(0).y(),view_linestring_geom.pointN(1).x(), view_linestring_geom.pointN(1).y()))
            del vl_test_point
            del vl_test_linestring
            del vl_view_point
            del vl_view_linestring
        else:
            # Sublayer[0] : [0:test:1:Point] ; should be : [0:test(geom1):1:Point]
            print('-E-> Testing invalid entry: Sublayer[{0}] : [{1}]'.format(0, '0:test:1:Point'))
            vl_test_point = QgsVectorLayer(u'{}|layerid=0|layername=0:test:1:Point'.format(datasource), u'test_point', u'ogr')
            self.assertFalse(vl_test_point.isValid())
            print('-I-> isValid() has returned False: Sublayer[{0}] : [{1}]'.format(0, '0:test:1:Point'))
            del vl_test_point
            print('-I-> Note:\n\t the layerid numbering returned between gdal 1.* and gdal 2.* are different\n\t gdal_1[{0}]  gdal_2[{1}]'.format('0:test(geom1):1:Point','3:test(geom1):1:Point'))
            vl_test_point = QgsVectorLayer(u'{}|layerid=3|layername=test(geom1)|featurescounted=1|geometrytype=Point|ogrtype=0'.format(datasource), u'test_point', u'ogr')
            vl_test_linestring = QgsVectorLayer(u'{}|layerid=4|layername=test(geom2)|featurescounted=1|geometrytype=LineString|ogrtype=0'.format(datasource), u'test_linestring', u'ogr')
            vl_view_point = QgsVectorLayer(u'{}|layerid=1|layername=view_test_geom1|featurescounted=1|geometrytype=Point|ogrtype=0'.format(datasource), u'view_point', u'ogr')
            vl_view_linestring = QgsVectorLayer(u'{}|layerid=2|layername=view_test_geom2|featurescounted=1|geometrytype=LineString|ogrtype=0'.format(datasource), u'view_linestring', u'ogr')
            features_01 = [f_iter for f_iter in vl_test_point.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            test_point_geom = [f_iter.geometry() for f_iter in features_01][0].geometry()
            features_02 = [f_iter for f_iter in vl_test_linestring.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            test_linestring_geom = [f_iter.geometry() for f_iter in features_02][0].geometry()
            features_03 = [f_iter for f_iter in vl_view_point.getFeatures(QgsFeatureRequest().setFilterExpression("pk_id = 1"))]
            view_point_geom = [f_iter.geometry() for f_iter in features_03][0].geometry()
            features_04 = [f_iter for f_iter in vl_view_linestring.getFeatures(QgsFeatureRequest().setFilterExpression("pk_id = 1"))]
            view_linestring_geom = [f_iter.geometry() for f_iter in features_04][0].geometry()
            print('-I-> Testing if the Point of the SpatialTable is the same as in the SpatialView. [{0}]'.format('SRID=4326;POINT(0 1)'))
            self.assertEquals((test_point_geom.x(), test_point_geom.y()), (view_point_geom.x(), view_point_geom.y()))
            print('-I-> Testing if the LineString of the SpatialTable is the same as in the SpatialView. [{0}]'.format('SRID=4326;LINESTRING(0 1,2 3)'))
            self.assertEqual((test_linestring_geom.pointN(0).x(), test_linestring_geom.pointN(0).y(),test_linestring_geom.pointN(1).x(), test_linestring_geom.pointN(1).y()),
                                        (view_linestring_geom.pointN(0).x(), view_linestring_geom.pointN(0).y(),view_linestring_geom.pointN(1).x(), view_linestring_geom.pointN(1).y()))
            del vl_test_point
            del vl_test_linestring
            del vl_view_point
            del vl_view_linestring

        print('-I-> test_01_{0} [{1}]'.format(test_name,'end of test'))

###############################################################################
# Test Spatialite SpatialTables with XYZM values
# - created with gdal autotest/auto/ogr_sql_sqlite.py spatialite_5()
# contains 47 SpatialTables with (Multi-) Points/Linestrings/Polygons/GeometryCollection as XY,XYZ,XYM,XYZM
# Test Geometries taken from /autotest/ogr/data/curves_line.csv and curves_polygon.csv
# - to test the correct reading of: CircularString, CompoundCurve, CurvePolygon, MultiCurve and MultiSurface
#

    def test_02_OgrSpatialTableXYZM(self):

        test_name='OgrSpatialTableXYZM'
        print('\n-I-> test_02_{0} [{1}]'.format(test_name,'start of test'))
        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrSpatialTableXYZM: Deprecated  version of gdal/ogr[{0}] qgis built with gdal[{0}] ogr_runtime_supported[{0}]'.format(self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_spatialite_5_ZM.sqlite')
        print('\n-I-> Reading db({0})'.format(datasource))
        vl_spatialite_5 = QgsVectorLayer(u'{}'.format(datasource), u'spatialite_5', u'ogr')
        self.assertTrue(vl_spatialite_5.isValid())
        count_layers=len(vl_spatialite_5.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{1}], [{2}] layers were found.'.format(self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_spatialite_5.dataProvider().subLayers()[index]))

        del vl_spatialite_5

        if (count_layers ==47):
            # Points
            vl_test_geom = QgsVectorLayer(u'{}|layerid=1|layername=test1|featurescounted=1|geometrytype=Point25D|ogrtype=0'.format(datasource), u'test_pointz', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'POINT Z\' EWKT[{0}] count[{1},{2}]'.format('SRID=4326;POINT(1 2 3)',vl_test_geom.featureCount(),len(features_vl_test)))
            test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
            # self.assertEqual(test_geom.wkbType(), QgsWkbTypes.PointZ)
            self.assertEquals((test_geom.x(), test_geom.y(), test_geom.z()), (1,2,3))
            del test_geom
            del features_vl_test
            del vl_test_geom
            # MultiPoints
            vl_test_geom = QgsVectorLayer(u'{}|layerid=22|layername=test22|featurescounted=1|geometrytype=MultiPoint25D|ogrtype=0'.format(datasource), u'test_multipointz', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'MULTIPOINT Z\' EWKT[{0}] count[{1},{2}]'.format('SRID=4326;MULTIPOINT(1 2 3,4 5 6)',vl_test_geom.featureCount(),len(features_vl_test)))
            count_features=len(features_vl_test)
            print('-I-> count_features[{0}]'.format(count_features))
            #test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
            #self.assertEqual(test_geom.numGeometries(), 2)
            #self.assertEqual(test_geom.wkbType(), QgsWkbTypes.MultiPointZ)
            #self.assertEquals((test_geom.geometryN(0).x(), test_geom.geometryN(0).y(), test_geom.geometryN(0).z()),
            #                            (test_geom.geometryN(1).x(), test_geom.geometryN(1).y(), test_geom.geometryN(1).z()),
            #                             (1,2,3,4,5,6))
            # del test_geom
            del features_vl_test
            del vl_test_geom
            vl_test_geom = QgsVectorLayer(u'{}|layerid=23|layername=test23|featurescounted=1|geometrytype=MultiPointM|ogrtype=0'.format(datasource), u'test_multipointz', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            # del test_geom
            # del features_vl_test
            del vl_test_geom
            # Linestrings
            vl_test_geom = QgsVectorLayer(u'{}|layerid=8|layername=test8|featurescounted=1|geometrytype=LineString25D|ogrtype=0'.format(datasource), u'test_linestringz', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'LINESTRING Z\' EWKT[{0}] count[{1},{2}]'.format('SRID=4326;LINESTRING(1 2 3,4 5 6)',vl_test_geom.featureCount(),len(features_vl_test)))
            test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
            self.assertEqual(test_geom.wkbType(), QgsWkbTypes.LineString25D)
            self.assertEqual((test_geom.pointN(0).x(), test_geom.pointN(0).y(),test_geom.pointN(0).z(),
                                         test_geom.pointN(1).x(), test_geom.pointN(1).y(),test_geom.pointN(1).z()),
                                        (1,2,3,4,5,6))
            del test_geom
            del features_vl_test
            del vl_test_geom
            # Polygons
            vl_test_geom = QgsVectorLayer(u'{}|layerid=16|layername=test16|featurescounted=1|geometrytype=Polygon25D|ogrtype=0'.format(datasource), u'test_polygonz', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'POLYGON Z\' EWKT[{0}] count[{1},{2}]'.format('SRID=4326;POLYGON((1 2 10,1 3 -10,2 3 20,2 2 -20,1 2 10))',vl_test_geom.featureCount(),len(features_vl_test)))
            test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
            self.assertEqual(test_geom.wkbType(), QgsWkbTypes.Polygon25D)
            self.assertEqual((test_geom.exteriorRing().pointN(0).x(), test_geom.exteriorRing().pointN(0).y(),test_geom.exteriorRing().pointN(0).z(),
                                         test_geom.exteriorRing().pointN(1).x(), test_geom.exteriorRing().pointN(1).y(),test_geom.exteriorRing().pointN(1).z(),
                                         test_geom.exteriorRing().pointN(2).x(), test_geom.exteriorRing().pointN(2).y(),test_geom.exteriorRing().pointN(2).z(),
                                         test_geom.exteriorRing().pointN(3).x(), test_geom.exteriorRing().pointN(3).y(),test_geom.exteriorRing().pointN(3).z(),
                                         test_geom.exteriorRing().pointN(4).x(), test_geom.exteriorRing().pointN(4).y(),test_geom.exteriorRing().pointN(4).z()),
                                        (1,2,10,1,3,-10,2,3,20,2,2,-20,1,2,10))
            del test_geom
            del features_vl_test
            del vl_test_geom
            vl_test_geom = QgsVectorLayer(u'{}|layerid=2|layername=test2|featurescounted=1|geometrytype=PointM|ogrtype=0'.format(datasource), u'test_pointm', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'POINT M\' EWKT[{0}] count[{1},{2}] HasGeometryType[{3}]'.format('SRID=4326;POINTM(1 2 3)',vl_test_geom.featureCount(),len(features_vl_test) , vl_test_geom.hasGeometryType()))
            # pprint(getmembers(features_vl_test))
            if (len(features_vl_test) > 0):
                test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
                if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
                    print('-I-> Using version [build gdal {0}] of gdal/ogr[{1}] which does not support M values (pointN(n).m() returns 0)'.format(self.gdal_build_version, self.gdal_version_num))
                    # build gdal.1 running with gdal 1: -I-> Sublayer[2] : [2:test2:1:Point:0]
                    self.assertNotEqual(test_geom.wkbType(), QgsWkbTypes.PointM)
                    self.assertEquals((test_geom.x(), test_geom.y(), test_geom.m()), (1,2,0))
                else:
                    # build gdal.1+2 running with gdal 2: -I-> Sublayer[2] : [2:test2:1:PointM:0]
                    self.assertEqual(test_geom.wkbType(), QgsWkbTypes.PointM)
                    self.assertEquals((test_geom.x(), test_geom.y(), test_geom.m()), (1,2,3))
                del test_geom
            else:
             print('-I-> Using version [build gdal {0}] of gdal/ogr[{1}] which does not support M values (pointN(n).m() returns 0)'.format(self.gdal_build_version, self.gdal_version_num))

            del features_vl_test
            del vl_test_geom
            vl_test_geom = QgsVectorLayer(u'{}|layerid=3|layername=test3|featurescounted=1|geometrytype=PointZM|ogrtype=0'.format(datasource), u'test_pointzm', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'POINT ZM\' EWKT[{0}] count[{1},{2}]'.format('SRID=4326;POINT(1 2 3 4)',vl_test_geom.featureCount(),len(features_vl_test)))
            if (len(features_vl_test) > 0):
                test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
                if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
                    print('-I-> Using version [build gdal.{0}] of gdal/ogr[{1}] which does not support M values (pointN(n).m() returns 0)'.format(self.gdal_build_version,self.gdal_version_num))
                    # build gdal.1 running with gdal 1: I-> Sublayer[3] : [3:test3:1:Point25D:0]
                    self.assertNotEqual(test_geom.wkbType(), QgsWkbTypes.PointZM)
                    self.assertEquals((test_geom.x(), test_geom.y(), test_geom.z(), test_geom.m()), (1,2,3,0))
                else:
                    # build gdal.1+2 running with gdal 2: -I-> Sublayer[3] : [3:test3:1:PointZM:0]
                    self.assertEqual(test_geom.wkbType(), QgsWkbTypes.PointZM)
                    self.assertEquals((test_geom.x(), test_geom.y(), test_geom.z(), test_geom.m()), (1,2,3,4))
                del test_geom
            else:
             print('-I-> Using version [build gdal.{0}] of gdal/ogr[{1}] which does not support M values (pointN(n).m() returns 0)'.format(self.gdal_build_version,self.gdal_version_num))

            del features_vl_test
            del vl_test_geom
            vl_test_geom = QgsVectorLayer(u'{}|layerid=10|layername=test10|featurescounted=1|geometrytype=LineStringM|ogrtype=0'.format(datasource), u'test_linestringm', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'LINESTRING M\' EWKT[{0}] count[{1},{2}]'.format('SRID=4326;LINESTRINGM(1 2 3,4 5 6)',vl_test_geom.featureCount(),len(features_vl_test)))
            if (len(features_vl_test) > 0):
                test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
                if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
                    print('-I-> Using version [build gdal.{0}] of gdal/ogr[{1}] which does not support M values (pointN(n).m() returns 0)'.format(self.gdal_build_version, self.gdal_version_num))
                    # build gdal.1 running with gdal 1:  -I-> Sublayer[10] : [10:test10:1:LineString:0]
                    self.assertNotEqual(test_geom.wkbType(), QgsWkbTypes.LineStringM)
                    self.assertEqual((test_geom.pointN(0).x(), test_geom.pointN(0).y(),test_geom.pointN(0).m(),
                                         test_geom.pointN(1).x(), test_geom.pointN(1).y(),test_geom.pointN(1).m()),
                                        (1,2,0,4,5,0))
                else:
                    # build gdal.1+2 running with gdal 2: -I-> Sublayer[10] : [10:test10:1:LineStringM:0]
                    self.assertEqual(test_geom.wkbType(), QgsWkbTypes.LineStringM)
                    self.assertEqual((test_geom.pointN(0).x(), test_geom.pointN(0).y(),test_geom.pointN(0).m(),
                                         test_geom.pointN(1).x(), test_geom.pointN(1).y(),test_geom.pointN(1).m()),
                                        (1,2,3,4,5,6))
            else:
             print('-I-> Using version [build gdal.{0}] of gdal/ogr[{1}] which does not support M values (pointN(n).m() returns 0)'.format(self.gdal_build_version,self.gdal_version_num))

            del features_vl_test
            del vl_test_geom
            vl_test_geom = QgsVectorLayer(u'{}|layerid=12|layername=test12|featurescounted=1|geometrytype=LineStringZM|ogrtype=0'.format(datasource), u'test_linestringm', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 1"))]
            print('-I-> Testing type/values of a \'LINESTRING ZM\' EWKT[{0}] count[{1},{2}]'.format('SRID=4326;LINESTRING(1 2 3 4,5 6 7 8)',vl_test_geom.featureCount(),len(features_vl_test)))
            if (len(features_vl_test) > 0):
                test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
                if (self.gdal_version_num < GDAL_COMPUTE_VERSION(2, 0, 0)):
                    print('-I-> Using version [build gdal.{0}] of gdal/ogr[{1}] which does not support M values (pointN(n).m() returns 0)'.format(self.gdal_build_version,self.gdal_version_num))
                    # build gdal.1 running with gdal: 1 -I-> Sublayer[12] : [12:test12:1:LineString25D:0]
                    self.assertNotEqual(test_geom.wkbType(), QgsWkbTypes.LineStringZM)
                    self.assertEqual((test_geom.pointN(0).x(), test_geom.pointN(0).y(),test_geom.pointN(0).z(),test_geom.pointN(0).m(),
                                         test_geom.pointN(1).x(), test_geom.pointN(1).y(),test_geom.pointN(1).z(),test_geom.pointN(1).m()),
                                        (1,2,3,0,5,6,7,0))
                else:
                    # build gdal.1+2 running with gdal 2: -I-> Sublayer[12] : [12:test12:1:LineStringZM:0]
                    self.assertEqual(test_geom.wkbType(), QgsWkbTypes.LineStringZM)
                    self.assertEqual((test_geom.pointN(0).x(), test_geom.pointN(0).y(),test_geom.pointN(0).z(),test_geom.pointN(0).m(),
                                         test_geom.pointN(1).x(), test_geom.pointN(1).y(),test_geom.pointN(1).z(),test_geom.pointN(1).m()),
                                        (1,2,3,4,5,6,7,8))
                del test_geom
            else:
             print('-I-> Using version [build gdal.{0}] of gdal/ogr[{1}] which does not support M values (pointN(n).m() returns 0)'.format(self.gdal_build_version, self.gdal_version_num))

            del features_vl_test
            del vl_test_geom
            test_geom = None
            vl_test_geom = None
            features_vl_test = None

        if (self.gdal_version_num >= GDAL_COMPUTE_VERSION(2, 0, 0)):
            datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_circularstring.csv')
            print('\n-I-> Reading db({0})'.format(datasource))
            vl_circularstring = QgsVectorLayer(u'{}|layerid=0|layername=gdal_220.autotest.ogr_circularstring|featurescounted=1|geometrytype=CircularString|ogrtype=1'.format(datasource), u'test_circularstring', u'ogr')
            print('-I-> Reading Geometry-Type=CircularString: hasGeometryType[{0},{1}] '.format(vl_circularstring.hasGeometryType(),vl_circularstring.wkbType()))
            self.assertTrue(vl_circularstring.isValid())
            count_fields=len(vl_circularstring.fields())
            for index in range(count_fields):
                print(u'-I-> Field[{0}]: name[{1}] type[{2}]'.format(index, vl_circularstring.fields()[index].name(), vl_circularstring.fields()[index].typeName()))

            features_vl_circularstring = next(vl_circularstring.getFeatures())
            test_geom = features_vl_circularstring.geometry()
            print('-I-> checking Geometry-Type=CircularString: [{0},{1}] '.format(test_geom.wkbType(), test_geom.type()))
            # AttributeError: 'QgsGeometry' object has no attribute 'geometryType'
            self.assertEqual(test_geom.wkbType(), QgsWkbTypes.CircularString)
            del test_geom
            del features_vl_circularstring
            del vl_circularstring
            datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_compoundcurve.csv')
            print('\n-I-> Reading db({0})'.format(datasource))
            vl_compoundcurve = QgsVectorLayer(u'{}|layerid=0|layername=gdal_220.autotest.ogr_compoundcurve|featurescounted=1|geometrytype=CompoundCurve|ogrtype=1'.format(datasource), u'test_compoundcurve', u'ogr')
            print('-I-> Reading Geometry-Type=CompoundCurve: hasGeometryType[{0},{1}] '.format(vl_compoundcurve.hasGeometryType(),vl_compoundcurve.wkbType()))
            self.assertTrue(vl_compoundcurve.isValid())
            features_vl_compoundcurve = next(vl_compoundcurve.getFeatures())
            test_geom = features_vl_compoundcurve.geometry()
            print('-I-> checking Geometry-Type=CompoundCurve: [{0},{1}] '.format(test_geom.wkbType(), test_geom.type()))
            self.assertEqual(test_geom.wkbType(), QgsWkbTypes.CompoundCurve)
            del test_geom
            del features_vl_compoundcurve
            del vl_compoundcurve
            datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_curvepolygon.csv')
            print('\n-I-> Reading db({0})'.format(datasource))
            vl_curvepolygon = QgsVectorLayer(u'{}|layerid=0|layername=gdal_220.autotest.ogr_curvepolygon|featurescounted=1|geometrytype=CurvePolygon|ogrtype=1'.format(datasource), u'test_curvepolygon', u'ogr')
            print('-I-> Reading Geometry-Type=CurvePolygon: hasGeometryType[{0},{1}] '.format(vl_curvepolygon.hasGeometryType(),vl_curvepolygon.wkbType()))
            self.assertTrue(vl_curvepolygon.isValid())
            features_vl_curvepolygon = next(vl_curvepolygon.getFeatures())
            test_geom = features_vl_curvepolygon.geometry()
            print('-I-> checking Geometry-Type=CurvePolygon: [{0},{1}] '.format(test_geom.wkbType(), test_geom.type()))
            self.assertEqual(test_geom.wkbType(), QgsWkbTypes.CurvePolygon)
            del test_geom
            del features_vl_curvepolygon
            del vl_curvepolygon
            datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_multicurves.csv')
            print('\n-I-> Reading db({0})'.format(datasource))
            vl_multicurves = QgsVectorLayer(u'{}|layerid=0|layername=gdal_220.autotest.ogr_multicurves|featurescounted=1|geometrytype=MultiCurve|ogrtype=1'.format(datasource), u'test_multicurves', u'ogr')
            print('-I-> Reading Geometry-Type=MultiCurve: hasGeometryType[{0},{1}] '.format(vl_multicurves.hasGeometryType(),vl_multicurves.wkbType()))
            self.assertTrue(vl_multicurves.isValid())
            features_vl_multicurves = next(vl_multicurves.getFeatures())
            test_geom = features_vl_multicurves.geometry()
            print('-I-> checking Geometry-Type=MultiCurve: [{0},{1}] '.format(test_geom.wkbType(), test_geom.type()))
            self.assertEqual(test_geom.wkbType(), QgsWkbTypes.MultiCurve)
            del test_geom
            del features_vl_multicurves
            del vl_multicurves
            datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_multisurface.csv')
            print('\n-I-> Reading db({0})'.format(datasource))
            vl_multisurface = QgsVectorLayer(u'{}|layerid=0|layername=gdal_220.autotest.ogr_multisurface|featurescounted=1|geometrytype=MultiSurface|ogrtype=1'.format(datasource), u'test_multisurface', u'ogr')
            print('-I-> Reading Geometry-Type=MultiSurface: hasGeometryType[{0},{1}] '.format(vl_multisurface.hasGeometryType(),vl_multisurface.wkbType()))
            self.assertTrue(vl_multisurface.isValid())
            features_vl_multisurface = next(vl_multisurface.getFeatures())
            test_geom = features_vl_multisurface.geometry()
            print('-I-> checking Geometry-Type=MultiSurface: [{0},{1}] '.format(test_geom.wkbType(), test_geom.type()))
            self.assertEqual(test_geom.wkbType(), QgsWkbTypes.MultiSurface)
            del test_geom
            del features_vl_multisurface
            del vl_multisurface
        else:
             print('-I-> Using version [build gdal.{0}] of gdal/ogr[{0}] which does not support:\n\t CircularString, CompoundCurve, CurvePolygon, MultiCurve and MultiSurface geometries.'.format(self.gdal_build_version, self.gdal_version_num))

        print('-I-> test_02_{0} [{1}]'.format(test_name,'end of test'))

###############################################################################
# GML 2 MultiPolygon with InternalRings
# - exported from Spatialite Database: SELECT AsGml(soldner_polygon)  FROM pg_bezirke_1938 WHERE id_admin=1902010800;
# Contains border of the District of Spandau, Berlin between 1938-1945
# - Spandau contains 7 Sub-Districts, some portions of 1 Sub-district contains 6 Exclaves and 4 Enclaves ; a 2nd Sub-District contains 1 Enclave
# --> 7+6=13 Polygons and 5 InternalRings
# Goal is to loop through all Polygons and caluclate the areas of the Polygons and InternalRings and compair the results returned by spatialite
# --> 7+6=13 Polygons and 5 InternalRings
# -I-> the area of the MultiPolygon does not include the area of the 5 InternalRings, returning the same area value as Spatialite

    def test_03_OgrGMLMutiPolygon(self):

        test_name='OgrGMLMutiPolygon'
        print('\n-I-> test_03_{0} [{1}]'.format(test_name,'start of test'))
        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrGMLMutiPolygon: Deprecated  version of gdal/ogr[{0}] qgis built with gdal[{0}] ogr_runtime_supported[{0}]'.format(self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.ogr_bezirk_Spandau_1938.3068.gml')
        print('\n-I-> Reading GML with 1 Geometry db({0})'.format(datasource))
        vl_spandau_1938 = QgsVectorLayer(u'{}'.format(datasource), u'spandau_1938', u'ogr')
        self.assertTrue(vl_spandau_1938.isValid())
        count_layers=len(vl_spandau_1938.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{1}], [{2}] layers were found.'.format(self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_spandau_1938.dataProvider().subLayers()[index]))

        del vl_spandau_1938
        if (count_layers ==1):
            vl_test_geom = QgsVectorLayer(u'{}|layerid=0|layername=bezirk_Spandau_1938|featurescounted=1|geometrytype=MultiPolygon|ogrtype=0'.format(datasource), u'test_multipolygon', u'ogr')
            self.assertTrue(vl_test_geom.isValid())
            count_fields=len(vl_test_geom.fields())
            print('-I-> Testing type/values of a \'MULTIPOLYGON\' layername[{0}] count[{1}] fields[{2}] hasGeometry[{3}]'.format('bezirk_Spandau_1938',vl_test_geom.featureCount(),count_fields,vl_test_geom.hasGeometryType()))
            for index in range(count_fields):
                print(u'-I-> Field[{0}]: name[{1}] type[{2}]'.format(index, vl_test_geom.fields()[index].name(), vl_test_geom.fields()[index].typeName()))
            # <ogr:belongs_to>1902010800</ogr:belongs_to>
            features_vl_test = [f_iter for f_iter in vl_test_geom.getFeatures(QgsFeatureRequest().setFilterExpression("fid='1902010800'"))]
            #pprint(getmembers(features_vl_test))
            count_features=len(features_vl_test)
            if (count_features > 0):
                geom_area_sum=0
                internal_ring_area_sum_total=0
                exterior_ring_area_sum=0
                polygon_ring_area_sum=0
                test_geom = [f_iter.geometry() for f_iter in features_vl_test][0].geometry()
                count_polygons=test_geom.numGeometries()
                geom_area=test_geom.area() # do not round
                print('-I-> MultiPolygon count[{0}] area[{1:f}]'.format(count_polygons,geom_area))
                self.assertEqual(round(geom_area,6), 87622761.119470)
                self.assertEqual(count_polygons, 13)
                self.assertEqual(test_geom.wkbType(), QgsWkbTypes.MultiPolygon)
                for index in range(count_polygons):
                    test_polygon = test_geom.geometryN(index)
                    self.assertEqual(test_polygon.wkbType(), QgsWkbTypes.Polygon)
                    count_rings=test_polygon.numInteriorRings()
                    polygon_area=round(test_polygon.area(),6)
                    geom_area_sum+=test_polygon.area() # do not round
                    polygon_ring_area_sum+=polygon_area
                    # Goal: Cast exterior/interiorRing to a Polygon properly to retrieve the area
                    exterior_ring = test_polygon.exteriorRing()
                    exterior_ring_area=round(QgsGeometry.fromQPolygonF(exterior_ring.toCurveType().asQPolygonF()).area(),6)
                    exterior_ring_area_sum+=QgsGeometry.fromQPolygonF(exterior_ring.toCurveType().asQPolygonF()).area()
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
                        polygon_area_check=0;
                        internal_ring_area_sum=0
                        for index_ring in range(count_rings):
                            interior_ring = test_polygon.interiorRing(index_ring)
                            ring_area=round(QgsGeometry.fromQPolygonF(interior_ring.toCurveType().asQPolygonF()).area(),6)
                            internal_ring_area_sum+=QgsGeometry.fromQPolygonF(interior_ring.toCurveType().asQPolygonF()).area()
                            print('\t-I-> InteriorRing[{0}] type[{1}] isClosed[{2}] isRing[{3}] area[{4:f}]'.format(index_ring,interior_ring.geometryType(),interior_ring.isClosed(),interior_ring.isRing(),ring_area))

                        internal_ring_area_sum_total+=internal_ring_area_sum
                        polygon_area_check=round(exterior_ring_area-internal_ring_area_sum,6)
                        print('\t-I-> Testing if External-Ring[{0:f}] minus Internal-Rings[{1:f}] == Polygon-Area[{2:f}]'.format(exterior_ring_area,internal_ring_area_sum,polygon_area))
                        self.assertEqual(polygon_area, polygon_area_check)
                        print('-I-> Polygon count[{0}] polygon_area[{1:f}] = (exterior_ring_area[{2:f}] minus internal_ring_area_sum[{3:f}])'.format(index, polygon_area,exterior_ring_area,internal_ring_area_sum))
                    else:
                        print('-I-> Polygon[{0}] area[{1:f}]: ExternalRing-Area[{2:f}] ; InteriorRings[{3}]'.format(index, polygon_area, exterior_ring_area, count_rings))

                geom_area_sum_check=exterior_ring_area_sum-internal_ring_area_sum_total
                geom_area_sum_diff=geom_area_sum-geom_area_sum_check;
                print('-I-> diff [{0:f}]  '.format(geom_area_sum_diff))
                print('-I-> MultiPolygon geom_area_sum_check[{0:f}] = exterior_ring_area_sum_total[{1:f}] minus internal_ring_area_sum_total[{2:f}])'.format(geom_area_sum_check, exterior_ring_area_sum, internal_ring_area_sum_total))
                print('-I-> MultiPolygon count[{0}] MultiPolygon-Area[{1:f}] Polygon-Areas[{2:f}] Ring-Areas[{3:f}] '.format(count_polygons,geom_area,geom_area_sum,geom_area_sum_check))
                self.assertEqual(geom_area,geom_area_sum)
                self.assertEqual(round(geom_area_sum,5),round(geom_area_sum_check,5))
                # -I-> MultiPolygon count[13] MultiPolygon-Area[87622761.119470] Polygon-Areas[87622761.119470] Ring-Areas[87622761.119469]
                # SELECT ROUND(ST_Area(soldner_polygon),5) AS area_polygon, ST_Area(soldner_ring) AS area_polygon,* FROM pg_bezirke_1938 WHERE id_admin=1902010800;
                # 87622761.119469	87622761.119469
                print('-I-> MultiPolygon-Area check correct [does not include the area of the Internal-Rings[{0:f}])'.format(internal_ring_area_sum_total))

        print('-I-> test_03_{0} [{1}]'.format(test_name,'end of test'))

###############################################################################
# Test GML with more than 1 geometry
# - 2 files are being checked
# - 1 with 2 POINTs with 1 record
# --  check if given values of POINT are correct
# - 1 created from a spatialite TABLE with
# --> ogr2ogr -f GML gdal_220.ogr_Rauletshof_1909.gml admin_geometries_11.db -sql "SELECT * FROM admin_geometries"
# - TABLE contains imformation about 6 historical streets, with metadata
# -- the outline of the street as a POLYGON
# -- the center of the street as a LINESTRING
# -- the center of the street LINESTRING as a POINT
# - Goals are:
# --  read record 2 (Raulet's Hof) for the 3 types
# --- determin correct Geometry-Type, testing that the correct Geometry-Field index is being used
# ---  check value of POINT (or first POINT of LINESTRING or of ExternalRing of POLYGON)
# Note: date fields are being returned as strings

    def test_04_OgrGMLMultipleGeometries(self):

        test_name='OgrGMLMultipleGeometries'
        print('\n-I-> test_04_{0} [{1}]'.format(test_name,'start of test'))
        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrGMLMutiPolygon: Deprecated  version of gdal/ogr[{0}] qgis built with gdal[{0}] ogr_runtime_supported[{0}]'.format(self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_multiplegeomfields.gml')
        print('\n-I-> Reading GML with 2 Geometries db({0}) - test with 2 POINTs, one record'.format(datasource))
        geometry_name_01=u"geomfield1"
        uri_name_point_01=u'{0}|layerid=0|layerid=0|layername=multiplegeomfields|geometryname={1}'.format(datasource,geometry_name_01)
        geometry_name_02=u"geomfield2"
        uri_name_point_02=u'{0}|layerid=0|layerid=0|layername=multiplegeomfields|geometryname={1}'.format(datasource,geometry_name_02)
        # Testing Point 01
        vl_point_01 = QgsVectorLayer(uri_name_point_01,test_name, u'ogr')
        self.assertTrue(vl_point_01.isValid())
        count_fields=len(vl_point_01.fields())
        print('-I-> Testing type/values of a \'POINT\' of first geometry layername[{0}] count[{1}] fields[{2}] hasGeometry[{3}]'.format('multiplegeomfields',vl_point_01.featureCount(),count_fields,vl_point_01.hasGeometryType()))
        for index in range(count_fields):
            print(u'-I-> Field[{0}]: name[{1}] type[{2}]'.format(index, vl_point_01.fields()[index].name(), vl_point_01.fields()[index].typeName()))
        count_layers=len(vl_point_01.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{1}], [{2}] layers were found.'.format(self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_point_01.dataProvider().subLayers()[index]))

        features_vl_point_01 = [f_iter for f_iter in vl_point_01.getFeatures()]
        # features_vl_point_01 = [f_iter for f_iter in vl_point_02.getFeatures(QgsFeatureRequest().setFilterExpression("fid='0'"))]
        print('-I-> Testing type/values of a \'POINT\' EWKT[{0}] count[{1},{2}]'.format('SRID=-1;POINT(1 2)',vl_point_01.featureCount(),len(features_vl_point_01)))
        test_geom_01 = [f_iter.geometry() for f_iter in features_vl_point_01][0].geometry()
        print('-I-> point_01[{0}: WKT[{1}]'.format(test_geom_01.wktTypeStr(),test_geom_01.asWkt()))
        self.assertEqual(test_geom_01.wkbType(), QgsWkbTypes.Point)
        self.assertEquals((test_geom_01.x(), test_geom_01.y()), (1,2))
        del test_geom_01
        del features_vl_point_01
        del vl_point_01
        # Testing Point 02
        vl_point_02 = QgsVectorLayer(uri_name_point_02,test_name, u'ogr')
        self.assertTrue(vl_point_02.isValid())
        count_layers=len(vl_point_02.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{1}], [{2}] layers were found.'.format(self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_point_02.dataProvider().subLayers()[index]))

        count_fields=len(vl_point_02.fields())
        for index in range(count_fields):
            print(u'-I-> Field[{0}]: name[{1}] type[{2}]'.format(index, vl_point_02.fields()[index].name(), vl_point_02.fields()[index].typeName()))

        features_vl_point_02 = [f_iter for f_iter in vl_point_02.getFeatures(QgsFeatureRequest().setFilterExpression(u"fid='multiplegeomfields.0'"))]
        print('-I-> Testing type/values of a \'POINT\' EWKT[{0}] count[{1},{2}]'.format('SRID=-1;POINT(2 3)',vl_point_02.featureCount(),len(features_vl_point_02)))
        test_geom_02 = [f_iter.geometry() for f_iter in features_vl_point_02][0].geometry()
        print('-I-> point_02[{0}: WKT[{1}]'.format(test_geom_02.wktTypeStr(),test_geom_02.asWkt()))
        # TODO: resove how to retrieve 2nd geometry
        self.assertEqual(test_geom_02.wkbType(), QgsWkbTypes.Point)
        self.assertEquals((test_geom_02.x(), test_geom_02.y()), (2,3))
        del test_geom_02
        del features_vl_point_02
        del vl_point_02

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.ogr_Rauletshof_1909.gml')
        print('\n-I-> Reading GML with 3 Geometries db({0}) - test with 1 POINT, 1 LINESTRING and 1 POLYGON, 6 records'.format(datasource))
        geometry_name_point=u"geometry_point"
        uri_name_point=u'{0}|layerid=0|layername=SELECT({1})|geometryname={1}|geometryid=0|ogrtype=2'.format(datasource,geometry_name_point)
        geometry_name_linestring=u"geometry_linestring"
        uri_name_linestring=u'{0}|layerid=1|layername=SELECT({1})|geometryname={0}|geometryid=1|ogrtype=2'.format(datasource,geometry_name_linestring)
        geometry_name_polygon=u"geometry_polygon"
        uri_name_polygon=u'{0}|layerid=1|layername=SELECT({1})|geometryname={1}|geometryid=2|ogrtype=2'.format(datasource,geometry_name_polygon)
        # Testing Point
        vl_points = QgsVectorLayer(uri_name_point, test_name, u'ogr')
        self.assertTrue(vl_points.isValid())
        print(u'-I-> Provider[{0},{1}] : url[{2}]'.format( vl_points.dataProvider().name(),vl_points.dataProvider().description(), vl_points.dataProvider().dataSourceUri()))
        count_fields=len(vl_points.fields())
        print('-I-> Testing type/values of a \'POINT\' of first geometry layername[{0}] count[{1}] fields[{2}] hasGeometry[{3}]'.format(geometry_name_point,vl_points.featureCount(),count_fields,vl_points.hasGeometryType()))
        for index in range(count_fields):
            print(u'-I-> Field[{0}]: name[{1}] type[{2}]'.format(index, vl_points.fields()[index].name(), vl_points.fields()[index].typeName()))

        count_layers=len(vl_points.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{1}], [{2}] layers were found.'.format(self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_points.dataProvider().subLayers()[index]))

        features_vl_points = [f_iter for f_iter in vl_points.getFeatures(QgsFeatureRequest().setFilterExpression(u"id_admin=2"))]
        print('-I-> Testing type/values of a \'POINT\' EWKT[{0}] count[{1},{2}]'.format('SRID=3068;POINT(24725.08837877141 20759.21417724223)',vl_points.featureCount(),len(features_vl_points)))
        test_geom_point = [f_iter.geometry() for f_iter in features_vl_points][0].geometry()
        print('-I-> test_geom_point[{0}]: WKT[{0}]'.format(test_geom_point.wktTypeStr(),test_geom_point.asWkt()))
        self.assertEqual(test_geom_point.wkbType(), QgsWkbTypes.Point)
        self.assertEquals((test_geom_point.x(), test_geom_point.y()), (24725.0883787714,20759.2141772422))
        got = [(f.attribute('name'), f.attribute('notes'), f.attribute('valid_until')) for f in vl_points.getFeatures(QgsFeatureRequest().setFilterExpression("id_admin = 2"))]
        # Date field valid_until is being returned as a string from ogr. Should be tested with: QDate(1935, 10, 11)
        self.assertEqual(got, [(u'Raulet\'s Hof [1-2]', u'Bezirk 26 - Rauletshof, Friedrichswerder, Mitte', '1935/10/11')])
        del test_geom_point
        del features_vl_points
        del vl_points
        # Testing Linestring
        vl_linestrings = QgsVectorLayer(uri_name_linestring, test_name, u'ogr')
        self.assertTrue(vl_linestrings.isValid())
        features_vl_linestrings = [f_iter for f_iter in vl_linestrings.getFeatures(QgsFeatureRequest().setFilterExpression(u"id_admin=2"))]
        print('-I-> Testing type/values of a \'LINESTRING\' EWKT[{0}] count[{1},{2}]'.format('SRID=3068;POINT(24742.22135058679 20740.69355198127)',vl_linestrings.featureCount(),len(features_vl_linestrings)))
        test_geom_linestring = [f_iter.geometry() for f_iter in features_vl_linestrings][0].geometry()
        self.assertEqual(test_geom_linestring.wkbType(), QgsWkbTypes.LineString)
        test_geom_point_0 = test_geom_linestring.pointN( 0)
        self.assertEquals((test_geom_point_0.x(), test_geom_point_0.y()), (24742.2213505868,20740.6935519813))
        test_geom_name = [f_iter['name'] for f_iter in features_vl_linestrings][0]
        print('-I-> test_geom_linestring[{0}]: name[{0}] first point WKT[{0}]'.format(test_geom_linestring.wktTypeStr(),test_geom_name,test_geom_point_0.asWkt()))
        del test_geom_point_0
        del test_geom_linestring
        del vl_linestrings
        del features_vl_linestrings
        # Testing Polygon
        vl_polygons = QgsVectorLayer(uri_name_polygon, test_name, u'ogr')
        self.assertTrue(vl_polygons.isValid())
        features_vl_polygons = [f_iter_polygon for f_iter_polygon in vl_polygons.getFeatures(QgsFeatureRequest().setFilterExpression(u"id_admin=2"))]
        print('-I-> Testing type/values of a \'POLYGON\' EWKT[{0}] count[{1},{2}]'.format('SRID=3068;POINT(24706.46898365996 20778.82165136906)',vl_polygons.featureCount(),len(features_vl_polygons)))
        test_geom_polygon = [f_iter_polygon.geometry() for f_iter_polygon in features_vl_polygons][0].geometry()
        print('-I-> test_geom_polygon[{0}]: name[{0}] first point WKT[{0}]'.format(test_geom_polygon.wktTypeStr(),test_geom_name,test_geom_polygon.asWkt()))
        self.assertEqual(test_geom_polygon.wkbType(), QgsWkbTypes.Polygon)
        test_geom_exterior_ring = test_geom_polygon.exteriorRing()
        test_geom_point_1 = test_geom_exterior_ring.pointN( 0)
        self.assertEquals((test_geom_point_1.x(), test_geom_point_1.y()), (24706.46898366,20778.8216513691))
        print('-I-> test_geom_polygon[{0}]: name[{0}] first point WKT[{0}]'.format(test_geom_polygon.wktTypeStr(),test_geom_name,test_geom_point_1.asWkt()))
        del test_geom_point_1
        del test_geom_exterior_ring
        del test_geom_polygon
        del features_vl_polygons
        del vl_polygons
        print('-I-> test_04_{0} [{1}]'.format(test_name,'end of test'))

###############################################################################
# Sample kmz file included in bug report 15168
# - https://hub.qgis.org/issues/15168
# Problem caused by duplicate layername. Retrievel by layername always returned first found layer.
# - lead to correction: Use "layerid=N" instead of "layername=XYZ" for OGR sublayers
# which causes problems with gdal 2.*, where layerid logic has changed.
#  Up to Gdal 2.* each geometry was 1 layer and could be retrieved using an index
#  Starting with Gdal 2.0, each table is one layer, that can contain more than 1 geometry
#

    def test_05_OgrKMLDuplicatelayerNames(self):

        test_name='OgrKMLDuplicatelayerNames'
        print('\n-I-> test_05_{0} [{1}]'.format(test_name,'start of test'))
        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrKMLDuplicatelayerNames: Deprecated  version of gdal/ogr[{0}] qgis built with gdal[{0}] ogr_runtime_supported[{0}]'.format(self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.qgis_bugreport_15168.zk.kmz')
        print('\n-I-> Reading db({0})'.format(datasource))
        vl_bugreport_15168 = QgsVectorLayer(u'{}'.format(datasource), u'bugreport_15168', u'ogr')
        self.assertTrue(vl_bugreport_15168.isValid())
        count_layers=len(vl_bugreport_15168.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{1}], [{2}] layers were found.'.format(self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
               print(u'-I-> Sublayer[{0}] : [{1}]'.format(index,vl_bugreport_15168.dataProvider().subLayers()[index]))

        del vl_bugreport_15168

        if (count_layers ==4):
            unique_layername_1=u"Directions from Taiwan, Taichung City, 中45鄉道 to 423, Taiwan, Taichung City, Dongshi District, 中45鄉道"
            duplicate_layername=u"Directions from 423, Taiwan, 台中市東勢區慶福里 to 423, Taiwan, Taichung City, Dongshi District, 中45鄉道"
            unique_layername_6=u"Directions from 423, Taiwan, Taichung City, Dongshi District, 中45鄉道 to 423, Taiwan, Taichung City, Dongshi District, 中45鄉道"
            vl_layer_3 = QgsVectorLayer(u'{0}|layerid=3|layername={1}|featurescounted=3|geometrytype=LineString25D|ogrtype=1'.format(datasource,duplicate_layername), u'test_layer_3', u'ogr')
            self.assertTrue(vl_layer_3.isValid())
            count_fields=len(vl_layer_3.fields())
            print('-I-> Testing type/values of a \'LineString25D\' of first duplicate layername[{0}] count[{1}] fields[{2}] hasGeometry[{3}]'.format('Directions from 423, Taiwan',vl_layer_3.featureCount(),count_fields,vl_layer_3.hasGeometryType()))
            for index in range(count_fields):
                print(u'-I-> Field[{0}]: name[{1}] type[{2}]'.format(index, vl_layer_3.fields()[index].name(), vl_layer_3.fields()[index].typeName()))
            vl_layer_5 = QgsVectorLayer(u'{0}|layerid=5|layername={1}|featurescounted=3|geometrytype=LineString25D|ogrtype=1'.format(datasource,duplicate_layername), u'test_layer_5', u'ogr')
            self.assertTrue(vl_layer_5.isValid())
            vl_layer_1 = QgsVectorLayer(u'{0}|layerid=1|layername={1}|featurescounted=3|geometrytype=LineString25D|ogrtype=0'.format(datasource,unique_layername_1), u'test_layer_1', u'ogr')
            self.assertTrue(vl_layer_1.isValid())
            vl_layer_6 = QgsVectorLayer(u'{0}|layerid=6|layername={1}|featurescounted=3|geometrytype=LineString25D|ogrtype=0'.format(datasource,unique_layername_6), u'test_layer_6', u'ogr')
            self.assertTrue(vl_layer_6.isValid())
            del vl_layer_1
            del vl_layer_6
            # alternative way to retrieve geometry without need of Filter:
            # features_vl_layer_3 = next(vl_layer_3.getFeatures())
            # geom_layer_3 = features_vl_layer_3.geometry()
            features_vl_layer_3 = [f_iter for f_iter in vl_layer_3.getFeatures(QgsFeatureRequest().setFilterExpression(u"Name='{0}'".format(duplicate_layername)))]
            geom_layer_3 = [f_iter.geometry() for f_iter in features_vl_layer_3][0].geometry()
            self.assertEqual(geom_layer_3.wkbType(), QgsWkbTypes.LineString25D)
            features_vl_layer_5 = [f_iter for f_iter in vl_layer_5.getFeatures(QgsFeatureRequest().setFilterExpression(u"Name='{0}'".format(duplicate_layername)))]
            geom_layer_5 = [f_iter.geometry() for f_iter in features_vl_layer_5][0].geometry()
            self.assertEqual(geom_layer_5.wkbType(), QgsWkbTypes.LineString25D)
            print('-I-> Checking  first point layername[{0}]\n\t geom_layer_3 x[{1:f}] y[{2:f}] z[{3:f}] \n\t geom_layer_5 x[{4:f}] y[{5:f}] z[{6:f}]'.format('Directions from 423, Taiwan',
                              geom_layer_3.pointN(0).x(), geom_layer_3.pointN(0).y(),geom_layer_3.pointN(0).z(),geom_layer_5.pointN(0).x(), geom_layer_5.pointN(0).y(),geom_layer_5.pointN(0).z()))
            self.assertNotEqual((geom_layer_3.pointN(0).x(), geom_layer_3.pointN(0).y(),geom_layer_3.pointN(0).z()),
                                              (geom_layer_5.pointN(0).x(), geom_layer_5.pointN(0).y(),geom_layer_5.pointN(0).z()))
            print('-I-> Checking geometries of duplicate layername[{0}]\n\t geom_layer_3[{1}]'.format('Directions from 423, Taiwan',geom_layer_3.wktTypeStr()))
            layer_3_num_points=geom_layer_3.numPoints()
            print('-I-> Checking geometries of duplicate layername[{0}]\n\t points_layer_3[{1}] '.format('Directions from 423, Taiwan', layer_3_num_points))
            layer_5_num_points=geom_layer_5.numPoints()
            print('-I-> Checking geometries of duplicate layername[{0}]\n\t points_layer_5[{1}] '.format('Directions from 423, Taiwan', layer_5_num_points))
            self.assertNotEqual(layer_3_num_points,layer_5_num_points)
            print('-I-> Checking geometries of duplicate layername[{0}]\n\t points_layer_3[{1}] points_layer_5[{2}] \n\t\t the point count should not be equal'.format('Directions from 423, Taiwan', layer_3_num_points,layer_5_num_points))
            layer_3_num_points-=1
            layer_5_num_points-=1
            print('-I-> Checking  last point layername[{0}]\n\t geom_layer_3 point[{1}] x[{2:f}] y[{3:f}] z[{4:f}] \n\t geom_layer_5 point[{5}] x[{6:f}] y[{7:f}] z[{8:f}]'.format('Directions from 423, Taiwan',
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

        print('-I-> test_05_{0} [{1}]'.format(test_name,'end of test'))

###############################################################################
# Test sqlite Integer64
# - created [as 'sqlite_test.db'] and filled with gdal autotest/ogr/ogr_sql_sqlite.py ogr_sqlite_1, 2, 11, 12, 13, 14, 15 and 16
# contains 13 tables, 7 layers (skipping: geometry_columns, spatial_ref_sys, a_layer, wgs84layer, wgs84layer_approx, testtypes)
# Table 'tpoly' contains 2 fields with BIGINT (Integer64) which will be tested

    def test_06_OgrInteger64(self):

        test_name='OgrInteger64'
        print('\n-I-> test_06_{0} [{1}]'.format(test_name,'start of test'))
        if (self.ogr_runtime_supported < 1):
            print('-I-> OgrInteger64: Deprecated  version of gdal/ogr[{0}] qgis built with gdal[{0}] ogr_runtime_supported[{0}]'.format(self.gdal_version_num, self.gdal_build_version,self.ogr_runtime_supported))
            return

        datasource = os.path.join(TEST_DATA_DIR, 'provider/gdal_220.autotest.ogr_sqlite_test.db')
        print('\n-I-> Reading db({0})'.format(datasource))
        vl_sqlite_test = QgsVectorLayer(u'{}'.format(datasource), u'sqlite_test', u'ogr')
        self.assertTrue(vl_sqlite_test.isValid())
        count_layers=len(vl_sqlite_test.dataProvider().subLayers())
        print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{1}], [{2}] layers were found.'.format(self.gdal_version_num, self.gdal_build_version,count_layers))
        for index in range(count_layers):
                print(u'-I-> Sublayer[{0}] : [{1}]'.format(index, vl_sqlite_test.dataProvider().subLayers()[index]))

        del vl_sqlite_test

        if (count_layers == 7):
            vl_layer_tpoly = QgsVectorLayer(u'{0}|layerid=1|layername=tpoly|featurescounted=18|geometrytype=Polygon|ogrtype=0'.format(datasource), u'test_layer_tpoly', u'ogr')
            self.assertTrue(vl_layer_tpoly.isValid())
            count_fields=len(vl_layer_tpoly.fields())
            print('-I-> Testing type/values of a \'Integer64\' of the layername[{0}] count[{1}] fields[{2}] hasGeometry[{3}]'.format('tpoly',vl_layer_tpoly.featureCount(),count_fields,vl_layer_tpoly.hasGeometryType()))
            for index in range(count_fields):
                print(u'-I-> Field[{0}]: name[{1}] type[{2}]'.format(index, vl_layer_tpoly.fields()[index].name(), vl_layer_tpoly.fields()[index].typeName()))

            # Integer64 [Max-Integer32: 2147483647]  1234567890123/2147483647=574,890473251 ; 2147483647*0.890473251=1912276744,613426397
            gdal_2_value=1234567890123
            gdal_2_value_update=1851851835185
            gdal_1_value=1912276171
            max_integer_32=2147483647
            print('-I-> Retrieving record 7 of layername[{0}], checking returned area and int64 test values [{1:f},{2}]'.format('tpoly',268597.625, gdal_2_value))
            got = [(f.attribute('ogc_fid'), f.attribute('area'), f.attribute('int64')) for f in vl_layer_tpoly.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 7"))]
            if (self.gdal_version_num >= GDAL_COMPUTE_VERSION(2, 0, 0)):
                self.assertEqual(got, [(7, 268597.625, gdal_2_value)])
            else:
                print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{1}], result will be [{2}] instead of [{3}].'.format(self.gdal_version_num, self.gdal_build_version,gdal_1_value,gdal_2_value))
                self.assertEqual(got, [(7, 268597.625, gdal_1_value)])

            self.assertTrue(vl_layer_tpoly.startEditing())
            self.assertTrue(vl_layer_tpoly.dataProvider().changeAttributeValues({8: {5: gdal_2_value_update}}))
            print('-I-> Retrieving record 8 of layername[%s], checking returned area and int64 test values [{1:f},{2}]'.format('tpoly',1634833.375, gdal_2_value))
            got = [(f.attribute('ogc_fid'), f.attribute('area'), f.attribute('int64')) for f in vl_layer_tpoly.getFeatures(QgsFeatureRequest().setFilterExpression("ogc_fid = 8"))]
            if (self.gdal_version_num >= GDAL_COMPUTE_VERSION(2, 0, 0)):
                self.assertEqual(got, [(8, 1634833.375, gdal_2_value_update)])
            else:
               print('-I-> Using version of gdal/ogr[{0}] qgis built with gdal[{1}], result will be [{2}] instead of [{3}].'.format(self.gdal_version_num, self.gdal_build_version,720930609,gdal_2_value_update))

            print('-I-> rollBack({0})'.format('tpoly'))
            vl_layer_tpoly.rollBack(True)
            del vl_layer_tpoly

        print('-I-> test_06_{0} [{1}]'.format(test_name,'end of test'))

if __name__ == '__main__':
    unittest.main()
