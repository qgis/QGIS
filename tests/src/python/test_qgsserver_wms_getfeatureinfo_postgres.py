"""QGIS Unit tests for QgsServer GetFeatureInfo WMS with PG.

From build dir, run: ctest -R PyQgsServerWMSGetFeatureInfoPG -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "22/01/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

import os

# Needed on Qt 5 so that the serialization of XML is consistent among all
# executions
os.environ["QT_HASH_SEED"] = "1"

import json
import urllib.parse
import xml.etree.ElementTree as ET

from qgis.core import (
    QgsExpression,
    QgsFeature,
    QgsFeatureRequest,
    QgsGeometry,
    QgsProject,
    QgsProviderRegistry,
    QgsVectorLayer,
)
from qgis.server import QgsBufferServerRequest, QgsBufferServerResponse, QgsServer
from qgis.testing import unittest
from test_qgsserver_wms import TestQgsServerWMSTestBase


class TestQgsServerWMSGetFeatureInfoPG(TestQgsServerWMSTestBase):
    """QGIS Server WMS Tests for GetFeatureInfo request"""

    @classmethod
    def setUpClass(cls):

        super().setUpClass()

        if "QGIS_PGTEST_DB" in os.environ:
            cls.dbconn = os.environ["QGIS_PGTEST_DB"]
        else:
            cls.dbconn = "service=qgis_test dbname=qgis_test sslmode=disable "

        # Test layer
        md = QgsProviderRegistry.instance().providerMetadata("postgres")
        uri = cls.dbconn + " dbname=qgis_test sslmode=disable "
        conn = md.createConnection(uri, {})
        conn.executeSql('DROP TABLE IF EXISTS "qgis_test"."someDataLong" CASCADE')
        conn.executeSql(
            'SELECT * INTO "qgis_test"."someDataLong" FROM "qgis_test"."someData"'
        )
        conn.executeSql(
            'ALTER TABLE "qgis_test"."someDataLong" ALTER COLUMN "pk" TYPE bigint'
        )
        conn.executeSql(
            'ALTER TABLE "qgis_test"."someDataLong" ALTER COLUMN "pk" SET NOT NULL'
        )
        conn.executeSql(
            'CREATE UNIQUE INDEX  someDataLongIdx ON "qgis_test"."someDataLong" ("pk")'
        )

        cls.vlconn = (
            cls.dbconn
            + ' sslmode=disable key=\'pk\' checkPrimaryKeyUnicity=0 srid=4326 type=POINT table="qgis_test"."someDataLong" (geom) sql='
        )

        # Create another layer with multiple PKs
        conn.executeSql('DROP TABLE IF EXISTS "qgis_test"."multiple_pks"')
        conn.executeSql(
            'CREATE TABLE "qgis_test"."multiple_pks" ( pk1 bigint not null, pk2 bigint not null, name text not null, geom geometry(POINT,4326), PRIMARY KEY ( pk1, pk2 ) )'
        )
        conn.executeSql(
            "INSERT INTO \"qgis_test\".\"multiple_pks\" VALUES ( 1, 1, '1-1', ST_GeomFromText('point(7 45)', 4326))"
        )
        conn.executeSql(
            "INSERT INTO \"qgis_test\".\"multiple_pks\" VALUES ( 1, 2, '1-2', ST_GeomFromText('point(8 46)', 4326))"
        )

        cls.vlconn_multiplepks = (
            cls.dbconn
            + " sslmode=disable key='pk1,pk2' estimatedmetadata=true srid=4326 type=Point checkPrimaryKeyUnicity='0' table=\"qgis_test\".\"multiple_pks\" (geom)"
        )

        cls.conn = conn

    def _baseFilterTest(self, info_format):

        vl = QgsVectorLayer(self.vlconn, "someData", "postgres")
        self.assertTrue(vl.isValid())

        # Pre-filtered
        vl2 = QgsVectorLayer(self.vlconn, "someData", "postgres")
        self.assertTrue(vl2.isValid())
        [f for f in vl2.getFeatures(QgsFeatureRequest(QgsExpression("pk > 2")))]

        base_features_url = (
            "http://qgis/?SERVICE=WMS&REQUEST=GetFeatureInfo&"
            + "LAYERS=someData&STYLES=&"
            + r"INFO_FORMAT={}&"
            + "SRS=EPSG%3A4326&"
            + "QUERY_LAYERS=someData&X=-1&Y=-1&"
            + "FEATURE_COUNT=100&"
            "FILTER=someData"
        )

        two_feature_url = base_features_url + urllib.parse.quote(':"pk" = 2')

        p = QgsProject()
        p.addMapLayers([vl])

        url = two_feature_url.format(urllib.parse.quote(info_format))

        req = QgsBufferServerRequest(url)
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, p)
        reference_body = bytes(res.body()).decode("utf8")

        # Pre-filter
        p = QgsProject()
        p.addMapLayers([vl2])

        req = QgsBufferServerRequest(url)
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, p)
        two_feature_body = bytes(res.body()).decode("utf8")

        self.assertEqual(reference_body, two_feature_body, info_format)

    def testGetFeatureInfoFilterPg(self):
        """Test issue GH #41124"""

        self._baseFilterTest("text/plain")
        self._baseFilterTest("text/html")
        self._baseFilterTest("text/xml")
        self._baseFilterTest("application/json")
        self._baseFilterTest("application/vnd.ogc.gml")

    def testMultiplePks(self):
        """Test issue GH #41786"""

        vl = QgsVectorLayer(self.vlconn_multiplepks, "someData", "postgres")
        self.assertTrue(vl.isValid())
        p = QgsProject()
        p.addMapLayers([vl])

        json_features_url = (
            "http://qgis/?SERVICE=WMS&REQUEST=GetFeatureInfo&"
            + "LAYERS=someData&STYLES=&"
            + "INFO_FORMAT=application/json&"
            + "SRS=EPSG%3A4326&"
            + "QUERY_LAYERS=someData&X=-1&Y=-1&"
            + "FEATURE_COUNT=100&"
        )

        req = QgsBufferServerRequest(json_features_url)
        res = QgsBufferServerResponse()
        self.server.handleRequest(req, res, p)
        j = json.loads(bytes(res.body()).decode("utf8"))
        self.assertEqual(
            j,
            {
                "features": [
                    {
                        "featureType": "someData",
                        "geometry": None,
                        "id": "someData.1@@1",
                        "properties": {"name": "1-1", "pk1": 1, "pk2": 1},
                        "type": "Feature",
                    },
                    {
                        "featureType": "someData",
                        "geometry": None,
                        "id": "someData.1@@2",
                        "properties": {"name": "1-2", "pk1": 1, "pk2": 2},
                        "type": "Feature",
                    },
                ],
                "type": "FeatureCollection",
            },
        )

    def testGetFeatureInfoPostgresTypes(self):
        # compare json list output with file
        self.wms_request_compare(
            "GetFeatureInfo",
            "&layers=json"
            + "&info_format=text%2Fxml"
            + "&srs=EPSG%3A3857"
            + "&QUERY_LAYERS=json"
            + "&FILTER=json"
            + urllib.parse.quote(':"pk" = 1'),
            "get_postgres_types_json_list",
            "test_project_postgres_types.qgs",
            normalizeJson=True,
        )

        # compare dict output with file
        self.wms_request_compare(
            "GetFeatureInfo",
            "&layers=json"
            + "&info_format=text%2Fxml"
            + "&srs=EPSG%3A3857"
            + "&QUERY_LAYERS=json"
            + "&FILTER=json"
            + urllib.parse.quote(':"pk" = 2'),
            "get_postgres_types_json_dict",
            "test_project_postgres_types.qgs",
            normalizeJson=True,
        )

        # compare decoded json field list
        response_header, response_body, query_string = self.wms_request(
            "GetFeatureInfo",
            "&layers=json"
            + "&info_format=text%2Fxml"
            + "&srs=EPSG%3A3857"
            + "&QUERY_LAYERS=json"
            + "&FILTER=json"
            + urllib.parse.quote(':"pk" = 1'),
            "test_project_postgres_types.qgs",
        )
        root = ET.fromstring(response_body)
        for attribute in root.iter("Attribute"):
            if attribute.get("name") == "jvalue":
                self.assertIsInstance(json.loads(attribute.get("value")), list)
                self.assertEqual(json.loads(attribute.get("value")), [1, 2, 3])
                self.assertEqual(json.loads(attribute.get("value")), [1.0, 2.0, 3.0])
            if attribute.get("name") == "jbvalue":
                self.assertIsInstance(json.loads(attribute.get("value")), list)
                self.assertEqual(json.loads(attribute.get("value")), [4, 5, 6])
                self.assertEqual(json.loads(attribute.get("value")), [4.0, 5.0, 6.0])

        # compare decoded json field dict
        response_header, response_body, query_string = self.wms_request(
            "GetFeatureInfo",
            "&layers=json"
            + "&info_format=text%2Fxml"
            + "&srs=EPSG%3A3857"
            + "&QUERY_LAYERS=json"
            + "&FILTER=json"
            + urllib.parse.quote(':"pk" = 2'),
            "test_project_postgres_types.qgs",
        )
        root = ET.fromstring(response_body)
        for attribute in root.iter("Attribute"):
            if attribute.get("name") == "jvalue":
                self.assertIsInstance(json.loads(attribute.get("value")), dict)
                self.assertEqual(json.loads(attribute.get("value")), {"a": 1, "b": 2})
                self.assertEqual(
                    json.loads(attribute.get("value")), {"a": 1.0, "b": 2.0}
                )
            if attribute.get("name") == "jbvalue":
                self.assertIsInstance(json.loads(attribute.get("value")), dict)
                self.assertEqual(json.loads(attribute.get("value")), {"c": 4, "d": 5})
                self.assertEqual(
                    json.loads(attribute.get("value")), {"c": 4.0, "d": 5.0}
                )

    def testGetFeatureInfoTolerance(self):
        self.wms_request_compare(
            "GetFeatureInfo",
            "&layers=layer3&styles=&"
            + "VERSION=1.3.0&"
            + "info_format=text%2Fxml&"
            + "width=400&height=200"
            + "&bbox=913119.2,5605988.9,913316.0,5606047.4"
            + "&CRS=EPSG:3857"
            + "&FEATURE_COUNT=10"
            + "&WITH_GEOMETRY=False"
            + "&QUERY_LAYERS=layer3&I=193&J=100"
            + "&FI_POINT_TOLERANCE=0",
            "wms_getfeatureinfo_point_tolerance_0_text_xml",
            "test_project_values_postgres.qgz",
        )

        self.wms_request_compare(
            "GetFeatureInfo",
            "&layers=layer3&styles=&"
            + "VERSION=1.3.0&"
            + "info_format=text%2Fxml&"
            + "width=400&height=200"
            + "&bbox=913119.2,5605988.9,913316.0,5606047.4"
            + "&CRS=EPSG:3857"
            + "&FEATURE_COUNT=10"
            + "&WITH_GEOMETRY=False"
            + "&QUERY_LAYERS=layer3&I=193&J=100"
            + "&FI_POINT_TOLERANCE=20",
            "wms_getfeatureinfo_point_tolerance_20_text_xml",
            "test_project_values_postgres.qgz",
        )

        self.wms_request_compare(
            "GetFeatureInfo",
            "&layers=ls2d&styles=&"
            + "VERSION=1.3.0&"
            + "info_format=text%2Fxml&"
            + "width=400&height=200"
            + "&bbox=-50396.4,-2783.0,161715.8,114108.6"
            + "&CRS=EPSG:3857"
            + "&FEATURE_COUNT=10"
            + "&WITH_GEOMETRY=False"
            + "&QUERY_LAYERS=ls2d&I=153&J=147"
            + "&FI_LINE_TOLERANCE=0",
            "wms_getfeatureinfo_line_tolerance_0_text_xml",
            "test_project_values_postgres.qgz",
        )

        self.wms_request_compare(
            "GetFeatureInfo",
            "&layers=ls2d&styles=&"
            + "VERSION=1.3.0&"
            + "info_format=text%2Fxml&"
            + "width=400&height=200"
            + "&bbox=-50396.4,-2783.0,161715.8,114108.6"
            + "&CRS=EPSG:3857"
            + "&FEATURE_COUNT=10"
            + "&WITH_GEOMETRY=False"
            + "&QUERY_LAYERS=ls2d&I=153&J=147"
            + "&FI_LINE_TOLERANCE=20",
            "wms_getfeatureinfo_line_tolerance_20_text_xml",
            "test_project_values_postgres.qgz",
        )

        self.wms_request_compare(
            "GetFeatureInfo",
            "&layers=p2d&styles=&"
            + "VERSION=1.3.0&"
            + "info_format=text%2Fxml&"
            + "width=400&height=200"
            + "&bbox=-135832.0,-66482.4,240321.9,167300.4"
            + "&CRS=EPSG:3857"
            + "&FEATURE_COUNT=10"
            + "&WITH_GEOMETRY=False"
            + "&QUERY_LAYERS=p2d&I=206&J=144"
            + "&FI_POLYGON_TOLERANCE=0",
            "wms_getfeatureinfo_polygon_tolerance_0_text_xml",
            "test_project_values_postgres.qgz",
        )

        self.wms_request_compare(
            "GetFeatureInfo",
            "&layers=p2d&styles=&"
            + "VERSION=1.3.0&"
            + "info_format=text%2Fxml&"
            + "width=400&height=200"
            + "&bbox=-135832.0,-66482.4,240321.9,167300.4"
            + "&CRS=EPSG:3857"
            + "&FEATURE_COUNT=10"
            + "&WITH_GEOMETRY=False"
            + "&QUERY_LAYERS=p2d&I=206&J=144"
            + "&FI_POLYGON_TOLERANCE=20",
            "wms_getfeatureinfo_polygon_tolerance_20_text_xml",
            "test_project_values_postgres.qgz",
        )

    def testGetFeatureInfoValueRelationArray(self):
        """Test GetFeatureInfo on "value relation" widget with array field (multiple selections)"""
        mypath = self.testdata_path + "test_project_values_postgres.qgz"
        self.wms_request_compare(
            "GetFeatureInfo",
            "&layers=layer3&styles=&"
            + "VERSION=1.3.0&"
            + "info_format=text%2Fxml&"
            + "width=926&height=787&srs=EPSG%3A4326"
            + "&bbox=912217,5605059,914099,5606652"
            + "&CRS=EPSG:3857"
            + "&FEATURE_COUNT=10"
            + "&WITH_GEOMETRY=True"
            + "&QUERY_LAYERS=layer3&I=487&J=308",
            "wms_getfeatureinfo-values3-text-xml",
            "test_project_values_postgres.qgz",
        )

    def testGetFeatureInfoNullValues(self):

        self.conn.executeSql(
            'DROP TABLE IF EXISTS "qgis_test"."test_server_gfi_null_values"'
        )
        self.conn.executeSql("""
            CREATE TABLE "qgis_test"."test_server_gfi_null_values" (
                "id" SERIAL PRIMARY KEY,
                "name" VARCHAR(50),
                "value" INTEGER
            )
        """)

        # Add polygon geometry column
        self.conn.executeSql(
            "SELECT AddGeometryColumn('qgis_test', 'test_server_gfi_null_values', 'geom', 3857, 'POLYGON', 2)"
        )

        uri = (
            self.conn.uri()
            + ' sslmode=disable key=\'pk\' srid=3857 type=POLYGON  checkPrimaryKeyUnicity=0 table="qgis_test"."test_server_gfi_null_values" (geom) sql='
        )
        vl = QgsVectorLayer(uri, "gfi_test", "postgres")
        self.assertTrue(vl.isValid())
        f = QgsFeature(vl.fields())
        f.setGeometry(
            QgsGeometry.fromWkt(
                "POLYGON((100 100, 100 200, 200 200, 200 100, 100 100))"
            )
        )
        f.setAttribute("name", "test1")
        f.setAttribute("value", "1234")
        self.assertTrue(vl.dataProvider().addFeatures([f]))
        p = QgsProject()
        self.assertTrue(p.addMapLayer(vl))

        def _test_val(vl, name, value, expected):

            self.assertTrue(
                vl.dataProvider().changeAttributeValues(
                    {
                        1: {
                            vl.fields().indexFromName("name"): name,
                            vl.fields().indexFromName("value"): value,
                        }
                    }
                )
            )

            server = QgsServer()
            uri = (
                "http://www.qgis.org/?SERVICE=WMS&REQUEST=GetFeatureInfo&"
                + "LAYERS=gfi_test&styles=&"
                + "VERSION=1.3.0&"
                + "INFO_FORMAT=application/json&"
                + "WIDTH=100&HEIGHT=100"
                + "&BBOX=90,90, 200,200"
                + "&CRS=EPSG:3857"
                + "&FEATURE_COUNT=10"
                + "&WITH_GEOMETRY=False"
                + "&QUERY_LAYERS=gfi_test&I=50&J=50"
            )

            request = QgsBufferServerRequest(uri)

            response = QgsBufferServerResponse()
            server.handleRequest(request, response, p)

            j = json.loads(bytes(response.body()).decode("utf8"))
            self.assertEqual(j["features"][0]["properties"], expected)

            # Same request with XML format
            request = QgsBufferServerRequest(
                uri.replace("INFO_FORMAT=application/json", "INFO_FORMAT=text/xml")
            )
            response = QgsBufferServerResponse()
            server.handleRequest(request, response, p)

            xml = bytes(response.body()).decode("utf8")
            exp_val = value if value is not None else "NULL"
            exp_name = name if name is not None else "NULL"
            # Current behavior for XML is to return 'NULL' string for null values
            self.assertIn(
                f'<Attribute name="value" value="{exp_val}"/>',
                xml,
                f"Expected value '{exp_val}' in XML response, got: {xml}",
            )
            self.assertIn(
                f'<Attribute name="name" value="{exp_name}"/>',
                xml,
                f"Expected name '{exp_name}' in XML response, got: {xml}",
            )

            # Try GML
            request = QgsBufferServerRequest(
                uri.replace(
                    "INFO_FORMAT=application/json",
                    "INFO_FORMAT=application/vnd.ogc.gml",
                )
            )
            response = QgsBufferServerResponse()
            server.handleRequest(request, response, p)
            xml = bytes(response.body()).decode("utf8")

            if value is not None:
                self.assertIn(
                    f"<qgs:value>{value}</qgs:value>",
                    xml,
                    f"Expected value '{value}' in GML response, got: {xml}",
                )
            else:
                self.assertIn(
                    f'<qgs:value xsi:nil="true"/>',
                    xml,
                    f"Expected value '{value}' in GML response, got: {xml}",
                )

            if name is not None:
                self.assertIn(
                    f"<qgs:name>{name}</qgs:name>",
                    xml,
                    f"Expected name '{name}' in GML response, got: {xml}",
                )
            else:
                self.assertIn(
                    f'<qgs:name xsi:nil="true"/>',
                    xml,
                    f"Expected name '{name}' in GML response, got: {xml}",
                )

        _test_val(vl, None, None, {"id": 1, "name": None, "value": None})
        _test_val(vl, "test1", 1234, {"id": 1, "name": "test1", "value": 1234})
        _test_val(vl, "", 0, {"id": 1, "name": "", "value": 0})


if __name__ == "__main__":
    unittest.main()
