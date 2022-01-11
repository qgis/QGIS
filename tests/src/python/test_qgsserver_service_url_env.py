# -*- coding: utf-8 -*-
"""
QGIS Unit tests for QgsServer with service URL defined in the environment variables
"""
__author__ = 'Stéphane Brunner'
__date__ = '12/01/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import os

# Deterministic XML
os.environ['QT_HASH_SEED'] = '1'

import urllib.request
import urllib.parse
import urllib.error

from qgis.server import QgsServer
from qgis.core import QgsFontUtils
from qgis.testing import unittest, start_app
from utilities import unitTestDataPath


start_app()


class TestQgsServerServiceUrlEnv(unittest.TestCase):

    def setUp(self):
        """Create the server instance"""
        self.fontFamily = QgsFontUtils.standardTestFontFamily()
        QgsFontUtils.loadStandardTestFonts(['All'])

        self.testdata_path = unitTestDataPath('qgis_server') + '/'
        project = os.path.join(self.testdata_path, "test_project_without_urls.qgs")

        del os.environ['QUERY_STRING']
        os.environ['QGIS_PROJECT_FILE'] = project
        # Disable landing page API to test standard legacy XML responses in case of errors
        os.environ["QGIS_SERVER_DISABLED_APIS"] = "Landing Page"

        self.server = QgsServer()

    def tearDown(self):
        """Cleanup env"""

        super().tearDown()
        for env in ["QGIS_SERVER_DISABLED_APIS", "QGIS_PROJECT_FILE"]:
            if env in os.environ:
                del os.environ[env]

    """Tests container"""

    def test_getcapabilities_url(self):

        # Service URL in environment
        for env_name, env_value, service in (
            ("QGIS_SERVER_SERVICE_URL", "http://test1", "WMS"),
            ("QGIS_SERVER_WMS_SERVICE_URL", "http://test2", "WMS")
            ("QGIS_SERVER_SERVICE_URL", "http://test3", "WFS", "href="),
            ("QGIS_SERVER_WFS_SERVICE_URL", "http://test4", "WFS", "href=")
            ("QGIS_SERVER_SERVICE_URL", "http://test5", "WCS"),
            ("QGIS_SERVER_WCS_SERVICE_URL", "http://test6", "WCS")
            ("QGIS_SERVER_SERVICE_URL", "http://test7", "WMTS"),
            ("QGIS_SERVER_WMTS_SERVICE_URL", "http://test8", "WMTS")
        ):
            try:
                os.environ[env_name] = env_value
                qs = "?" + "&".join(["%s=%s" % i for i in list({
                    "SERVICE": service,
                    "REQUEST": "GetCapabilities",
                }.items())])

                r, _ = self._result(self._execute_request(qs))

                item_found = False
                for item in str(r).split("\\n"):
                    if "href=" in item:
                        self.assertEqual(env_value in item, True)
                        item_found = True
                self.assertTrue(item_found)
            finally:
                del os.environ[env_name]


if __name__ == '__main__':
    unittest.main()
