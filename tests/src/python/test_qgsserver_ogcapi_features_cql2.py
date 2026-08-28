"""QGIS Unit tests for QgsServer OGC API Features CQL2 filtering.

From build dir, run: ctest -R PyQgsServerApiFeaturesCql2 -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Jürgen E. Fischer"
__date__ = "2026-08-06"
__copyright__ = "Copyright 2026, The QGIS Project"

import json
import os

# Deterministic XML
os.environ["QT_HASH_SEED"] = "1"

from qgis.core import (
    QgsApplication,
    QgsFeature,
    QgsGeometry,
    QgsProject,
    QgsVectorLayer,
)
from qgis.server import QgsBufferServerRequest, QgsBufferServerResponse
from qgis.testing import unittest
from test_qgsserver_api import QgsServerAPITestBase


class QgsServerOgcApiFeaturesCql2Test(QgsServerAPITestBase):
    """QGIS API server CQL2 filtering tests"""

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

    def setUp(self):
        super().setUp()
        # Default url has changed in QGIS 4 stick to /wfs3 for the tests
        os.environ.update({"QGIS_SERVER_API_WFS3_ROOT_PATH": "/wfs3"})
        iface = self.server.serverInterface()
        iface.reloadSettings()
        iface.serviceRegistry().cleanUp()
        iface.serviceRegistry().init(QgsApplication.libexecPath() + "server", iface)

    def _make_layer(self, fields_def, wkb_type="Point", crs=4326, name="testlayer"):
        """Create a memory vector layer with given fields and return it."""
        crs_str = f"EPSG:{crs}" if isinstance(crs, int) else crs
        uri = f"{wkb_type}?crs={crs_str}"
        for f in fields_def:
            uri += f"&field={f}"
        layer = QgsVectorLayer(uri, name, "memory")
        self.assertTrue(layer.isValid(), f"Layer creation failed: {name}")
        return layer

    def _add_feature(self, layer, attrs, geom_wkt):
        """Add a feature to the layer and return its id."""
        f = QgsFeature(layer.fields())
        for i, val in enumerate(attrs):
            f.setAttribute(i, val)
        if geom_wkt is not None:
            f.setGeometry(QgsGeometry.fromWkt(geom_wkt))
        layer.dataProvider().addFeature(f)
        return f.id()

    def _item_count(self, url, project):
        """Return number of features returned by an items request."""
        j = self._getJsonResponse(url, project)
        return len(j.get("features", []))

    def _request(self, url, project, expected_status=200):
        """Send a request and return (status_code, body_dict)."""
        request = QgsBufferServerRequest(url)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        body = bytes(response.body()).decode("utf8")
        try:
            body_json = json.loads(body)
        except json.JSONDecodeError:
            body_json = body
        self.assertEqual(
            response.statusCode(),
            expected_status,
            f"Expected status {expected_status}, got {response.statusCode()}: {body}",
        )
        return response.statusCode(), body_json

    def _getJsonResponse(self, url, project, expected_error=None):
        request = QgsBufferServerRequest(url)
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response, project)
        if expected_error is None:
            self.assertEqual(
                response.statusCode(),
                200,
                f"Request failed with status {response.statusCode()} and message: {bytes(response.body()).decode('utf8')} for URL: {url}",
            )
        else:
            self.assertEqual(
                response.statusCode(),
                400,
                f"Request failed with status {response.statusCode()} and message: {bytes(response.body()).decode('utf8')} for URL: {url}",
            )
            return None

        response_str = bytes(response.body()).decode("utf8")
        j = json.loads(response_str)

        if expected_error is not None:
            self.assertEqual(j[0]["description"], expected_error)
        return j

    def _setup_filter_layer(self):
        """Create a standard filter test layer with fid, name, val fields."""
        layer = self._make_layer(
            [
                "fid:integer",
                "name:string",
                "val:double",
            ]
        )
        self._add_feature(layer, [1, "alpha", 1.5], "POINT(1 1)")
        self._add_feature(layer, [2, "beta", 2.5], "POINT(2 2)")
        self._add_feature(layer, [3, "gamma", 3.5], "POINT(3 3)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])
        return project, layer

    # ------------------------------------------------------------------ #
    # OAPIF filtering tests (CQL2-Text, QGIS expression, conformance, …) #
    # ------------------------------------------------------------------ #

    # Conformance

    def test_conformance_filtering(self):
        """Conformance endpoint must include filtering CQL2 classes."""
        status, j = self._request("http://server.qgis.org/wfs3/conformance", None)
        conforms = j.get("conformsTo", [])
        self.assertIn(
            "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/queryables",
            conforms,
        )
        self.assertIn(
            "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/queryables-query-parameters",
            conforms,
        )
        self.assertIn(
            "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/filter",
            conforms,
        )
        self.assertIn(
            "http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/features-filter",
            conforms,
        )
        self.assertIn(
            "http://www.opengis.net/spec/cql2/1.0/conf/basic-cql2",
            conforms,
        )
        self.assertIn(
            "http://www.opengis.net/spec/cql2/1.0/conf/basic-spatial-functions",
            conforms,
        )

    # Queryables

    def test_queryables_json(self):
        """Queryables endpoint returns JSON with property types."""
        layer = self._make_layer(
            [
                "fid:integer",
                "name:string",
                "val:double",
                "flag:bool",
            ]
        )
        self._add_feature(layer, [1, "alpha", 1.5, True], "POINT(1 1)")
        self._add_feature(layer, [2, "beta", 2.5, False], "POINT(2 2)")
        self._add_feature(layer, [3, "gamma", 3.5, True], "POINT(3 3)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        j = self._getJsonResponse(
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/queryables.json",
            project,
        )
        self.assertEqual(j["type"], "object")
        props = j.get("properties", {})
        # Geometry property
        geom_key = None
        for k, v in props.items():
            if v.get("x-ogc-role") == "primary-geometry":
                geom_key = k
                break
        self.assertIsNotNone(geom_key, "No primary-geometry property found")
        self.assertEqual(props[geom_key]["format"], "geometry-point")
        # Field types
        self.assertEqual(props["fid"]["type"], "integer")
        self.assertEqual(props["name"]["type"], "string")
        self.assertEqual(props["val"]["type"], "number")
        self.assertEqual(props["flag"]["type"], "boolean")

    def test_queryables_in_collection_links(self):
        """Collection JSON includes a queryables link."""
        layer = self._make_layer(["fid:integer", "name:string"])
        self._add_feature(layer, [1, "hello"], "POINT(0 0)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        j = self._getJsonResponse(
            f"http://server.qgis.org/wfs3/collections/{layer.name()}.json",
            project,
        )
        links = j.get("links", [])
        queryable_links = [link for link in links if link.get("rel") == "queryables"]
        self.assertGreater(len(queryable_links), 0, "No queryables link in collection")

    # Layer existence

    def test_layer_exists_in_collections(self):
        """The testlayer should appear in the collections list."""
        layer = self._make_layer(["fid:integer", "name:string"])
        self._add_feature(layer, [1, "hello"], "POINT(0 0)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections.json", project
        )
        collection_ids = [c["id"] for c in j.get("collections", [])]
        self.assertIn("testlayer", collection_ids)

    def test_layer_exists_in_project(self):
        """The testlayer should be present in the project map layers."""
        layer = self._make_layer(["fid:integer"])

        project = QgsProject()
        project.addMapLayer(layer)

        layer_ids = project.mapLayers()
        self.assertIn(layer.id(), layer_ids)
        self.assertEqual(layer_ids[layer.id()].name(), "testlayer")

    # filter-lang=qgis tests

    def test_filter_qgis_eq(self):
        """filter-lang=qgis with simple equality."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=qgis&filter=%22fid%22=2"
        )
        self.assertEqual(self._item_count(url, project), 1)

    def test_filter_qgis_gt(self):
        """filter-lang=qgis with greater-than."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=qgis&filter=%22val%22>2"
        )
        self.assertEqual(self._item_count(url, project), 2)

    def test_filter_qgis_like(self):
        """filter-lang=qgis with LIKE."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=qgis&filter=%22name%22LIKE%27a%25%27"
        )
        self.assertEqual(self._item_count(url, project), 1)

    def test_filter_qgis_and(self):
        """filter-lang=qgis with AND."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=qgis&filter=%22fid%22>1 AND %22fid%22<3"
        )
        self.assertEqual(self._item_count(url, project), 1)

    # CQL2 basic comparison operators

    def test_cql2_eq(self):
        """CQL2 equality filter."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid=2"
        )
        self.assertEqual(self._item_count(url, project), 1)

    def test_cql2_ne(self):
        """CQL2 not-equal filter."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid<>2"
        )
        self.assertEqual(self._item_count(url, project), 2)

    def test_cql2_lt(self):
        """CQL2 less-than filter."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid<2"
        )
        self.assertEqual(self._item_count(url, project), 1)

    def test_cql2_gt(self):
        """CQL2 greater-than filter."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid>2"
        )
        self.assertEqual(self._item_count(url, project), 1)

    def test_cql2_le(self):
        """CQL2 less-or-equal filter."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid<=2"
        )
        self.assertEqual(self._item_count(url, project), 2)

    def test_cql2_ge(self):
        """CQL2 greater-or-equal filter."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid>=2"
        )
        self.assertEqual(self._item_count(url, project), 2)

    def test_cql2_string_eq(self):
        """CQL2 string equality filter."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=name='beta'"
        )
        self.assertEqual(self._item_count(url, project), 1)

    def test_cql2_is_null(self):
        """CQL2 IS NULL filter."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid IS NULL"
        )
        self.assertEqual(self._item_count(url, project), 0)

    def test_cql2_is_not_null(self):
        """CQL2 IS NOT NULL filter."""
        project, layer = self._setup_filter_layer()
        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid IS NOT NULL"
        )
        self.assertEqual(self._item_count(url, project), 3)

    def test_cql2_boolean(self):
        """CQL2 boolean literal filter."""
        layer = self._make_layer(["fid:integer", "flag:bool"])
        self._add_feature(layer, [1, True], "POINT(1 1)")
        self._add_feature(layer, [2, False], "POINT(2 2)")
        self._add_feature(layer, [3, True], "POINT(3 3)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=flag=TRUE"
        )
        self.assertEqual(self._item_count(url, project), 2)

    # CQL2 logical operators

    def test_cql2_and(self):
        """CQL2 AND filter."""
        layer = self._make_layer(["fid:integer", "val:double"])
        self._add_feature(layer, [1, 1.0], "POINT(1 1)")
        self._add_feature(layer, [2, 2.0], "POINT(2 2)")
        self._add_feature(layer, [3, 3.0], "POINT(3 3)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid>1 AND fid<3"
        )
        self.assertEqual(self._item_count(url, project), 1)

    def test_cql2_or(self):
        """CQL2 OR filter."""
        layer = self._make_layer(["fid:integer", "val:double"])
        self._add_feature(layer, [1, 1.0], "POINT(1 1)")
        self._add_feature(layer, [2, 2.0], "POINT(2 2)")
        self._add_feature(layer, [3, 3.0], "POINT(3 3)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid=1 OR fid=3"
        )
        self.assertEqual(self._item_count(url, project), 2)

    def test_cql2_not(self):
        """CQL2 NOT filter."""
        layer = self._make_layer(["fid:integer", "val:double"])
        self._add_feature(layer, [1, 1.0], "POINT(1 1)")
        self._add_feature(layer, [2, 2.0], "POINT(2 2)")
        self._add_feature(layer, [3, 3.0], "POINT(3 3)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=NOT(fid=2)"
        )
        self.assertEqual(self._item_count(url, project), 2)

    def test_cql2_parentheses_precedence(self):
        """CQL2 operator precedence with parentheses."""
        layer = self._make_layer(["fid:integer", "val:double"])
        self._add_feature(layer, [1, 1.0], "POINT(1 1)")
        self._add_feature(layer, [2, 2.0], "POINT(2 2)")
        self._add_feature(layer, [3, 3.0], "POINT(3 3)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=(fid=1 OR fid=2) AND val>1"
        )
        self.assertEqual(self._item_count(url, project), 1)

    # CQL2 IN and BETWEEN

    def test_cql2_in(self):
        """CQL2 IN filter."""
        layer = self._make_layer(["fid:integer"])
        for i in range(1, 6):
            self._add_feature(layer, [i], f"POINT({i} {i})")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid IN (1, 3, 5)"
        )
        self.assertEqual(self._item_count(url, project), 3)

    def test_cql2_not_in(self):
        """CQL2 NOT IN filter."""
        layer = self._make_layer(["fid:integer"])
        for i in range(1, 6):
            self._add_feature(layer, [i], f"POINT({i} {i})")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid NOT IN (1, 3, 5)"
        )
        self.assertEqual(self._item_count(url, project), 2)

    def test_cql2_between(self):
        """CQL2 BETWEEN filter (inclusive)."""
        layer = self._make_layer(["fid:integer"])
        for i in range(1, 6):
            self._add_feature(layer, [i], f"POINT({i} {i})")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid BETWEEN 2 AND 4"
        )
        self.assertEqual(self._item_count(url, project), 3)

    def test_cql2_not_between(self):
        """CQL2 NOT BETWEEN filter."""
        layer = self._make_layer(["fid:integer"])
        for i in range(1, 6):
            self._add_feature(layer, [i], f"POINT({i} {i})")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid NOT BETWEEN 2 AND 4"
        )
        self.assertEqual(self._item_count(url, project), 2)

    # CQL2 spatial operators

    def test_cql2_s_intersects(self):
        """CQL2 S_INTERSECTS filter."""
        layer = self._make_layer(["fid:integer"])
        self._add_feature(layer, [1], "POINT(0 0)")
        self._add_feature(layer, [2], "POINT(5 5)")
        self._add_feature(layer, [3], "POINT(10 10)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=S_INTERSECTS(var('geometry'), POLYGON((4 4, 6 4, 6 6, 4 6, 4 4)))"
        )
        self.assertEqual(self._item_count(url, project), 1)

    def test_cql2_s_within(self):
        """CQL2 S_WITHIN filter."""
        layer = self._make_layer(["fid:integer"])
        self._add_feature(layer, [1], "POINT(0 0)")
        self._add_feature(layer, [2], "POINT(5 5)")
        self._add_feature(layer, [3], "POINT(10 10)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=S_WITHIN(var('geometry'), POLYGON((4 4, 6 4, 6 6, 4 6, 4 4)))"
        )
        self.assertEqual(self._item_count(url, project), 1)

    def test_cql2_s_disjoint(self):
        """CQL2 S_DISJOINT filter."""
        layer = self._make_layer(["fid:integer"])
        self._add_feature(layer, [1], "POINT(0 0)")
        self._add_feature(layer, [2], "POINT(5 5)")
        self._add_feature(layer, [3], "POINT(10 10)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=S_DISJOINT(var('geometry'), POLYGON((4 4, 6 4, 6 6, 4 6, 4 4)))"
        )
        self.assertEqual(self._item_count(url, project), 2)

    # CQL2 LIKE

    def test_cql2_like(self):
        """CQL2 LIKE filter (case-sensitive)."""
        layer = self._make_layer(["fid:integer", "name:string"])
        self._add_feature(layer, [1, "Alpha"], "POINT(1 1)")
        self._add_feature(layer, [2, "beta"], "POINT(2 2)")
        self._add_feature(layer, [3, "Gamma"], "POINT(3 3)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=name LIKE 'b%'"
        )
        self.assertEqual(self._item_count(url, project), 1)

    # Error handling

    def test_invalid_filter_lang(self):
        """Unknown filter-lang returns 400."""
        layer = self._make_layer(["fid:integer"])
        self._add_feature(layer, [1], "POINT(1 1)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=sql&filter=fid=1"
        )
        status, body = self._request(url, project, expected_status=400)
        self.assertIn("Invalid filter language", str(body))

    def test_invalid_cql2_syntax(self):
        """Malformed CQL2 expression returns 400."""
        layer = self._make_layer(["fid:integer"])
        self._add_feature(layer, [1], "POINT(1 1)")

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        url = (
            f"http://server.qgis.org/wfs3/collections/{layer.name()}/items?"
            "filter-lang=cql2-text&filter=fid ==="
        )
        status, body = self._request(url, project, expected_status=400)
        self.assertIn("Invalid filter", str(body))


if __name__ == "__main__":
    unittest.main()
