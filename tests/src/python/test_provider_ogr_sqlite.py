# -*- coding: utf-8 -*-
"""QGIS Unit tests for the OGR/GPKG provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '2016-06-01'
__copyright__ = 'Copyright 2016, Even Rouault'

import qgis  # NOQA

import os
import tempfile
import shutil
import hashlib
from osgeo import gdal, ogr

from qgis.core import (QgsVectorLayer,
                       QgsFeature,
                       QgsFeatureRequest,
                       QgsFieldConstraints,
                       QgsPointXY,
                       NULL,
                       QgsRectangle,
                       QgsVectorDataProvider)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant, QByteArray

start_app()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


class TestPyQgsOGRProviderSqlite(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        shutil.rmtree(cls.basetestpath, True)

    def testFidSupport(self):
        tmpfile = os.path.join(self.basetestpath, 'testFidSupport.sqlite')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint, options=['FID=fid'])
        lyr.CreateField(ogr.FieldDefn('strfield', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('intfield', ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(12)
        f.SetField(0, 'foo')
        f.SetField(1, 123)
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer('{}'.format(tmpfile), 'test', 'ogr')
        self.assertEqual(len(vl.fields()), 3)
        got = [(f.attribute('fid'), f.attribute('strfield'), f.attribute('intfield')) for f in vl.getFeatures()]
        self.assertEqual(got, [(12, 'foo', 123)])

        got = [(f.attribute('fid'), f.attribute('strfield')) for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression("strfield = 'foo'"))]
        self.assertEqual(got, [(12, 'foo')])

        got = [(f.attribute('fid'), f.attribute('strfield')) for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression("fid = 12"))]
        self.assertEqual(got, [(12, 'foo')])

        result = [f['strfield'] for f in vl.dataProvider().getFeatures(QgsFeatureRequest().setSubsetOfAttributes(['strfield'], vl.dataProvider().fields()))]
        self.assertEqual(result, ['foo'])

        result = [f['fid'] for f in vl.dataProvider().getFeatures(QgsFeatureRequest().setSubsetOfAttributes(['fid'], vl.dataProvider().fields()))]
        self.assertEqual(result, [12])

        # Test that when the 'fid' field is not set, regular insertion is done
        f = QgsFeature()
        f.setFields(vl.fields())
        f.setAttributes([None, 'automatic_id'])
        (res, out_f) = vl.dataProvider().addFeatures([f])
        self.assertEqual(out_f[0].id(), 13)
        self.assertEqual(out_f[0].attribute('fid'), 13)
        self.assertEqual(out_f[0].attribute('strfield'), 'automatic_id')

        # Test that when the 'fid' field is set, it is really used to set the id
        f = QgsFeature()
        f.setFields(vl.fields())
        f.setAttributes([9876543210, 'bar'])
        (res, out_f) = vl.dataProvider().addFeatures([f])
        self.assertEqual(out_f[0].id(), 9876543210)
        self.assertEqual(out_f[0].attribute('fid'), 9876543210)
        self.assertEqual(out_f[0].attribute('strfield'), 'bar')

        got = [(f.attribute('fid'), f.attribute('strfield')) for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression("fid = 9876543210"))]
        self.assertEqual(got, [(9876543210, 'bar')])

        self.assertTrue(vl.dataProvider().changeAttributeValues({9876543210: {1: 'baz'}}))

        got = [(f.attribute('fid'), f.attribute('strfield')) for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression("fid = 9876543210"))]
        self.assertEqual(got, [(9876543210, 'baz')])

        self.assertTrue(vl.dataProvider().changeAttributeValues({9876543210: {0: 9876543210, 1: 'baw'}}))

        got = [(f.attribute('fid'), f.attribute('strfield')) for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression("fid = 9876543210"))]
        self.assertEqual(got, [(9876543210, 'baw')])

        # Not allowed: changing the fid regular field
        self.assertFalse(vl.dataProvider().changeAttributeValues({9876543210: {0: 12, 1: 'baw'}}))

        got = [(f.attribute('fid'), f.attribute('strfield')) for f in vl.getFeatures(QgsFeatureRequest().setFilterExpression("fid = 9876543210"))]
        self.assertEqual(got, [(9876543210, 'baw')])

        # Cannot delete fid
        self.assertFalse(vl.dataProvider().deleteAttributes([0]))

        # Delete first "genuine" attribute
        self.assertTrue(vl.dataProvider().deleteAttributes([1]))

        got = [(f.attribute('fid'), f.attribute('intfield')) for f in vl.dataProvider().getFeatures(QgsFeatureRequest().setFilterExpression("fid = 12"))]
        self.assertEqual(got, [(12, 123)])

    def testNotNullConstraint(self):
        """ test detection of not null constraint on OGR layer """

        tmpfile = os.path.join(self.basetestpath, 'testNotNullConstraint.sqlite')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint, options=['FID=fid'])
        lyr.CreateField(ogr.FieldDefn('field1', ogr.OFTInteger))
        fld2 = ogr.FieldDefn('field2', ogr.OFTInteger)
        fld2.SetNullable(False)
        lyr.CreateField(fld2)
        ds = None

        vl = QgsVectorLayer('{}'.format(tmpfile), 'test', 'ogr')
        self.assertTrue(vl.isValid())

        # test some bad indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1), QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0) & QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(1) & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.dataProvider().fieldConstraints(2) & QgsFieldConstraints.ConstraintNotNull)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(fields.at(1).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(fields.at(2).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(fields.at(2).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull), QgsFieldConstraints.ConstraintOriginProvider)

    def testDefaultValues(self):
        """ test detection of defaults on OGR layer """

        tmpfile = os.path.join(self.basetestpath, 'testDefaults.sqlite')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint, options=['FID=fid'])
        lyr.CreateField(ogr.FieldDefn('field1', ogr.OFTInteger))
        fld2 = ogr.FieldDefn('field2', ogr.OFTInteger)
        fld2.SetDefault('5')
        lyr.CreateField(fld2)
        fld3 = ogr.FieldDefn('field3', ogr.OFTString)
        fld3.SetDefault("'some ''default'")
        lyr.CreateField(fld3)
        fld4 = ogr.FieldDefn('field4', ogr.OFTDate)
        fld4.SetDefault("CURRENT_DATE")
        lyr.CreateField(fld4)
        fld5 = ogr.FieldDefn('field5', ogr.OFTTime)
        fld5.SetDefault("CURRENT_TIME")
        lyr.CreateField(fld5)
        fld6 = ogr.FieldDefn('field6', ogr.OFTDateTime)
        fld6.SetDefault("CURRENT_TIMESTAMP")
        lyr.CreateField(fld6)

        ds = None

        vl = QgsVectorLayer('{}'.format(tmpfile), 'test', 'ogr')
        self.assertTrue(vl.isValid())

        # test some bad indexes
        self.assertFalse(vl.dataProvider().defaultValue(-1))
        self.assertFalse(vl.dataProvider().defaultValue(1001))

        # test default
        self.assertEqual(vl.dataProvider().defaultValue(1), NULL)
        self.assertEqual(vl.dataProvider().defaultValue(2), 5)
        self.assertEqual(vl.dataProvider().defaultValue(3), "some 'default")
        self.assertEqual(vl.dataProvider().defaultValue(4), QDate.currentDate())
        # time may pass, so we allow 1 second difference here
        self.assertTrue(vl.dataProvider().defaultValue(5).secsTo(QTime.currentTime()) < 1)
        self.assertTrue(vl.dataProvider().defaultValue(6).secsTo(QDateTime.currentDateTime()) < 1)

    def testSubsetStringFids(self):
        """
          - tests that feature ids are stable even if a subset string is set
          - tests that the subset string is correctly set on the ogr layer event when reloading the data source (issue #17122)
        """

        tmpfile = os.path.join(self.basetestpath, 'subsetStringFids.sqlite')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint, options=['FID=fid'])
        lyr.CreateField(ogr.FieldDefn('type', ogr.OFTInteger))
        lyr.CreateField(ogr.FieldDefn('value', ogr.OFTInteger))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(0)
        f.SetField(0, 1)
        f.SetField(1, 11)
        f.SetGeometry(ogr.CreateGeometryFromWkt('Point (0 0)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(1)
        f.SetField(0, 1)
        f.SetField(1, 12)
        f.SetGeometry(ogr.CreateGeometryFromWkt('Point (1 1)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(2)
        f.SetField(0, 1)
        f.SetField(1, 13)
        f.SetGeometry(ogr.CreateGeometryFromWkt('Point (2 2)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(3)
        f.SetField(0, 2)
        f.SetField(1, 14)
        f.SetGeometry(ogr.CreateGeometryFromWkt('Point (3 3)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(4)
        f.SetField(0, 2)
        f.SetField(1, 15)
        f.SetGeometry(ogr.CreateGeometryFromWkt('Point (4 4)'))
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(5)
        f.SetField(0, 2)
        f.SetField(1, 16)
        f.SetGeometry(ogr.CreateGeometryFromWkt('Point (5 5)'))
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer(tmpfile, 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertEqual([f.name() for f in vl.fields()], ['fid', 'type', 'value'])
        original_fields = vl.fields()

        vl = QgsVectorLayer(tmpfile + "|subset=type=2", 'test', 'ogr')
        self.assertTrue(vl.isValid())

        def run_checks():
            self.assertEqual([f.name() for f in vl.fields()], ['fid', 'type', 'value'])

            # expression
            req = QgsFeatureRequest()
            req.setFilterExpression("value=16")
            it = vl.getFeatures(req)
            f = QgsFeature()
            self.assertTrue(it.nextFeature(f))
            self.assertEqual(f.id(), 5)
            self.assertEqual(f.attributes(), [5, 2, 16])
            self.assertEqual([field.name() for field in f.fields()], ['fid', 'type', 'value'])
            self.assertEqual(f.geometry().asWkt(), 'Point (5 5)')

            # filter fid
            req = QgsFeatureRequest()
            req.setFilterFid(5)
            it = vl.getFeatures(req)
            f = QgsFeature()
            self.assertTrue(it.nextFeature(f))
            self.assertEqual(f.id(), 5)
            self.assertEqual(f.attributes(), [5, 2, 16])
            self.assertEqual([field.name() for field in f.fields()], ['fid', 'type', 'value'])
            self.assertEqual(f.geometry().asWkt(), 'Point (5 5)')

            # filter fids
            req = QgsFeatureRequest()
            req.setFilterFids([5])
            it = vl.getFeatures(req)
            f = QgsFeature()
            self.assertTrue(it.nextFeature(f))
            self.assertEqual(f.id(), 5)
            self.assertEqual(f.attributes(), [5, 2, 16])
            self.assertEqual([field.name() for field in f.fields()], ['fid', 'type', 'value'])
            self.assertEqual(f.geometry().asWkt(), 'Point (5 5)')

            # check with subset of attributes
            req = QgsFeatureRequest()
            req.setFilterFids([5])
            req.setSubsetOfAttributes([2])
            it = vl.getFeatures(req)
            f = QgsFeature()
            self.assertTrue(it.nextFeature(f))
            self.assertEqual(f.id(), 5)
            self.assertEqual(f.attributes()[2], 16)
            self.assertEqual([field.name() for field in f.fields()], ['fid', 'type', 'value'])
            self.assertEqual(f.geometry().asWkt(), 'Point (5 5)')

            # filter rect and expression
            req = QgsFeatureRequest()
            req.setFilterExpression("value=16 or value=14")
            req.setFilterRect(QgsRectangle(4.5, 4.5, 5.5, 5.5))
            it = vl.getFeatures(req)
            f = QgsFeature()
            self.assertTrue(it.nextFeature(f))
            self.assertEqual(f.id(), 5)
            self.assertEqual(f.attributes(), [5, 2, 16])
            self.assertEqual([field.name() for field in f.fields()], ['fid', 'type', 'value'])
            self.assertEqual(f.geometry().asWkt(), 'Point (5 5)')

            # filter rect and fids
            req = QgsFeatureRequest()
            req.setFilterFids([3, 5])
            req.setFilterRect(QgsRectangle(4.5, 4.5, 5.5, 5.5))
            it = vl.getFeatures(req)
            f = QgsFeature()
            self.assertTrue(it.nextFeature(f))
            self.assertEqual(f.id(), 5)
            self.assertEqual(f.attributes(), [5, 2, 16])
            self.assertEqual([field.name() for field in f.fields()], ['fid', 'type', 'value'])
            self.assertEqual(f.geometry().asWkt(), 'Point (5 5)')

            # Ensure that orig_ogc_fid is still retrieved even if attribute subset is passed
            req = QgsFeatureRequest()
            req.setSubsetOfAttributes([])
            it = vl.getFeatures(req)
            ids = []
            geoms = {}
            while it.nextFeature(f):
                ids.append(f.id())
                geoms[f.id()] = f.geometry().asWkt()
            self.assertCountEqual(ids, [3, 4, 5])
            self.assertEqual(geoms, {3: 'Point (3 3)', 4: 'Point (4 4)', 5: 'Point (5 5)'})

        run_checks()
        # Check that subset string is correctly set on reload
        vl.reload()
        run_checks()

    def test_SplitFeature(self):
        """Test sqlite feature can be split"""
        tmpfile = os.path.join(self.basetestpath, 'testGeopackageSplitFeatures.sqlite')
        ds = ogr.GetDriverByName('SQlite').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POLYGON ((0 0,0 1,1 1,1 0,0 0))'))
        lyr.CreateFeature(f)
        f = None
        ds = None

        layer = QgsVectorLayer('{}'.format(tmpfile) + "|layername=" + "test", 'test', 'ogr')

        # Check that pk field has unique constraint
        fields = layer.fields()
        pkfield = fields.at(0)
        self.assertTrue(pkfield.constraints().constraints() & QgsFieldConstraints.ConstraintUnique)

        self.assertTrue(layer.isValid())
        self.assertTrue(layer.isSpatial())
        self.assertEqual([f for f in layer.getFeatures()][0].geometry().asWkt(), 'Polygon ((0 0, 0 1, 1 1, 1 0, 0 0))')
        layer.startEditing()
        self.assertEqual(layer.splitFeatures([QgsPointXY(0.5, 0), QgsPointXY(0.5, 1)], 0), 0)
        self.assertTrue(layer.commitChanges())
        self.assertEqual(layer.featureCount(), 2)

        layer = QgsVectorLayer('{}'.format(tmpfile) + "|layername=" + "test", 'test', 'ogr')
        self.assertEqual(layer.featureCount(), 2)
        self.assertEqual([f for f in layer.getFeatures()][0].geometry().asWkt(), 'Polygon ((0.5 0, 0.5 1, 1 1, 1 0, 0.5 0))')
        self.assertEqual([f for f in layer.getFeatures()][1].geometry().asWkt(), 'Polygon ((0.5 1, 0.5 0, 0 0, 0 1, 0.5 1))')

    def testBlob(self):
        """
        Test binary blob field
        """
        tmpfile = os.path.join(self.basetestpath, 'binaryfield.sqlite')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint, options=['FID=fid'])
        lyr.CreateField(ogr.FieldDefn('strfield', ogr.OFTString))
        lyr.CreateField(ogr.FieldDefn('intfield', ogr.OFTInteger))
        lyr.CreateField(ogr.FieldDefn('binfield', ogr.OFTBinary))
        lyr.CreateField(ogr.FieldDefn('binfield2', ogr.OFTBinary))
        f = None
        ds = None

        vl = QgsVectorLayer(tmpfile)
        self.assertTrue(vl.isValid())

        fields = vl.fields()
        bin1_field = fields[fields.lookupField('binfield')]
        self.assertEqual(bin1_field.type(), QVariant.ByteArray)
        self.assertEqual(bin1_field.typeName(), 'Binary')
        bin2_field = fields[fields.lookupField('binfield2')]
        self.assertEqual(bin2_field.type(), QVariant.ByteArray)
        self.assertEqual(bin2_field.typeName(), 'Binary')

        dp = vl.dataProvider()
        f = QgsFeature(fields)
        bin_1 = b'xxx'
        bin_2 = b'yyy'
        bin_val1 = QByteArray(bin_1)
        bin_val2 = QByteArray(bin_2)
        f.setAttributes([1, 'str', 100, bin_val1, bin_val2])
        self.assertTrue(dp.addFeature(f))

        f2 = next(dp.getFeatures())
        self.assertEqual(f2.attributes(), [1, 'str', 100, QByteArray(bin_1), QByteArray(bin_2)])

        bin_3 = b'zzz'
        bin_4 = b'aaa'
        bin_val3 = QByteArray(bin_3)
        bin_val4 = QByteArray(bin_4)
        self.assertTrue(dp.changeAttributeValues({f2.id(): {fields.lookupField('intfield'): 200,
                                                            fields.lookupField('binfield'): bin_val4,
                                                            fields.lookupField('binfield2'): bin_val3}}))

        f5 = next(dp.getFeatures())
        self.assertEqual(f5.attributes(), [1, 'str', 200, QByteArray(bin_4), QByteArray(bin_3)])

    def testUniqueValuesOnFidColumn(self):
        """Test regression #21311 OGR provider returns an empty set for sqlite uniqueValues"""

        tmpfile = os.path.join(self.basetestpath, 'testSQLiteUniqueValuesOnFidColumn.db')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        lyr.CreateField(ogr.FieldDefn('str_field', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POLYGON ((0 0,0 1,1 1,1 0,0 0))'))
        f.SetField('str_field', 'one')
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetGeometry(ogr.CreateGeometryFromWkt('POLYGON ((0 0,0 2,2 2,2 0,0 0))'))
        f.SetField('str_field', 'two')
        lyr.CreateFeature(f)
        f = None
        ds = None
        vl1 = QgsVectorLayer(tmpfile)
        self.assertTrue(vl1.isValid())
        self.assertEqual(vl1.uniqueValues(0), {1, 2})
        self.assertEqual(vl1.uniqueValues(1), {'one', 'two'})

    def testSpatialIndexCapability(self):
        """ Test https://github.com/qgis/QGIS/issues/44513 """

        tmpfile = os.path.join(self.basetestpath, 'testSpatialIndexCapability.db')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(tmpfile)
        ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        ds = None

        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile), 'test', 'ogr')
        caps = vl.dataProvider().capabilities()
        self.assertFalse(caps & QgsVectorDataProvider.CreateSpatialIndex)

    def testSpatialIndexCapabilitySpatialite(self):
        """ Test https://github.com/qgis/QGIS/issues/44513 """

        tmpfile = os.path.join(self.basetestpath, 'testSpatialIndexCapabilitySpatialite.db')
        ds = ogr.GetDriverByName('SQLite').CreateDataSource(tmpfile, options=['SPATIALITE=YES'])
        ds.CreateLayer('test', geom_type=ogr.wkbPolygon)
        ds = None

        vl = QgsVectorLayer('{}|layerid=0'.format(tmpfile), 'test', 'ogr')
        caps = vl.dataProvider().capabilities()
        self.assertTrue(caps & QgsVectorDataProvider.CreateSpatialIndex)


if __name__ == '__main__':
    unittest.main()
