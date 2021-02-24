# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsServer GetFeatureInfo WMS with PG.

From build dir, run: ctest -R PyQgsServerWMSGetFeatureInfoPG -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '22/01/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import tempfile

# Needed on Qt 5 so that the serialization of XML is consistent among all
# executions
os.environ['QT_HASH_SEED'] = '1'

import re
import urllib.parse
import json

from qgis.testing import unittest

from test_qgsserver_wms import TestQgsServerWMSTestBase
from qgis.core import QgsProject, QgsVectorLayer, QgsFeatureRequest, QgsExpression, QgsProviderRegistry
from qgis.server import QgsBufferServerRequest, QgsBufferServerResponse


class TestQgsServerWMSGetFeatureInfoPG(TestQgsServerWMSTestBase):
    """QGIS Server WMS Tests for GetFeatureInfo request"""

    @classmethod
    def setUpClass(cls):

        super().setUpClass()

        if 'QGIS_PGTEST_DB' in os.environ:
            cls.dbconn = os.environ['QGIS_PGTEST_DB']
        else:
            cls.dbconn = 'service=qgis_test dbname=qgis_test sslmode=disable '

        # Test layer
        md = QgsProviderRegistry.instance().providerMetadata('postgres')
        uri = cls.dbconn + ' dbname=qgis_test sslmode=disable '
        conn = md.createConnection(uri, {})
        conn.executeSql('DROP TABLE IF EXISTS "qgis_test"."someDataLong" CASCADE')
        conn.executeSql('SELECT * INTO "qgis_test"."someDataLong" FROM "qgis_test"."someData"')
        conn.executeSql('ALTER TABLE "qgis_test"."someDataLong" ALTER COLUMN "pk" TYPE bigint')
        conn.executeSql('ALTER TABLE "qgis_test"."someDataLong" ALTER COLUMN "pk" TYPE bigint')
        conn.executeSql('ALTER TABLE "qgis_test"."someDataLong" ALTER COLUMN "pk" SET NOT NULL')
        conn.executeSql('CREATE UNIQUE INDEX  someDataLongIdx ON "qgis_test"."someDataLong" ("pk")')

        cls.vlconn = cls.dbconn + ' sslmode=disable key=\'pk\' checkPrimaryKeyUnicity=0 srid=4326 type=POINT table="qgis_test"."someDataLong" (geom) sql='

        # Create another layer with multiple PKs
        conn.executeSql('DROP TABLE IF EXISTS "qgis_test"."multiple_pks"')
        conn.executeSql(
            'CREATE TABLE "qgis_test"."multiple_pks" ( pk1 bigint not null, pk2 bigint not null, name text not null, geom geometry(POINT,4326), PRIMARY KEY ( pk1, pk2 ) )')
        conn.executeSql(
            'INSERT INTO "qgis_test"."multiple_pks" VALUES ( 1, 1, \'1-1\', ST_GeomFromText(\'point(7 45)\', 4326))')
        conn.executeSql(
            'INSERT INTO "qgis_test"."multiple_pks" VALUES ( 1, 2, \'1-2\', ST_GeomFromText(\'point(8 46)\', 4326))')

        cls.vlconn_multiplepks = cls.dbconn + \
            " sslmode=disable key='pk1,pk2' estimatedmetadata=true srid=4326 type=Point checkPrimaryKeyUnicity='0' table=\"qgis_test\".\"multiple_pks\" (geom)"

    def _baseFilterTest(self, info_format):

        vl = QgsVectorLayer(self.vlconn, 'someData', 'postgres')
        self.assertTrue(vl.isValid())

        # Pre-filtered
        vl2 = QgsVectorLayer(self.vlconn, 'someData', 'postgres')
        self.assertTrue(vl2.isValid())
        [f for f in vl2.getFeatures(QgsFeatureRequest(QgsExpression('pk > 2')))]

        base_features_url = ('http://qgis/?SERVICE=WMS&REQUEST=GetFeatureInfo&' +
                             'LAYERS=someData&STYLES=&' +
                             r'INFO_FORMAT={}&' +
                             'SRS=EPSG%3A4326&' +
                             'QUERY_LAYERS=someData&X=-1&Y=-1&' +
                             'FEATURE_COUNT=100&'
                             'FILTER=someData')

        two_feature_url = base_features_url + urllib.parse.quote(':"pk" = 2')

        p = QgsProject()
        p.addMapLayers([vl])

        url = two_feature_url.format(urllib.parse.quote(info_format))

        req = QgsBufferServerRequest(url)
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, p)
        reference_body = bytes(res.body()).decode('utf8')

        # Pre-filter
        p = QgsProject()
        p.addMapLayers([vl2])

        req = QgsBufferServerRequest(url)
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, p)
        two_feature_body = bytes(res.body()).decode('utf8')

        self.assertEqual(reference_body, two_feature_body, info_format)

    def testGetFeatureInfoFilterPg(self):
        """Test issue GH #41124"""

        self._baseFilterTest('text/plain')
        self._baseFilterTest('text/html')
        self._baseFilterTest('text/xml')
        self._baseFilterTest('application/json')
        self._baseFilterTest('application/vnd.ogc.gml')

    def testMultiplePks(self):
        """Test issue GH #41786"""

        vl = QgsVectorLayer(self.vlconn_multiplepks, 'someData', 'postgres')
        self.assertTrue(vl.isValid())
        p = QgsProject()
        p.addMapLayers([vl])

        json_features_url = ('http://qgis/?SERVICE=WMS&REQUEST=GetFeatureInfo&' +
                             'LAYERS=someData&STYLES=&' +
                             'INFO_FORMAT=application/json&' +
                             'SRS=EPSG%3A4326&' +
                             'QUERY_LAYERS=someData&X=-1&Y=-1&' +
                             'FEATURE_COUNT=100&')

        req = QgsBufferServerRequest(json_features_url)
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, p)
        j = json.loads(bytes(res.body()).decode('utf8'))
        self.assertEqual(j, {'features': [{'geometry': None,
                                           'id': 'someData.1@@1',
                                           'properties': {'name': '1-1', 'pk1': 1, 'pk2': 1},
                                           'type': 'Feature'},
                                          {'geometry': None,
                                           'id': 'someData.1@@2',
                                           'properties': {'name': '1-2', 'pk1': 1, 'pk2': 2},
                                           'type': 'Feature'}],
                             'type': 'FeatureCollection'})


if __name__ == '__main__':
    unittest.main()
