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

import qgis  # NOQA

import os

from qgis.core import (QgsSettings,
                       QgsVectorLayer,
                       QgsFeatureRequest,
                       QgsFeature,
                       QgsField,
                       QgsFields,
                       QgsFieldConstraints,
                       QgsDataSourceUri,
                       QgsWkbTypes,
                       QgsGeometry,
                       QgsPointXY,
                       QgsRectangle,
                       QgsProviderRegistry,
                       NULL,
                       QgsVectorLayerExporter,
                       QgsCoordinateReferenceSystem,
                       QgsDataProvider)

from qgis.PyQt.QtCore import QDate, QTime, QDateTime, QVariant, QDir
from utilities import unitTestDataPath
from qgis.testing import start_app, unittest
from providertestbase import ProviderTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsMssqlProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        # These are the connection details for the SQL Server instance running on Travis
        cls.dbconn = "service='testsqlserver' user=sa password='<YourStrong!Passw0rd>' "
        if 'QGIS_MSSQLTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_MSSQLTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=',
            'test', 'mssql')
        assert cls.vl.dataProvider() is not None, "No data provider for {}".format(cls.vl.source())
        assert cls.vl.isValid(), cls.vl.dataProvider().error().message()
        cls.source = cls.vl.dataProvider()
        cls.poly_vl = QgsVectorLayer(
            cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=',
            'test', 'mssql')
        assert cls.poly_vl.isValid(), cls.poly_vl.dataProvider().error().message()
        cls.poly_provider = cls.poly_vl.dataProvider()

        # Triggers a segfault in the sql server odbc driver on Travis - TODO test with more recent Ubuntu base image
        if os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'):
            del cls.getEditableLayer

        # Use connections API
        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        cls.conn_api = md.createConnection(cls.dbconn, {})

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        pass

    def setUp(self):
        for t in ['new_table', 'new_table_multipoint', 'new_table_multipolygon']:
            self.execSQLCommand('DROP TABLE IF EXISTS qgis_test.[{}]'.format(t))

    def execSQLCommand(self, sql):
        self.assertTrue(self.conn_api)
        self.conn_api.executeSql(sql)

    def getSource(self):
        # create temporary table for edit tests
        self.execSQLCommand('DROP TABLE IF EXISTS qgis_test.edit_data')
        self.execSQLCommand(
            """CREATE TABLE qgis_test.edit_data (pk INTEGER PRIMARY KEY,cnt integer, name nvarchar(max), name2 nvarchar(max), num_char nvarchar(max), dt datetime, [date] date, [time] time, geom geometry)""")
        self.execSQLCommand("INSERT INTO [qgis_test].[edit_data] (pk, cnt, name, name2, num_char, dt, [date], [time], geom) VALUES "
                            "(5, -200, NULL, 'NuLl', '5', '2020-05-04T12:13:14', '2020-05-02', '12:13:01', geometry::STGeomFromText('POINT(-71.123 78.23)', 4326)),"
                            "(3, 300, 'Pear', 'PEaR', '3', NULL, NULL, NULL, NULL),"
                            "(1, 100, 'Orange', 'oranGe', '1', '2020-05-03T12:13:14', '2020-05-03', '12:13:14', geometry::STGeomFromText('POINT(-70.332 66.33)', 4326)),"
                            "(2, 200, 'Apple', 'Apple', '2', '2020-05-04T12:14:14', '2020-05-04', '12:14:14', geometry::STGeomFromText('POINT(-68.2 70.8)', 4326)),"
                            "(4, 400, 'Honey', 'Honey', '4', '2021-05-04T13:13:14', '2021-05-04', '13:13:14', geometry::STGeomFromText('POINT(-65.32 78.3)', 4326))")

        vl = QgsVectorLayer(
            self.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."edit_data" (geom) sql=',
            'test', 'mssql')
        return vl

    def testDeleteFeaturesPktInt(self):
        vl = self.getSource()
        dp = vl.dataProvider()

        self.assertEqual(dp.featureCount(), 5)

        self.assertTrue(dp.deleteFeatures([1, 3, 4]))
        self.assertEqual(dp.featureCount(), 2)

        self.assertFalse(dp.deleteFeatures([3]))
        self.assertFalse(dp.deleteFeatures([10]))
        self.assertFalse(dp.deleteFeatures([3, 10]))

        self.assertTrue(dp.deleteFeatures([5]))
        self.assertEqual(dp.featureCount(), 1)

        self.assertTrue(dp.deleteFeatures([2]))
        self.assertEqual(dp.featureCount(), 0)

    def getEditableLayer(self):
        return self.getSource()

    def enableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', True)
        return True

    def disableCompiler(self):
        QgsSettings().setValue('/qgis/compileExpressions', False)

    def partiallyCompiledFilters(self):
        filters = set([
            'name ILIKE \'QGIS\'',
            'name = \'Apple\'',
            '\"NaMe\" = \'Apple\'',
            'name = \'apple\'',
            'name LIKE \'Apple\'',
            'name LIKE \'aPple\'',
            'name ILIKE \'aPple\'',
            'name LIKE \'Ap_le\'',
            'name LIKE \'Ap\\_le\'',
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
            'intersects(point_on_surface($geometry),geom_from_wkt( \'Polygon ((-74.4 78.2, -74.4 79.1, -66.8 79.1, -66.8 78.2, -74.4 78.2))\'))',
            '"dt" = to_datetime(\'000www14ww13ww12www4ww5ww2020\',\'zzzwwwsswwmmwwhhwwwdwwMwwyyyy\')',
            '"date" = to_date(\'www4ww5ww2020\',\'wwwdwwMwwyyyy\')',
            '"time" = to_time(\'000www14ww13ww12www\',\'zzzwwwsswwmmwwhhwww\')'
        ])
        return filters

    def testGetFeaturesUncompiled(self):
        if os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'):
            return
        super().testGetFeaturesUncompiled()

    def testGetFeaturesExp(self):
        if os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'):
            return
        super().testGetFeaturesExp()

    def testOrderBy(self):
        if os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'):
            return
        super().testOrderBy()

    def testOrderByCompiled(self):
        if os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'):
            return
        super().testOrderByCompiled()

    # HERE GO THE PROVIDER SPECIFIC TESTS
    def testDateTimeTypes(self):
        vl = QgsVectorLayer('%s table="qgis_test"."date_times" sql=' %
                            (self.dbconn), "testdatetimes", "mssql")
        assert (vl.isValid())

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

    @unittest.skipIf(os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'), 'Failing on Travis')
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
        error, message = QgsVectorLayerExporter.exportLayer(layer, uri, 'mssql',
                                                            QgsCoordinateReferenceSystem('EPSG:4326'))
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

    @unittest.skipIf(os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'), 'Failing on Travis')
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
        error, message = QgsVectorLayerExporter.exportLayer(layer, uri, 'mssql',
                                                            QgsCoordinateReferenceSystem('EPSG:3111'))
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

    @unittest.skipIf(os.environ.get('QGIS_CONTINUOUS_INTEGRATION_RUN', 'true'), 'Failing on Travis')
    def testCurveGeometries(self):
        geomtypes = ['CompoundCurveM', 'CurvePolygonM', 'CircularStringM', 'CompoundCurveZM', 'CurvePolygonZM',
                     'CircularStringZM', 'CompoundCurveZ', 'CurvePolygonZ', 'CircularStringZ', 'CompoundCurve',
                     'CurvePolygon', 'CircularString']
        geoms = [
            'CompoundCurveM ((0 -23.43778 10, 0 23.43778 10),CircularStringM (0 23.43778 10, -45 33.43778 10, -90 23.43778 10),(-90 23.43778 10, -90 -23.43778 10),CircularStringM (-90 -23.43778 10, -45 -23.43778 10, 0 -23.43778 10))',
            'CurvePolygonM (CompoundCurveM ((0 -23.43778 10, 0 -15.43778 10, 0 23.43778 10),CircularStringM (0 23.43778 10, -45 100 10, -90 23.43778 10),(-90 23.43778 10, -90 -23.43778 10),CircularStringM (-90 -23.43778 10, -45 -16.43778 10, 0 -23.43778 10)),CompoundCurveM (CircularStringM (-30 0 10, -48 -12 10, -60 0 10, -48 -6 10, -30 0 10)))',
            'CircularStringM (0 0 10, 0.14644660940672 0.35355339059327 10, 0.5 0.5 10, 0.85355339059327 0.35355339059327 10, 1 0 10, 0.85355339059327 -0.35355339059327 10, 0.5 -0.5 10, 0.14644660940672 -0.35355339059327 10, 0 0 10)',
            'CompoundCurveZM ((0 -23.43778 2 10, 0 23.43778 2 10),CircularStringZM (0 23.43778 2 10, -45 33.43778 2 10, -90 23.43778 2 10),(-90 23.43778 2 10, -90 -23.43778 2 10),CircularStringZM (-90 -23.43778 2 10, -45 -23.43778 2 10, 0 -23.43778 2 10))',
            'CurvePolygonZM (CompoundCurveZM ((0 -23.43778 5 10, 0 -15.43778 8 10, 0 23.43778 6 10),CircularStringZM (0 23.43778 6 10, -45 100 6 10, -90 23.43778 6 10),(-90 23.43778 6 10, -90 -23.43778 5 10),CircularStringZM (-90 -23.43778 5 10, -45 -16.43778 5 10, 0 -23.43778 5 10)),CompoundCurveZM (CircularStringZM (-30 0 10 10, -48 -12 10 10, -60 0 10 10, -48 -6 10 10, -30 0 10 10)))',
            'CircularStringZM (0 0 1 10, 0.14644660940672 0.35355339059327 1 10, 0.5 0.5 1 10, 0.85355339059327 0.35355339059327 1 10, 1 0 1 10, 0.85355339059327 -0.35355339059327 1 10, 0.5 -0.5 1 10, 0.14644660940672 -0.35355339059327 1 10, 0 0 1 10)',
            'CompoundCurveZ ((0 -23.43778 2, 0 23.43778 2),CircularStringZ (0 23.43778 2, -45 33.43778 2, -90 23.43778 2),(-90 23.43778 2, -90 -23.43778 2),CircularStringZ (-90 -23.43778 2, -45 -23.43778 2, 0 -23.43778 2))',
            'CurvePolygonZ (CompoundCurveZ ((0 -23.43778 5, 0 -15.43778 8, 0 23.43778 6),CircularStringZ (0 23.43778 6, -45 100 6, -90 23.43778 6),(-90 23.43778 6, -90 -23.43778 5),CircularStringZ (-90 -23.43778 5, -45 -16.43778 5, 0 -23.43778 5)),CompoundCurveZ (CircularStringZ (-30 0 10, -48 -12 10, -60 0 10, -48 -6 10, -30 0 10)))',
            'CircularStringZ (0 0 1, 0.14644660940672 0.35355339059327 1, 0.5 0.5 1, 0.85355339059327 0.35355339059327 1, 1 0 1, 0.85355339059327 -0.35355339059327 1, 0.5 -0.5 1, 0.14644660940672 -0.35355339059327 1, 0 0 1)',
            'CompoundCurve ((0 -23.43778, 0 23.43778),CircularString (0 23.43778, -45 33.43778, -90 23.43778),(-90 23.43778, -90 -23.43778),CircularString (-90 -23.43778, -45 -23.43778, 0 -23.43778))',
            'CurvePolygon (CompoundCurve ((0 -23.43778, 0 -15.43778, 0 23.43778),CircularString (0 23.43778, -45 100, -90 23.43778),(-90 23.43778, -90 -23.43778),CircularString (-90 -23.43778, -45 -16.43778, 0 -23.43778)),CompoundCurve (CircularString (-30 0, -48 -12, -60 0, -48 -6, -30 0)))',
            'CircularString (0 0, 0.14644660940672 0.35355339059327, 0.5 0.5, 0.85355339059327 0.35355339059327, 1 0, 0.85355339059327 -0.35355339059327, 0.5 -0.5, 0.14644660940672 -0.35355339059327, 0 0)']
        for idx, t in enumerate(geoms):
            f = QgsFeature()
            g = QgsGeometry.fromWkt(t)
            f.setGeometry(g)
            layer = QgsVectorLayer(geomtypes[idx] + "?crs=epsg:4326", "addfeat", "memory")
            pr = layer.dataProvider()
            pr.addFeatures([f])
            uri = self.dbconn + ' table="qgis_test"."new_table_curvegeom_' + str(idx) + '" sql='
            error, message = QgsVectorLayerExporter.exportLayer(layer, uri, 'mssql',
                                                                QgsCoordinateReferenceSystem('EPSG:4326'))
            self.assertEqual(error, QgsVectorLayerExporter.NoError)
            new_layer = QgsVectorLayer(uri, 'new', 'mssql')
            self.assertTrue(new_layer.isValid())
            self.assertEqual(new_layer.wkbType(), g.wkbType())
            self.assertEqual(new_layer.crs().authid(), 'EPSG:4326')
            result_geoms = [f.geometry().asWkt(14) for f in new_layer.getFeatures()]
            self.assertEqual(result_geoms, [t])
            self.execSQLCommand('DROP TABLE IF EXISTS [qgis_test].[new_table_curvegeom_{}]'.format(str(idx)))

    def testStyle(self):
        self.execSQLCommand('DROP TABLE IF EXISTS layer_styles')

        res, err = QgsProviderRegistry.instance().styleExists('mssql', 'not valid', '')
        self.assertFalse(res)
        self.assertTrue(err)

        vl = self.getSource()
        self.assertTrue(vl.isValid())
        self.assertTrue(
            vl.dataProvider().isSaveAndLoadStyleToDatabaseSupported())

        # table layer_styles does not exist

        res, err = QgsProviderRegistry.instance().styleExists('mssql', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('mssql', vl.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, -1)
        self.assertEqual(idlist, [])
        self.assertEqual(namelist, [])
        self.assertEqual(desclist, [])
        self.assertFalse(errmsg)

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        mFilePath = QDir.toNativeSeparators(
            '%s/symbol_layer/%s.qml' % (unitTestDataPath(), "singleSymbol"))
        status = vl.loadNamedStyle(mFilePath)
        self.assertTrue(status)

        # The style is saved as non-default
        errorMsg = vl.saveStyleToDatabase(
            "by day", "faded greens and elegant patterns", False, "")
        self.assertFalse(errorMsg)

        res, err = QgsProviderRegistry.instance().styleExists('mssql', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('mssql', vl.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('mssql', vl.source(), 'by day')
        self.assertTrue(res)
        self.assertFalse(err)

        # the style id should be "1", not "by day"
        qml, errmsg = vl.getStyleFromDatabase("by day")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 1)
        self.assertFalse(errmsg)
        self.assertEqual(idlist, ["1"])
        self.assertEqual(namelist, ["by day"])
        self.assertEqual(desclist, ["faded greens and elegant patterns"])

        qml, errmsg = vl.getStyleFromDatabase("100")
        self.assertFalse(qml)
        self.assertTrue(errmsg)

        qml, errmsg = vl.getStyleFromDatabase("1")
        self.assertTrue(qml.startswith('<!DOCTYPE qgis'), qml)
        self.assertFalse(errmsg)

        # We save now the style again twice but with one as default
        errorMsg = vl.saveStyleToDatabase(
            "related style", "faded greens and elegant patterns", False, "")
        self.assertFalse(errorMsg)
        errorMsg = vl.saveStyleToDatabase(
            "default style", "faded greens and elegant patterns", True, "")
        self.assertFalse(errorMsg)

        res, err = QgsProviderRegistry.instance().styleExists('mssql', vl.source(), '')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('mssql', vl.source(), 'a style')
        self.assertFalse(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('mssql', vl.source(), 'default style')
        self.assertTrue(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('mssql', vl.source(), 'related style')
        self.assertTrue(res)
        self.assertFalse(err)
        res, err = QgsProviderRegistry.instance().styleExists('mssql', vl.source(), 'by day')
        self.assertTrue(res)
        self.assertFalse(err)

        related_count, idlist, namelist, desclist, errmsg = vl.listStylesInDatabase()
        self.assertEqual(related_count, 3)
        self.assertFalse(errmsg)
        self.assertCountEqual(idlist, ["1", "2", "3"])
        self.assertCountEqual(namelist, ["default style", "related style", "by day"])
        self.assertCountEqual(desclist, ["faded greens and elegant patterns"] * 3)

    def testInsertPolygonInMultiPolygon(self):
        layer = QgsVectorLayer("MultiPolygon?crs=epsg:4326&field=id:integer", "addfeat", "memory")
        pr = layer.dataProvider()
        f = QgsFeature()
        f.setAttributes([1])
        f.setGeometry(QgsGeometry.fromWkt('MultiPolygon(((0 0, 1 0, 1 1, 0 1, 0 0)),((10 0, 11 0, 11 1, 10 1, 10 0)))'))
        pr.addFeatures([f])

        uri = '{} table="qgis_test"."new_table_multipolygon" sql='.format(self.dbconn)
        error, message = QgsVectorLayerExporter.exportLayer(layer, uri, 'mssql',
                                                            QgsCoordinateReferenceSystem('EPSG:4326'))
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
        self.assertEqual(geom, ['MultiPolygon (((0 0, 1 0, 1 1, 0 1, 0 0)),((10 0, 11 0, 11 1, 10 1, 10 0)))',
                                'MultiPolygon (((30 0, 31 0, 31 1, 30 1, 30 0)))'])

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
        self.assertEqual(geom, {1: 'LineString (2 3, 4 5)', 2: 'LineString (3 4, 5 6)', 3: 'LineString (1 2, 3 4)',
                                4: 'LineString (5 6, 7 8)', 5: ''})

    def testInvalidGeometries(self):
        """ Test what happens when SQL Server is a POS and throws an exception on encountering an invalid geometry """
        vl = QgsVectorLayer('%s srid=4167 type=POLYGON table="qgis_test"."invalid_polys" (ogr_geometry) sql=' %
                            (self.dbconn), "testinvalid", "mssql")
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.dataProvider().extent().toString(1),
                         QgsRectangle(173.953, -41.513, 173.967, -41.502).toString(1))

        # burn through features - don't want SQL server to trip up on the invalid ones
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
        vl = QgsVectorLayer(
            '%s srid=4167 type=POLYGON  estimatedmetadata=true table="qgis_test"."invalid_polys" (ogr_geometry) sql=' %
            (self.dbconn), "testinvalid", "mssql")
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().extent().toString(1),
                         QgsRectangle(173.954, -41.513, 173.967, -41.502).toString(1))

        # Now, play on the edge! Let's disable invalid geometry handling and watch things crash and burn
        vl = QgsVectorLayer(
            '%s srid=4167 type=POLYGON table="qgis_test"."invalid_polys" (ogr_geometry) disableInvalidGeometryHandling="1" sql=' %
            (self.dbconn), "testinvalid", "mssql")
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.dataProvider().extent().toString(1), 'Empty')  # HAHA - you asked for it
        # burn through features - don't expect anything wrong here yet
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
        vl = QgsVectorLayer(
            '%s srid=4167 type=POLYGON  estimatedmetadata=true table="qgis_test"."invalid_polys" (ogr_geometry) disableInvalidGeometryHandling="1" sql=' %
            (self.dbconn), "testinvalid", "mssql")
        self.assertTrue(vl.isValid())
        self.assertEqual(vl.dataProvider().extent().toString(1), 'Empty')

    def testEvaluateDefaultValueClause(self):

        vl = QgsVectorLayer(
            '%s table="qgis_test"."someData" sql=' %
            (self.dbconn), "testdatetimes", "mssql")

        # Activate EvaluateDefaultValues
        vl.dataProvider().setProviderProperty(QgsDataProvider.EvaluateDefaultValues, True)

        name_index = vl.fields().lookupField('name')
        defaultValue = vl.dataProvider().defaultValue(name_index)
        self.assertEqual(defaultValue, 'qgis')

    def testPktComposite(self):
        """
        Check that tables with PKs composed of many fields of different types are correctly read and written to
        """
        vl = QgsVectorLayer('{} type=POINT estimatedmetadata=true key=\'"pk1","pk2"\' table="qgis_test"."tb_test_compound_pk" (geom)'.format(self.dbconn), "test_compound", "mssql")
        self.assertTrue(vl.isValid())

        fields = vl.fields()

        f = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk1 = 1 AND pk2 = 2')))
        # first of all: we must be able to fetch a valid feature
        self.assertTrue(f.isValid())
        self.assertEqual(f['pk1'], 1)
        self.assertEqual(f['pk2'], 2)
        self.assertEqual(f['value'], 'test 2')

        # can we edit a field?
        vl.startEditing()
        vl.changeAttributeValue(f.id(), fields.indexOf('value'), 'Edited Test 2')
        self.assertTrue(vl.commitChanges())

        # Did we get it right? Let's create a new QgsVectorLayer and try to read back our changes:
        vl2 = QgsVectorLayer('{} type=POINT estimatedmetadata=true table="qgis_test"."tb_test_compound_pk" (geom) key=\'"pk1","pk2"\' '.format(self.dbconn), "test_compound2", "mssql")
        self.assertTrue(vl2.isValid())
        f2 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression('pk1 = 1 AND pk2 = 2')))
        self.assertTrue(f2.isValid())

        # Then, making sure we really did change our value.
        self.assertEqual(f2['value'], 'Edited Test 2')

        # How about inserting a new field?
        f3 = QgsFeature(vl2.fields())
        f3['pk1'] = 4
        f3['pk2'] = -9223372036854775800
        f3['value'] = 'other test'
        vl.startEditing()
        res, f3 = vl.dataProvider().addFeatures([f3])
        self.assertTrue(res)
        self.assertTrue(vl.commitChanges())

        # can we catch it on another layer?
        f4 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression('pk2 = -9223372036854775800')))

        self.assertTrue(f4.isValid())
        expected_attrs = [4, -9223372036854775800, 'other test']
        self.assertEqual(f4.attributes(), expected_attrs)

        # Finally, let's delete one of the features.
        f5 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression('pk1 = 2 AND pk2 = 1')))
        vl2.startEditing()
        vl2.deleteFeatures([f5.id()])
        self.assertTrue(vl2.commitChanges())

        # did we really delete? Let's try to get the deleted feature from the first layer.
        f_iterator = vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk1 = 2 AND pk2 = 1'))
        got_feature = True

        try:
            f6 = next(f_iterator)
            got_feature = f6.isValid()
        except StopIteration:
            got_feature = False

        self.assertFalse(got_feature)

    def testPktCompositeFloat(self):
        """
        Check that tables with PKs composed of many fields of different types are correctly read and written to
        """
        vl = QgsVectorLayer('{} type=POINT key=\'"pk1","pk2","pk3"\' table="qgis_test"."tb_test_composite_float_pk" (geom)'.format(self.dbconn), "test_composite_float", "mssql")
        self.assertTrue(vl.isValid())

        fields = vl.fields()

        f = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk3 = 3.14159274')))
        # first of all: we must be able to fetch a valid feature
        self.assertTrue(f.isValid())
        self.assertEqual(f['pk1'], 1)
        self.assertEqual(f['pk2'], 2)

        self.assertEqual(round(f['pk3'], 6), round(3.14159274, 6))
        self.assertEqual(f['value'], 'test 2')

        # can we edit a field?
        vl.startEditing()
        vl.changeAttributeValue(f.id(), fields.indexOf('value'), 'Edited Test 2')
        self.assertTrue(vl.commitChanges())

        # Did we get it right? Let's create a new QgsVectorLayer and try to read back our changes:
        vl2 = QgsVectorLayer('{} sslmode=disable srid=4326 key=\'"pk1","pk2","pk3"\' table="qgis_test"."tb_test_composite_float_pk" (geom)'.format(self.dbconn), "test_composite_float2", "mssql")
        self.assertTrue(vl2.isValid())
        f2 = next(vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk3 = 3.14159274')))
        self.assertTrue(f2.isValid())

        # just making sure we have the correct feature
        # Only 6 decimals for PostgreSQL 11.
        self.assertEqual(round(f2['pk3'], 6), round(3.14159274, 6))

        # Then, making sure we really did change our value.
        self.assertEqual(f2['value'], 'Edited Test 2')

        # How about inserting a new field?
        f3 = QgsFeature(vl2.fields())
        f3['pk1'] = 4
        f3['pk2'] = -9223372036854775800
        f3['pk3'] = 7.29154
        f3['value'] = 'other test'
        vl.startEditing()
        res, f3 = vl.dataProvider().addFeatures([f3])
        self.assertTrue(res)
        self.assertTrue(vl.commitChanges())

        # can we catch it on another layer?
        f4 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression('pk2 = -9223372036854775800')))

        self.assertTrue(f4.isValid())
        expected_attrs = [4, -9223372036854775800, 7.29154, 'other test']
        gotten_attrs = [f4['pk1'], f4['pk2'], round(f4['pk3'], 5), f4['value']]
        self.assertEqual(gotten_attrs, expected_attrs)

        # Finally, let's delete one of the features.
        f5 = next(vl2.getFeatures(QgsFeatureRequest().setFilterExpression('pk3 = 7.29154')))
        vl2.startEditing()
        vl2.deleteFeatures([f5.id()])
        self.assertTrue(vl2.commitChanges())

        # did we really delete?
        f_iterator = vl.getFeatures(QgsFeatureRequest().setFilterExpression('pk3 = 7.29154'))
        got_feature = True

        try:
            f6 = next(f_iterator)
            got_feature = f6.isValid()
        except StopIteration:
            got_feature = False

        self.assertFalse(got_feature)

    def testNotNullConstraint(self):
        vl = QgsVectorLayer('%s table="qgis_test"."constraints" sql=' %
                            (self.dbconn), "testdatetimes", "mssql")
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 4)

        # test some bad field indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1),
                         QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(
            1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0) &
                        QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(1)
                         & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(vl.dataProvider().fieldConstraints(2) &
                        QgsFieldConstraints.ConstraintNotNull)
        self.assertFalse(vl.dataProvider().fieldConstraints(3)
                         & QgsFieldConstraints.ConstraintNotNull)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints()
                        & QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(fields.at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(1).constraints().constraints()
                         & QgsFieldConstraints.ConstraintNotNull)
        self.assertTrue(fields.at(2).constraints().constraints()
                        & QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(fields.at(2).constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(3).constraints().constraints()
                         & QgsFieldConstraints.ConstraintNotNull)

    def testUniqueConstraint(self):
        vl = QgsVectorLayer('%s table="qgis_test"."constraints" sql=' %
                            (self.dbconn), "testdatetimes", "mssql")
        self.assertTrue(vl.isValid())
        self.assertEqual(len(vl.fields()), 4)

        # test some bad field indexes
        self.assertEqual(vl.dataProvider().fieldConstraints(-1),
                         QgsFieldConstraints.Constraints())
        self.assertEqual(vl.dataProvider().fieldConstraints(
            1001), QgsFieldConstraints.Constraints())

        self.assertTrue(vl.dataProvider().fieldConstraints(0)
                        & QgsFieldConstraints.ConstraintUnique)
        self.assertTrue(vl.dataProvider().fieldConstraints(1)
                        & QgsFieldConstraints.ConstraintUnique)
        self.assertFalse(vl.dataProvider().fieldConstraints(2)
                         & QgsFieldConstraints.ConstraintUnique)
        self.assertFalse(vl.dataProvider().fieldConstraints(3)
                         & QgsFieldConstraints.ConstraintUnique)

        # test that constraints have been saved to fields correctly
        fields = vl.fields()
        self.assertTrue(fields.at(0).constraints().constraints()
                        & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(0).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique),
                         QgsFieldConstraints.ConstraintOriginProvider)
        self.assertTrue(fields.at(1).constraints().constraints()
                        & QgsFieldConstraints.ConstraintUnique)
        self.assertEqual(fields.at(1).constraints().constraintOrigin(QgsFieldConstraints.ConstraintUnique),
                         QgsFieldConstraints.ConstraintOriginProvider)
        self.assertFalse(fields.at(2).constraints().constraints()
                         & QgsFieldConstraints.ConstraintUnique)
        self.assertFalse(fields.at(3).constraints().constraints()
                         & QgsFieldConstraints.ConstraintUnique)

    def testIdentityFieldHandling(self):
        """
        Test identity field handling
        """
        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        conn = md.createConnection(self.dbconn, {})

        conn.execSql('DROP TABLE IF EXISTS qgis_test.test_identity')
        conn.execSql("""CREATE TABLE [qgis_test].[test_identity](
        [pk] [int] IDENTITY(1,1) NOT NULL,
        [name] [nchar](10) NULL,
        [geom] [geometry] NULL,
 CONSTRAINT [PK_test_table]  PRIMARY KEY CLUSTERED
(
        [pk] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]""")

        uri = '{} table="qgis_test"."test_identity" sql='.format(self.dbconn)
        vl = QgsVectorLayer(uri, '', 'mssql')
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.dataProvider().pkAttributeIndexes(), [0])
        self.assertEqual(vl.dataProvider().defaultValueClause(0), 'Autogenerate')
        identity_field = vl.dataProvider().fields().at(0)
        self.assertEqual(identity_field.name(), 'pk')
        self.assertEqual(identity_field.constraints().constraints(), QgsFieldConstraints.ConstraintNotNull)
        self.assertEqual(identity_field.constraints().constraintOrigin(QgsFieldConstraints.ConstraintNotNull), QgsFieldConstraints.ConstraintOriginProvider)
        self.assertEqual(identity_field.constraints().constraintStrength(QgsFieldConstraints.ConstraintNotNull),
                         QgsFieldConstraints.ConstraintStrengthHard)
        self.assertTrue(identity_field.isReadOnly())

    def getSubsetString(self):
        return '[cnt] > 100 and [cnt] < 410'

    def getSubsetString2(self):
        return '[cnt] > 100 and [cnt] < 400'

    def getSubsetString3(self):
        return '[name]=\'Apple\''

    def getSubsetStringNoMatching(self):
        return '[name]=\'AppleBearOrangePear\''

    def testExtentFromGeometryTable(self):
        """
        Check if the behavior of the mssql provider if extent is defined in the geometry_column table
        """
        # Create a layer
        layer = QgsVectorLayer("Point?field=id:integer&field=fldtxt:string&field=fldint:integer",
                               "layer", "memory")
        pr = layer.dataProvider()
        f1 = QgsFeature()
        f1.setAttributes([1, "test", 1])
        f1.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(1, 2)))
        f2 = QgsFeature()
        f2.setAttributes([2, "test2", 3])
        f3 = QgsFeature()
        f3.setAttributes([3, "test2", NULL])
        f3.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(3, 2)))
        f4 = QgsFeature()
        f4.setAttributes([4, NULL, 3])
        f4.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(4, 3)))
        pr.addFeatures([f1, f2, f3, f4])
        uri = '{} table="qgis_test"."layer_extent_in_geometry_table" sql='.format(self.dbconn)
        QgsVectorLayerExporter.exportLayer(layer, uri, 'mssql', QgsCoordinateReferenceSystem('EPSG:4326'))

        layerUri = QgsDataSourceUri(uri)
        # Load and check if the layer is valid
        loadedLayer = QgsVectorLayer(layerUri.uri(), "valid", "mssql")
        self.assertTrue(loadedLayer.isValid())
        extent = loadedLayer.extent()
        self.assertEqual(extent.toString(1),
                         QgsRectangle(1.0, 2.0, 4.0, 3.0).toString(1))

        # Load with flag extent in geometry_columns table and check if the layer is still valid and extent doesn't change
        layerUri.setParam('extentInGeometryColumns', '1')
        loadedLayer = QgsVectorLayer(layerUri.uri(), "invalid", "mssql")
        self.assertTrue(loadedLayer.isValid())
        extent = loadedLayer.extent()
        self.assertEqual(extent.toString(1),
                         QgsRectangle(1.0, 2.0, 4.0, 3.0).toString(1))

        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        conn = md.createConnection(self.dbconn, {})
        conn.addField(QgsField('qgis_xmin', QVariant.Double, 'FLOAT(24)'), 'dbo', 'geometry_columns')
        conn.addField(QgsField('qgis_xmax', QVariant.Double, 'FLOAT(24)'), 'dbo', 'geometry_columns')
        conn.addField(QgsField('qgis_ymin', QVariant.Double, 'FLOAT(24)'), 'dbo', 'geometry_columns')
        conn.addField(QgsField('qgis_ymax', QVariant.Double, 'FLOAT(24)'), 'dbo', 'geometry_columns')

        # try with empty attribute
        layerUri.setParam('extentInGeometryColumns', '1')
        loadedLayer = QgsVectorLayer(layerUri.uri(), "invalid", "mssql")
        self.assertTrue(loadedLayer.isValid())
        self.assertTrue(loadedLayer.isValid())
        extent = loadedLayer.extent()
        self.assertEqual(extent.toString(1),
                         QgsRectangle(1.0, 2.0, 4.0, 3.0).toString(1))

        conn.execSql('UPDATE dbo.geometry_columns SET qgis_xmin=0, qgis_xmax=5.5, qgis_ymin=0.5, qgis_ymax=6 WHERE f_table_name=\'layer_extent_in_geometry_table\'')

        # try with valid attribute
        layerUri.setParam('extentInGeometryColumns', '1')
        loadedLayer = QgsVectorLayer(layerUri.uri(), "valid", "mssql")
        self.assertTrue(loadedLayer.isValid())
        extent = loadedLayer.extent()
        self.assertEqual(extent.toString(1),
                         QgsRectangle(0.0, 0.5, 5.5, 6.0).toString(1))

    def test_insert_pk_escaping(self):
        """
        Test that inserting features works with complex pk name
        see https://github.com/qgis/QGIS/issues/42290
        """
        md = QgsProviderRegistry.instance().providerMetadata('mssql')
        conn = md.createConnection(self.dbconn, {})

        conn.execSql('DROP TABLE IF EXISTS qgis_test.test_complex_pk_name')
        conn.execSql('CREATE TABLE qgis_test.test_complex_pk_name ([test-field] int)')

        uri = '{} table="qgis_test"."test_complex_pk_name" sql='.format(self.dbconn)
        vl = QgsVectorLayer(uri, '', 'mssql')
        self.assertTrue(vl.isValid())

        self.assertEqual(vl.primaryKeyAttributes(), [0])

        vl.startEditing()
        f = QgsFeature(vl.fields())
        f.setAttributes([1])
        self.assertTrue(vl.addFeature(f))
        self.assertTrue(vl.commitChanges())

        vl = QgsVectorLayer(uri, '', 'mssql')
        features = list(vl.getFeatures())
        self.assertEqual([f['test-field'] for f in features], [1])


if __name__ == '__main__':
    unittest.main()
