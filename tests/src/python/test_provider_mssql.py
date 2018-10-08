# -*- coding: utf-8 -*-
"""QGIS Unit tests for the MS SQL provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2015-12-07'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.core import (QgsSettings,
                       QgsVectorLayer,
                       QgsFeatureRequest,
                       QgsFeature,
                       QgsWkbTypes,
                       QgsField,
                       QgsGeometry,
                       QgsPointXY,
                       QgsRectangle,
                       NULL,
                       QgsVectorLayerExporter,
                       QgsCoordinateReferenceSystem)

from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant

from utilities import unitTestDataPath
from qgis.testing import start_app, unittest
from providertestbase import ProviderTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsMssqlProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = "service='Driver={ODBC Driver 13 for SQL Server};server=127.0.0.1;uid=SA;pwd=<YourStrong!Passw0rd>' user='SA' password='<YourStrong!Passw0rd>'"

        if 'QGIS_MSSQLTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_MSSQLTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'mssql')
        assert(cls.vl.isValid())
        cls.source = cls.vl.dataProvider()
        cls.poly_vl = QgsVectorLayer(
            cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test', 'mssql')
        assert(cls.poly_vl.isValid())
        cls.poly_provider = cls.poly_vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def partiallyCompiledFilters(self):
        filters = set([
            'name ILIKE \'QGIS\'',
            'name = \'Apple\'',
            'name = \'apple\'',
            'name LIKE \'Apple\'',
            'name LIKE \'aPple\'',
            'name ILIKE \'aPple\'',
            'name ILIKE \'%pp%\'',
            '"name" || \' \' || "name" = \'Orange Orange\'',
            '"name" || \' \' || "cnt" = \'Orange 100\'',
            '"name"="name2"',
            'lower(name) = \'apple\'',
            'upper(name) = \'APPLE\'',
            'name = trim(\'   Apple   \')'
        ])
        return filters

    def uncompiledFilters(self):
        filters = set([
            '"name" IS NULL',
            '"name" IS NOT NULL',
            '"name" NOT LIKE \'Ap%\'',
            '"name" NOT ILIKE \'QGIS\'',
            '"name" NOT ILIKE \'pEAR\'',
            'name <> \'Apple\'',
            '"name" <> \'apple\'',
            '(name = \'Apple\') is not null',
            '\'x\' || "name" IS NOT NULL',
            '\'x\' || "name" IS NULL',
            '"name" ~ \'[OP]ra[gne]+\'',
            'false and NULL',
            'true and NULL',
            'NULL and false',
            'NULL and true',
            'NULL and NULL',
            'false or NULL',
            'true or NULL',
            'NULL or false',
            'NULL or true',
            'NULL or NULL',
            'not null',
            'not name = \'Apple\'',
            'not name IS NULL',
            'not name = \'Apple\' or name = \'Apple\'',
            'not name = \'Apple\' or not name = \'Apple\'',
            'not name = \'Apple\' and pk = 4',
            'not name = \'Apple\' and not pk = 4',
            'pk = coalesce(NULL,3,4)',
            'x($geometry) < -70',
            'y($geometry) > 70',
            'xmin($geometry) < -70',
            'ymin($geometry) > 70',
            'xmax($geometry) < -70',
            'ymax($geometry) > 70',
            'disjoint($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
            'intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))',
            'contains(geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'),$geometry)',
            'distance($geometry,geom_from_wkt( \'Point (-70 70)\')) > 7',
            'intersects($geometry,geom_from_gml( \'<gml:Polygon srsName="EPSG:4326"><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-72.2,66.1 -65.2,66.1 -65.2,72.0 -72.2,72.0 -72.2,66.1</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon>\'))',
            'x($geometry) < -70',
            'y($geometry) > 79',
            'xmin($geometry) < -70',
            'ymin($geometry) < 76',
            'xmax($geometry) > -68',
            'ymax($geometry) > 80',
            'area($geometry) > 10',
            'perimeter($geometry) < 12',
            'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\')) = \'FF2FF1212\'',
            'relate($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'), \'****F****\')',
            'crosses($geometry,geom_from_wkt( \'Linestring (-68.2 82.1, -66.95 82.1, -66.95 79.05)\'))',
            'overlaps($geometry,geom_from_wkt( \'Polygon ((-68.2 82.1, -66.95 82.1, -66.95 79.05, -68.2 79.05, -68.2 82.1))\'))',
            'within($geometry,geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
            'overlaps(translate($geometry,-1,-1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
            'overlaps(buffer($geometry,1),geom_from_wkt( \'Polygon ((-75.1 76.1, -75.1 81.6, -68.8 81.6, -68.8 76.1, -75.1 76.1))\'))',
            'intersects(centroid($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
            'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))'
        ])
        return filters

    # HERE GO THE PROVIDER SPECIFIC TESTS
    def testDateTimeTypes(self):
        vl = QgsVectorLayer('%s table="qgis_test"."date_times" sql=' %
                            (self.dbconn), "testdatetimes", "mssql")
        assert(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName(
            'date_field')).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName(
            'time_field')).type(), QVariant.Time)
        self.assertEqual(fields.at(fields.indexFromName(
            'datetime_field')).type(), QVariant.DateTime)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        date_idx = vl.fields().lookupField('date_field')
        self.assertIsInstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 3, 4))
        time_idx = vl.fields().lookupField('time_field')
        self.assertIsInstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 52))
        datetime_idx = vl.fields().lookupField('datetime_field')
        self.assertIsInstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(
            QDate(2004, 3, 4), QTime(13, 41, 52)))

    def testFloatDecimalFields(self):
        vl = QgsVectorLayer('%s table="qgis_test"."float_dec" sql=' %
                            (self.dbconn), "testprec", "mssql")
        self.assertTrue(vl.isValid())

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName(
            'float_field')).type(), QVariant.Double)
        self.assertEqual(fields.at(fields.indexFromName(
            'float_field')).length(), 15)
        self.assertEqual(fields.at(fields.indexFromName(
            'float_field')).precision(), -1)

        self.assertEqual(fields.at(fields.indexFromName(
            'dec_field')).type(), QVariant.Double)
        self.assertEqual(fields.at(fields.indexFromName(
            'dec_field')).length(), 7)
        self.assertEqual(fields.at(fields.indexFromName(
            'dec_field')).precision(), 3)

        f = next(vl.getFeatures(QgsFeatureRequest()))

        float_idx = vl.fields().lookupField('float_field')
        self.assertIsInstance(f.attributes()[float_idx], float)
        self.assertAlmostEqual(f.attributes()[float_idx], 1.1111111111, 5)
        dec_idx = vl.fields().lookupField('dec_field')
        self.assertIsInstance(f.attributes()[dec_idx], float)
        self.assertEqual(f.attributes()[dec_idx], 1.123)

    def testCreateLayer(self):
        layer = QgsVectorLayer("Point?field=id:integer&field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory")
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes([1, "test", 1])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
        f2 = QgsFeature()
        f2.setAttributes([2, "test2", 3])
        f3 = QgsFeature()
        f3.setAttributes([3, "test2", NULL])
        f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(3, 2)))
        f4 = QgsFeature()
        f4.setAttributes([4, NULL, 3])
        f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(4, 3)))
        pr.addFeatures([f, f2, f3, f4])

        uri = '{} table="qgis_test"."new_table" sql='.format(self.dbconn)
        error, message = QgsVectorLayerExporter.exportLayer(layer, uri, 'mssql', QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(error, QgsVectorLayerExporter.NoError)

        new_layer = QgsVectorLayer(uri, 'new', 'mssql')
        self.assertTrue(new_layer.isValid())
        self.assertEqual(new_layer.wkbType(), QgsWkbTypes.Point)
        self.assertEqual([f.name() for f in new_layer.fields()], ['qgs_fid', 'id', 'fldtxt', 'fldint'])

        features = [f.attributes() for f in new_layer.getFeatures()]
        self.assertEqual(features, [[1, 1, 'test', 1],
                                    [2, 2, 'test2', 3],
                                    [3, 3, 'test2', NULL],
                                    [4, 4, NULL, 3]])
        geom = [f.geometry().asWkt() for f in new_layer.getFeatures()]
        self.assertEqual(geom, ['Point (1 2)', '', 'Point (3 2)', 'Point (4 3)'])

    def testCreateLayerMultiPoint(self):
        layer = QgsVectorLayer("MultiPoint?crs=epsg:3111&field=id:integer&field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory")
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes([1, "test", 1])
        f.setGeometry(QgsGeometry.fromWkt('MultiPoint(1 2, 3 4)'))
        f2 = QgsFeature()
        f2.setAttributes([2, "test2", 3])
        f3 = QgsFeature()
        f3.setAttributes([3, "test2", NULL])
        f3.setGeometry(QgsGeometry.fromWkt('MultiPoint(7 8)'))
        pr.addFeatures([f, f2, f3])

        uri = '{} table="qgis_test"."new_table_multipoint" sql='.format(self.dbconn)
        error, message = QgsVectorLayerExporter.exportLayer(layer, uri, 'mssql', QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(error, QgsVectorLayerExporter.NoError)

        new_layer = QgsVectorLayer(uri, 'new', 'mssql')
        self.assertTrue(new_layer.isValid())
        self.assertEqual(new_layer.wkbType(), QgsWkbTypes.MultiPoint)
        self.assertEqual(new_layer.crs().authid(), 'EPSG:3111')
        self.assertEqual([f.name() for f in new_layer.fields()], ['qgs_fid', 'id', 'fldtxt', 'fldint'])

        features = [f.attributes() for f in new_layer.getFeatures()]
        self.assertEqual(features, [[1, 1, 'test', 1],
                                    [2, 2, 'test2', 3],
                                    [3, 3, 'test2', NULL]])
        geom = [f.geometry().asWkt() for f in new_layer.getFeatures()]
        self.assertEqual(geom, ['MultiPoint ((1 2),(3 4))', '', 'MultiPoint ((7 8))'])

    def testInsertPolygonInMultiPolygon(self):
        layer = QgsVectorLayer("MultiPolygon?crs=epsg:4326&field=id:integer", "addfeat", "memory")
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes([1])
        f.setGeometry(QgsGeometry.fromWkt('MultiPolygon(((0 0, 1 0, 1 1, 0 1, 0 0)),((10 0, 11 0, 11 1, 10 1, 10 0)))'))
        pr.addFeatures([f])

        uri = '{} table="qgis_test"."new_table_multipolygon" sql='.format(self.dbconn)
        error, message = QgsVectorLayerExporter.exportLayer(layer, uri, 'mssql', QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(error, QgsVectorLayerExporter.NoError)

        new_layer = QgsVectorLayer(uri, 'new', 'mssql')
        self.assertTrue(new_layer.isValid())
        self.assertEqual(new_layer.wkbType(), QgsWkbTypes.MultiPolygon)
        geom = [f.geometry().asWkt() for f in new_layer.getFeatures()]
        self.assertEqual(geom, ['MultiPolygon (((0 0, 1 0, 1 1, 0 1, 0 0)),((10 0, 11 0, 11 1, 10 1, 10 0)))'])

        # add single part
        f2 = QgsFeature()
        f2.setAttributes([2])
        f2.setGeometry(QgsGeometry.fromWkt('Polygon((30 0, 31 0, 31 1, 30 1, 30 0))'))
        self.assertTrue(new_layer.dataProvider().addFeatures([f2]))

        # should become multipart
        geom = [f.geometry().asWkt() for f in new_layer.getFeatures()]
        self.assertEqual(geom, ['MultiPolygon (((0 0, 1 0, 1 1, 0 1, 0 0)),((10 0, 11 0, 11 1, 10 1, 10 0)))', 'MultiPolygon (((30 0, 31 0, 31 1, 30 1, 30 0)))'])

    def testOverwriteExisting(self):
        layer = QgsVectorLayer("NoGeometry?field=pk:integer", "addfeat", "memory")
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes([133])
        pr.addFeatures([f])

        uri = '{} table="qgis_test"."sacrificialLamb" sql='.format(self.dbconn)
        new_layer = QgsVectorLayer(uri, 'new', 'mssql')
        self.assertTrue(new_layer.isValid())
        self.assertEqual([f.attributes() for f in new_layer.getFeatures()], [[1]])

        # try to overwrite
        error, message = QgsVectorLayerExporter.exportLayer(layer, uri, 'mssql', QgsCoordinateReferenceSystem())
        self.assertEqual(error, QgsVectorLayerExporter.ErrCreateLayer)

        # should not have overwritten features
        self.assertEqual([f.attributes() for f in new_layer.getFeatures()], [[1]])

    def testMultiGeomColumns(self):
        uri = '{} table="qgis_test"."multiGeomColumns" (geom1) sql='.format(self.dbconn)
        new_layer = QgsVectorLayer(uri, 'new', 'mssql')
        self.assertTrue(new_layer.isValid())

        geom = {f[0]: f.geometry().asWkt() for f in new_layer.getFeatures()}
        self.assertEqual(geom, {1: 'Point (2 3)', 2: 'Point (3 4)', 3: '', 4: 'Point (5 6)', 5: 'Point (1 2)'})

        uri = '{} table="qgis_test"."multiGeomColumns" (geom2) sql='.format(self.dbconn)
        new_layer2 = QgsVectorLayer(uri, 'new', 'mssql')
        self.assertTrue(new_layer2.isValid())

        geom = {f[0]: f.geometry().asWkt() for f in new_layer2.getFeatures()}
        self.assertEqual(geom, {1: 'LineString (2 3, 4 5)', 2: 'LineString (3 4, 5 6)', 3: 'LineString (1 2, 3 4)', 4: 'LineString (5 6, 7 8)', 5: ''})

    def testInvalidGeometries(self):
        """ Test what happens when SQL Server is a POS and throws an exception on encountering an invalid geometry """
        vl = QgsVectorLayer('%s srid=4167 type=POLYGON table="qgis_test"."invalid_polys" (ogr_geometry) sql=' %
                            (self.dbconn), "testinvalid", "mssql")
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.dataProvider().extent().toString(1), QgsRectangle(173.953, -41.513, 173.967, -41.502).toString(1))

        #burn through features - don't want SQL server to trip up on the invalid ones
        count = 0
        for f in vl.dataProvider().getFeatures():
            count += 1
        self.assertEqual(count, 39)

        count = 0

        for f in vl.dataProvider().getFeatures(QgsFeatureRequest(QgsRectangle(173, -42, 174, -41))):
            count += 1
        # two invalid geometry features
        self.assertEqual(count, 37)
        # sorry... you get NO chance to see these features exist and repair them... because SQL server. Use PostGIS instead and live a happier life!

        # with estimated metadata
        vl = QgsVectorLayer('%s srid=4167 type=POLYGON  estimatedmetadata=true table="qgis_test"."invalid_polys" (ogr_geometry) sql=' %
                            (self.dbconn), "testinvalid", "mssql")
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().extent().toString(1), QgsRectangle(173.954, -41.513, 173.967, -41.502).toString(1))

        # Now, play on the edge! Let's disable invalid geometry handling and watch things crash and burn
        vl = QgsVectorLayer('%s srid=4167 type=POLYGON table="qgis_test"."invalid_polys" (ogr_geometry) disableInvalidGeometryHandling="1" sql=' %
                            (self.dbconn), "testinvalid", "mssql")
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.dataProvider().extent().toString(1), 'Empty') # HAHA - you asked for it
        #burn through features - don't expect anything wrong here yet
        count = 0
        for f in vl.dataProvider().getFeatures():
            count += 1
        self.assertEqual(count, 39)
        count = 0

        for f in vl.dataProvider().getFeatures(QgsFeatureRequest(QgsRectangle(173, -42, 174, -41))):
            count += 1
        # now you only get 1 feature *sad trumpet*
        self.assertEqual(count, 1)
        count = 0

        # same, with estimated metadata
        vl = QgsVectorLayer('%s srid=4167 type=POLYGON  estimatedmetadata=true table="qgis_test"."invalid_polys" (ogr_geometry) disableInvalidGeometryHandling="1" sql=' %
                            (self.dbconn), "testinvalid", "mssql")
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().extent().toString(1), 'Empty')


if __name__ == '__main__':
    unittest.main()
