"""QGIS Unit tests for QgsServer Landing Page Plugin.

From build dir, run: ctest -R PyQgsServerLandingPage -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "03/08/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

import json
import os
import shutil

# Deterministic XML
os.environ["QT_HASH_SEED"] = "1"

from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsFeature,
    QgsGeometry,
    QgsProject,
    QgsVectorLayer,
)
from qgis.PyQt import QtCore
from qgis.server import (
    QgsAccessControlFilter,
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
)
from qgis.testing import unittest
from test_qgsserver_api import QgsServerAPITestBase
from utilities import unitTestDataPath


class QgsServerLandingPageTest(QgsServerAPITestBase):
    """QGIS Server Landing Page tests"""

    # Set to True in child classes to re-generate reference files for this class
    regenerate_api_reference = False

    @classmethod
    def setUpClass(cls):
        super().setUpClass()

        cls.temp_dir = QtCore.QTemporaryDir()

        temp_dir = cls.temp_dir.path()
        cls.directories = [
            os.path.join(temp_dir, "landingpage", "projects"),
            os.path.join(temp_dir, "landingpage", "projects2"),
        ]
        shutil.copytree(
            os.path.join(unitTestDataPath("qgis_server"), "landingpage"),
            os.path.join(temp_dir, "landingpage"),
        )

    def setUp(self):
        """Setup env"""

        super().setUp()
        try:
            del os.environ["QGIS_SERVER_DISABLED_APIS"]
        except:
            pass

        try:
            del os.environ["QGIS_SERVER_LANDING_PAGE_PREFIX"]
        except:
            pass

        os.environ["QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES"] = "||".join(
            self.directories
        )

        if not os.environ.get("TRAVIS", False):
            os.environ["QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS"] = (
                "postgresql://localhost:5432?sslmode=disable&dbname=landing_page_test&schema=public"
            )

    def test_landing_page_redirects(self):
        """Test landing page redirects"""

        request = QgsBufferServerRequest("http://server.qgis.org/")
        request.setHeader("Accept", "application/json")
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)
        self.assertEqual(
            response.headers()["Location"], "http://server.qgis.org/index.json"
        )

        request = QgsBufferServerRequest("http://server.qgis.org")
        request.setHeader("Accept", "application/json")
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)
        self.assertEqual(
            response.headers()["Location"], "http://server.qgis.org/index.json"
        )

        response = QgsBufferServerResponse()
        request.setHeader("Accept", "text/html")
        self.server.handleRequest(request, response)
        self.assertEqual(
            response.headers()["Location"], "http://server.qgis.org/index.html"
        )

    def compareProjects(self, actual, expected, expected_path):
        """Order-agnostic project comparison"""

        actual_raw = self.normalize_json(actual).split("\n")
        expected_raw = self.normalize_json(expected).split("\n")
        actual_raw = "\n".join(actual_raw[actual_raw.index("") + 1 :])
        expected_raw = "\n".join(expected_raw[expected_raw.index("") + 1 :])
        actual_j = json.loads(actual_raw)
        expected_j = json.loads(expected_raw)
        actual_projects = {p["title"]: p for p in actual_j["projects"]}
        expected_projects = {p["title"]: p for p in expected_j["projects"]}

        if self.regenerate_api_reference:
            # Try to change timestamp
            try:
                content = actual.split("\n")
                j = "".join(content[content.index("") + 1 :])
                j = json.loads(j)
                j["timeStamp"] = "2019-07-05T12:27:07Z"
                actual = (
                    "\n".join(content[:2])
                    + "\n"
                    + json.dumps(j, ensure_ascii=False, indent=2)
                )
            except:
                pass
            f = open(expected_path.encode("utf8"), "w+", encoding="utf8")
            f.write(actual)
            f.close()
            print(f"Reference file {expected_path.encode('utf8')} regenerated!")

        for title in expected_projects.keys():
            self.assertLinesEqual(
                json.dumps(actual_projects[title], indent=4),
                json.dumps(expected_projects[title], indent=4),
                expected_path.encode("utf8"),
            )

    @staticmethod
    def _layer_ids_from_toc(node):
        layer_ids = set()
        if isinstance(node, dict):
            layer_id = node.get("id")
            if layer_id:
                layer_ids.add(layer_id)
            for child in node.get("children", []):
                layer_ids |= QgsServerLandingPageTest._layer_ids_from_toc(child)
        elif isinstance(node, list):
            for child in node:
                layer_ids |= QgsServerLandingPageTest._layer_ids_from_toc(child)
        return layer_ids

    class _RestrictedLayerAccessControl(QgsAccessControlFilter):
        def __init__(self, server_iface, excluded_layer_ids):
            self._excluded_layer_ids = set(excluded_layer_ids)
            super().__init__(server_iface)

        def layerPermissions(self, layer):
            rights = QgsAccessControlFilter.LayerPermissions()
            rights.canRead = layer.id() not in self._excluded_layer_ids
            rights.canUpdate = rights.canRead
            rights.canInsert = rights.canRead
            rights.canDelete = rights.canRead
            return rights

    def test_landing_page_json(self):
        """Test landing page in JSON format"""

        request = QgsBufferServerRequest("http://server.qgis.org/index.json")
        request.setBaseUrl(QtCore.QUrl("http://server.qgis.org/"))
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)
        j_actual = "Content-Type: application/json\n\n"
        j_actual += bytes(response.body()).decode("utf8)")

        if not os.environ.get("TRAVIS", False):
            expected_path = os.path.join(
                unitTestDataPath("qgis_server"),
                "landingpage",
                "test_landing_page_with_pg_index.json",
            )
        else:
            expected_path = os.path.join(
                unitTestDataPath("qgis_server"),
                "landingpage",
                "test_landing_page_index.json",
            )

        j_expected = open(expected_path).read()
        self.compareProjects(j_actual, j_expected, expected_path)

    def test_project_json(self):
        """Test landing page project call in JSON format"""

        # Get hashes for test projects
        request = QgsBufferServerRequest("http://server.qgis.org/index.json")
        request.setHeader("Accept", "application/json")
        response = QgsBufferServerResponse()
        self.server.handleRequest(request, response)

        j = json.loads(bytes(response.body()))
        test_projects = {p["id"]: p["title"].replace(" ", "_") for p in j["projects"]}

        for identifier, name in test_projects.items():
            request = QgsBufferServerRequest("http://server.qgis.org/map/" + identifier)
            request.setHeader("Accept", "application/json")
            self.compareApi(
                request,
                None,
                f"test_project_{name.replace('.', '_')}.json",
                subdir="landingpage",
            )

    def test_landing_page_json_empty(self):
        """Test landing page in JSON format with no projects"""

        os.environ["QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES"] = ""
        os.environ["QGIS_SERVER_LANDING_PAGE_PROJECTS_PG_CONNECTIONS"] = ""
        request = QgsBufferServerRequest("http://server.qgis.org/index.json")
        self.compareApi(
            request, None, "test_landing_page_empty_index.json", subdir="landingpage"
        )

    def test_landing_page_prefix(self):

        os.environ["QGIS_SERVER_LANDING_PAGE_PREFIX"] = "/mylanding"

        def _test_error(uri):
            request = QgsBufferServerRequest(uri)
            request.setHeader("Accept", "application/json")
            response = QgsBufferServerResponse()
            self.server.handleRequest(request, response)
            self.assertEqual(
                self.strip_version_xmlns(bytes(response.body())),
                self.strip_version_xmlns(
                    b'<?xml version="1.0" encoding="UTF-8"?>\n<ServiceExceptionReport  >\n <ServiceException code="Service configuration error">Service unknown or unsupported. Current supported services (case-sensitive): WMS WFS WCS WMTS SampleService, or use a WFS3 (OGC API Features) endpoint</ServiceException>\n</ServiceExceptionReport>\n'
                ),
            )

        _test_error("http://server.qgis.org/index.json")
        _test_error("http://server.qgis.org/index.html")
        _test_error("http://server.qgis.org/")
        _test_error("http://server.qgis.org")

        def _test_valid(uri):
            request = QgsBufferServerRequest(uri)
            request.setHeader("Accept", "application/json")
            response = QgsBufferServerResponse()
            self.server.handleRequest(request, response)
            if "index" not in uri:
                self.assertEqual(
                    response.headers()["Location"],
                    "http://server.qgis.org/mylanding/index.json",
                )
            else:
                # Just check that it's valid json
                j = json.loads(bytes(response.body()))

        _test_valid("http://server.qgis.org/mylanding")
        _test_valid("http://server.qgis.org/mylanding/")
        _test_valid("http://server.qgis.org/mylanding/index.json")

        # Test redirects with prefix
        os.environ["QGIS_SERVER_LANDING_PAGE_PREFIX"] = "/ows/catalog"
        request = QgsBufferServerRequest("http://server.qgis.org/ows/catalog")
        response = QgsBufferServerResponse()
        request.setHeader("Accept", "text/html")
        self.server.handleRequest(request, response)
        self.assertEqual(
            response.headers()["Location"],
            "http://server.qgis.org/ows/catalog/index.html",
        )

        request = QgsBufferServerRequest("http://server.qgis.org/ows/catalog/")
        response = QgsBufferServerResponse()
        request.setHeader("Accept", "text/html")
        self.server.handleRequest(request, response)
        self.assertEqual(
            response.headers()["Location"],
            "http://server.qgis.org/ows/catalog/index.html",
        )

    def test_project_json_non_earth_crs(self):
        """Test landing page omits geographic_extent for non-Earth CRS projects"""
        tmpdir = QtCore.QTemporaryDir()

        # Create a minimal project with a non-Earth (Moon) CRS layer
        project = QgsProject()
        layer = QgsVectorLayer(
            "Point?crs=IAU_2015:30190&field=fid:integer", "moon_layer", "memory"
        )
        project.addMapLayers([layer])
        project.setCrs(QgsCoordinateReferenceSystem("IAU_2015:30190"))
        project.setTitle("Moon Test Project")
        f = QgsFeature(layer.fields())
        f.setGeometry(QgsGeometry.fromWkt("point(0.5 0.5)"))
        f.setAttributes([1])
        layer.dataProvider().addFeatures([f])
        project_path = os.path.join(tmpdir.path(), "moon_project.qgs")
        project.write(project_path)

        original_dirs = os.environ.get(
            "QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES", ""
        )
        try:
            os.environ["QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES"] = tmpdir.path()

            # Get the project list to find the non-Earth project's ID
            request = QgsBufferServerRequest("http://server.qgis.org/index.json")
            request.setHeader("Accept", "application/json")
            response = QgsBufferServerResponse()
            self.server.handleRequest(request, response)
            j = json.loads(bytes(response.body()))
            self.assertEqual(len(j["projects"]), 1)
            project_id = j["projects"][0]["id"]

            # Request the project info
            request = QgsBufferServerRequest(f"http://server.qgis.org/map/{project_id}")
            request.setHeader("Accept", "application/json")
            response = QgsBufferServerResponse()
            self.server.handleRequest(request, response)
            jdata = json.loads(bytes(response.body()))
            project_info = jdata["project"]

            # CRS should be the native IAU CRS, not EPSG:4326
            self.assertNotEqual(project_info["crs"], "EPSG:4326")
            self.assertIn("IAU_2015:30190", project_info["crs"])

            # geographic_extent must be absent — it is Earth-specific (EPSG:4326)
            self.assertNotIn("geographic_extent", project_info)

        finally:
            os.environ["QGIS_SERVER_LANDING_PAGE_PROJECTS_DIRECTORIES"] = original_dirs

    def test_landing_page_json_omits_acl_denied_layers(self):
        server = QgsServer()
        request = QgsBufferServerRequest("http://server.qgis.org/index.json")
        request.setHeader("Accept", "application/json")
        response = QgsBufferServerResponse()
        server.handleRequest(request, response)
        initial_index = json.loads(bytes(response.body()))

        projects_with_layers = [
            p for p in initial_index["projects"] if p.get("wms_layers")
        ]
        self.assertTrue(projects_with_layers)
        target_project = projects_with_layers[0]
        denied_layer_id = next(iter(target_project["wms_layers"].keys()))

        iface = server.serverInterface()
        acl_filter = self._RestrictedLayerAccessControl(iface, [denied_layer_id])
        iface.registerAccessControl(acl_filter, 100)

        response = QgsBufferServerResponse()
        server.handleRequest(request, response)
        filtered_index = json.loads(bytes(response.body()))

        filtered_target_project = next(
            (
                p
                for p in filtered_index["projects"]
                if p.get("id") == target_project["id"]
            ),
            None,
        )
        self.assertIsNotNone(filtered_target_project)
        filtered_target_project = filtered_target_project or {}

        self.assertNotIn(
            denied_layer_id, filtered_target_project.get("wms_layers") or {}
        )
        self.assertNotIn(
            denied_layer_id, filtered_target_project.get("wms_layers_queryable") or []
        )
        self.assertNotIn(
            denied_layer_id, filtered_target_project.get("wms_layers_searchable") or []
        )
        self.assertNotIn(
            denied_layer_id,
            (filtered_target_project.get("wms_layers_typename_id_map") or {}).values(),
        )
        self.assertNotIn(
            denied_layer_id,
            (filtered_target_project.get("capabilities") or {}).get("wfsLayerIds", []),
        )
        self.assertNotIn(
            denied_layer_id,
            self._layer_ids_from_toc(filtered_target_project.get("toc") or {}),
        )

    def test_project_json_omits_acl_denied_layers(self):
        server = QgsServer()
        request = QgsBufferServerRequest("http://server.qgis.org/index.json")
        request.setHeader("Accept", "application/json")
        response = QgsBufferServerResponse()
        server.handleRequest(request, response)
        index_data = json.loads(bytes(response.body()))

        projects_with_layers = [
            p for p in index_data["projects"] if p.get("wms_layers")
        ]
        self.assertTrue(projects_with_layers)
        target_project = projects_with_layers[0]
        self.assertTrue(target_project["wms_layers"])
        project_id = target_project["id"]
        denied_layer_id = next(iter(target_project["wms_layers"].keys()))
        denied_layer_identifier = next(
            (
                name
                for name, layer_id in target_project[
                    "wms_layers_typename_id_map"
                ].items()
                if layer_id == denied_layer_id
            ),
            denied_layer_id,
        )

        iface = server.serverInterface()
        acl_filter = self._RestrictedLayerAccessControl(iface, [denied_layer_id])
        iface.registerAccessControl(acl_filter, 100)

        request = QgsBufferServerRequest(f"http://server.qgis.org/map/{project_id}")
        request.setHeader("Accept", "application/json")
        response = QgsBufferServerResponse()
        server.handleRequest(request, response)
        project_data = json.loads(bytes(response.body()))["project"]

        self.assertNotIn(denied_layer_id, project_data.get("wms_layers") or {})
        self.assertNotIn(
            denied_layer_id, project_data.get("wms_layers_queryable") or []
        )
        self.assertNotIn(
            denied_layer_id, project_data.get("wms_layers_searchable") or []
        )
        self.assertNotIn(
            denied_layer_id,
            (project_data.get("wms_layers_typename_id_map") or {}).values(),
        )
        self.assertNotIn(
            denied_layer_identifier,
            (project_data.get("wms_layers_map") or {}).values(),
        )
        self.assertNotIn(
            denied_layer_id,
            (project_data.get("capabilities") or {}).get("wfsLayerIds", []),
        )
        self.assertNotIn(
            denied_layer_id,
            self._layer_ids_from_toc(project_data.get("toc") or {}),
        )


if __name__ == "__main__":
    unittest.main()
