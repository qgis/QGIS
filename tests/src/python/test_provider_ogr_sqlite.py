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
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
import tempfile
import shutil
from osgeo import gdal, ogr

from qgis.core import QgsVectorLayer, QgsFeature, QgsFeatureRequest, QgsFieldConstraints, NULL
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QDate, QTime, QDateTime

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

        # We do not use @unittest.expectedFailure since the test might actually succeed
        # on Linux 64bit with GDAL 1.11, where "long" is 64 bit...
        # GDAL 2.0 is guaranteed to properly support it on all platforms
        version_num = int(gdal.VersionInfo('VERSION_NUM'))
        if version_num < GDAL_COMPUTE_VERSION(2, 0, 0):
            return

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
        self.assertTrue(vl.dataProvider().changeAttributeValues({9876543210: {0: 12, 1: 'baw'}}))

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

        self.assertFalse(vl.dataProvider().fieldConstraints(0) & QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(1) & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.dataProvider().fieldConstraints(2) & QgsFieldConstraints.ConstraintNotNull)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertFalse(fields.at(0).constraints().constraints() & QgsFieldConstraints.ConstraintNotNull)
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
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(1)
        f.SetField(0, 1)
        f.SetField(1, 12)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(2)
        f.SetField(0, 1)
        f.SetField(1, 13)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(3)
        f.SetField(0, 2)
        f.SetField(1, 14)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(4)
        f.SetField(0, 2)
        f.SetField(1, 15)
        lyr.CreateFeature(f)
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(5)
        f.SetField(0, 2)
        f.SetField(1, 16)
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer(tmpfile + "|subset=type=2", 'test', 'ogr')
        self.assertTrue(vl.isValid())
        self.assertTrue(vl.fields().at(vl.fields().count() - 1).name() == "orig_ogc_fid")

        req = QgsFeatureRequest()
        req.setFilterExpression("value=16")
        it = vl.getFeatures(req)
        f = QgsFeature()
        self.assertTrue(it.nextFeature(f))
        self.assertTrue(f.id() == 5)

        # Ensure that orig_ogc_fid is still retrieved even if attribute subset is passed
        req = QgsFeatureRequest()
        req.setSubsetOfAttributes([])
        it = vl.getFeatures(req)
        ids = []
        while it.nextFeature(f):
            ids.append(f.id())
        self.assertTrue(len(ids) == 3)
        self.assertTrue(3 in ids)
        self.assertTrue(4 in ids)
        self.assertTrue(5 in ids)

        # Check that subset string is correctly set on reload
        vl.reload()
        self.assertTrue(vl.fields().at(vl.fields().count() - 1).name() == "orig_ogc_fid")


if __name__ == '__main__':
    unittest.main()
