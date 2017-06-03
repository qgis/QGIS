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

from qgis.core import QgsVectorLayer, QgsFeatureRequest, QgsRectangle

from qgis.PyQt.QtCore import QSettings, QDate, QTime, QDateTime, QVariant

from utilities import unitTestDataPath
from qgis.testing import start_app, unittest
from providertestbase import ProviderTestCase

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsMssqlProvider(unittest.TestCase, ProviderTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.dbconn = u"dbname='gis' host=localhost\sqlexpress"
        if 'QGIS_MSSQLTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_MSSQLTEST_DB']
        # Create test layers
        cls.vl = QgsVectorLayer(
            cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POINT table="qgis_test"."someData" (geom) sql=', 'test', 'mssql')
        assert(cls.vl.isValid())
        cls.provider = cls.vl.dataProvider()
        cls.poly_vl = QgsVectorLayer(
            cls.dbconn + ' sslmode=disable key=\'pk\' srid=4326 type=POLYGON table="qgis_test"."some_poly_data" (geom) sql=', 'test', 'mssql')
        assert(cls.poly_vl.isValid())
        cls.poly_provider = cls.poly_vl.dataProvider()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""

    def enableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', True)

    def disableCompiler(self):
        QSettings().setValue(u'/qgis/compileExpressions', False)

    def uncompiledFilters(self):
        filters = set(['"name" NOT LIKE \'Ap%\'',
                       '"name" IS NULL',
                       '"name" IS NOT NULL',
                       '"name" NOT ILIKE \'QGIS\'',
                       '"name" NOT ILIKE \'pEAR\'',
                       'name <> \'Apple\'',
                       '"name" <> \'apple\'',
                       '(name = \'Apple\') is not null',
                       '"name" || \' \' || "cnt" = \'Orange 100\'',
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
                       'not name IS NULL',
                       'not name = \'Apple\'',
                       'not name = \'Apple\' or name = \'Apple\'',
                       'not name = \'Apple\' or not name = \'Apple\'',
                       'not name = \'Apple\' and pk = 4',
                       'not name = \'Apple\' and not pk = 4',
                       'intersects($geometry,geom_from_wkt( \'Polygon ((-72.2 66.1, -65.2 66.1, -65.2 72.0, -72.2 72.0, -72.2 66.1))\'))'])
        return filters

    def partiallyCompiledFilters(self):
        return set(['name ILIKE \'QGIS\'',
                    'name = \'Apple\'',
                    'name = \'apple\'',
                    'name LIKE \'Apple\'',
                    'name LIKE \'aPple\'',
                    '"name"="name2"',
                    'name ILIKE \'aPple\'',
                    'name ILIKE \'%pp%\'',
                    '"name" || \' \' || "name" = \'Orange Orange\''])

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

        date_idx = vl.fieldNameIndex('date_field')
        assert isinstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 3, 4))
        time_idx = vl.fieldNameIndex('time_field')
        assert isinstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 52))
        datetime_idx = vl.fieldNameIndex('datetime_field')
        assert isinstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(f.attributes()[datetime_idx], QDateTime(
            QDate(2004, 3, 4), QTime(13, 41, 52)))

    def testInvalidGeometries(self):
        """ Test what happens when SQL Server is a POS and throws an exception on encountering an invalid geometry """
        vl = QgsVectorLayer('%s srid=4167 type=POLYGON table="qgis_test"."invalid_polys" (ogr_geometry) sql=' %
                            (self.dbconn), "testinvalid", "mssql")
        assert(vl.isValid())

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
        assert(vl.isValid())
        self.assertEqual(vl.dataProvider().extent().toString(1), QgsRectangle(173.954, -41.513, 173.967, -41.502).toString(1))


if __name__ == '__main__':
    unittest.main()
