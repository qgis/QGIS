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

from qgis.PyQt import QtCore
from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
)
from qgis.testing import unittest
from test_qgsserver_api import QgsServerAPITestBase
from utilities import unitTestDataPath


class QgsServerLandingPageTest(QgsServerAPITestBase):
    """QGIS Server Landing Page tests"""

    # Set to True in child classes to re-generate reference files for this class
    # regeregenerate_api_reference = True

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

        if self.regeregenerate_api_reference:
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
                bytes(response.body()),
                b'<?xml version="1.0" encoding="UTF-8"?>\n<ServerException>Project file error. For OWS services: please provide a SERVICE and a MAP parameter pointing to a valid QGIS project file</ServerException>\n',
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


if __name__ == "__main__":
    unittest.main()
