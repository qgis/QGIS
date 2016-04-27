# -*- coding: utf-8 -*-
"""QGIS Unit tests for the OGR/GPKG provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '2016-04-21'
__copyright__ = 'Copyright 2016, Even Rouault'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os
import tempfile
import shutil
import glob
from osgeo import gdal, ogr

from qgis.core import QgsVectorLayer, QgsFeature, QgsGeometry, QgsFeatureRequest
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


def GDAL_COMPUTE_VERSION(maj, min, rev):
    return ((maj) * 1000000 + (min) * 10000 + (rev) * 100)


class TestPyQgsOGRProviderGpkg(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # Create test layer
        cls.basetestpath = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        shutil.rmtree(cls.basetestpath, True)

    def testSingleToMultiPolygonPromotion(self):

        tmpfile = os.path.join(self.basetestpath, 'testSingleToMultiPolygonPromotion.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbMultiPolygon)
        ds = None

        vl = QgsVectorLayer(u'{}|layerid=0'.format(tmpfile), u'test', u'ogr')
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('POLYGON ((0 0,0 1,1 1,0 0))'))
        vl.dataProvider().addFeatures([f])
        got = [f for f in vl.getFeatures()][0]
        got_geom = got.geometry()
        reference = QgsGeometry.fromWkt('MultiPolygon (((0 0, 0 1, 1 1, 0 0)))')
        # The geometries must be binarily identical
        self.assertEqual(got_geom.asWkb(), reference.asWkb(), 'Expected {}, got {}'.format(reference.exportToWkt(), got_geom.exportToWkt()))

    @unittest.expectedFailure(int(gdal.VersionInfo('VERSION_NUM')) < GDAL_COMPUTE_VERSION(2, 0, 0))
    def testCurveGeometryType(self):

        tmpfile = os.path.join(self.basetestpath, 'testCurveGeometryType.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbCurvePolygon)
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile), u'test', u'ogr')
        self.assertEqual(vl.dataProvider().subLayers(), [u'0:test:0:CurvePolygon'])
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('POLYGON ((0 0,0 1,1 1,0 0))'))
        vl.dataProvider().addFeatures([f])
        got = [f for f in vl.getFeatures()][0]
        got_geom = got.geometry()
        reference = QgsGeometry.fromWkt('CurvePolygon (((0 0, 0 1, 1 1, 0 0)))')
        # The geometries must be binarily identical
        self.assertEqual(got_geom.asWkb(), reference.asWkb(), 'Expected {}, got {}'.format(reference.exportToWkt(), got_geom.exportToWkt()))

    def testFidSupport(self):

        # We do not use @unittest.expectedFailure since the test might actually succeed
        # on Linux 64bit with GDAL 1.11, where "long" is 64 bit...
        # GDAL 2.0 is guaranteed to properly support it on all platforms
        version_num = int(gdal.VersionInfo('VERSION_NUM'))
        if version_num < GDAL_COMPUTE_VERSION(2, 0, 0):
            return

        tmpfile = os.path.join(self.basetestpath, 'testFidSupport.gpkg')
        ds = ogr.GetDriverByName('GPKG').CreateDataSource(tmpfile)
        lyr = ds.CreateLayer('test', geom_type=ogr.wkbPoint)
        lyr.CreateField(ogr.FieldDefn('strfield', ogr.OFTString))
        f = ogr.Feature(lyr.GetLayerDefn())
        f.SetFID(12)
        f.SetField(0, 'foo')
        lyr.CreateFeature(f)
        f = None
        ds = None

        vl = QgsVectorLayer(u'{}'.format(tmpfile), u'test', u'ogr')
        self.assertEqual(len(vl.fields()), 2)
        got = [(f.attribute('fid'), f.attribute('strfield')) for f in vl.getFeatures()]
        self.assertEqual(got, [(12, 'foo')])

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

if __name__ == '__main__':
    unittest.main()
