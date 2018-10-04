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

from qgis.core import QgsSettings, QgsVectorLayer, QgsFeatureRequest

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
        cls.dbconn = "dbname='gis' host=localhost\sqlexpress"
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
            '"name"="name2"'
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
            'sqrt(pk) >= 2',
            'radians(cnt) < 2',
            'degrees(pk) <= 200',
            'abs(cnt) <= 200',
            'cos(pk) < 0',
            'sin(pk) < 0',
            'tan(pk) < 0',
            'acos(-1) < pk',
            'asin(1) < pk',
            'atan(3.14) < pk',
            'atan2(3.14, pk) < 1',
            'exp(pk) < 10',
            'ln(pk) <= 1',
            'log(3, pk) <= 1',
            'log10(pk) < 0.5',
            'pk < pi()',
            'round(3.14) <= pk',
            'round(0.314,1) * 10 = pk',
            'floor(3.14) <= pk',
            'ceil(3.14) <= pk',
            'pk < pi()',
            'round(cnt / 66.67) <= 2',
            'floor(cnt / 66.67) <= 2',
            'ceil(cnt / 66.67) <= 2',
            'pk < pi() / 2',
            'pk = char(51)',
            'pk = coalesce(NULL,3,4)',
            'lower(name) = \'apple\'',
            'upper(name) = \'APPLE\'',
            'name = trim(\'   Apple   \')',
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


if __name__ == '__main__':
    unittest.main()
