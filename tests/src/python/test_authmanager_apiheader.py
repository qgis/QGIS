"""QGIS Unit tests for the API header authentication method.

From build dir, run: ctest -R PyQgsAuthManagerApiHeader -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Stefan Crooijmans"
__date__ = "22/10/2026"
__copyright__ = "Copyright 2026, The QGIS Project"

import os
import tempfile
import unittest
from shutil import rmtree

from qgis.PyQt.QtCore import QUrl
from qgis.PyQt.QtNetwork import QNetworkRequest
from qgis.core import QgsApplication, QgsAuthMethodConfig
from qgis.testing import QgisTestCase, start_app

QGIS_AUTH_DB_DIR_PATH = tempfile.mkdtemp()

os.environ["QGIS_AUTH_DB_DIR_PATH"] = QGIS_AUTH_DB_DIR_PATH

qgis_app = start_app()


class TestAuthManagerApiHeader(QgisTestCase):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.authm = QgsApplication.authManager()
        assert cls.authm.setMasterPassword("masterpassword", True)

    @classmethod
    def tearDownClass(cls):
        rmtree(QGIS_AUTH_DB_DIR_PATH)
        super().tearDownClass()

    def _store(self, headers):
        config = QgsAuthMethodConfig("APIHeader")
        config.setName("api header test")
        config.setConfigMap(headers)
        ok, config = self.authm.storeAuthenticationConfig(config)
        self.assertTrue(ok)
        return config.id()

    def _update(self, authcfg):
        request = QNetworkRequest(QUrl("http://localhost/"))
        self.assertTrue(self.authm.updateNetworkRequest(request, authcfg))
        return request

    def test_headers_are_sent(self):
        request = self._update(
            self._store({"Authorization": "Bearer secret", "X-Custom": "value"})
        )
        self.assertEqual(request.rawHeader(b"Authorization"), b"Bearer secret")
        self.assertEqual(request.rawHeader(b"X-Custom"), b"value")

    def test_surrounding_whitespace_is_trimmed(self):
        """A key pasted with a trailing line break must still reach the server"""
        request = self._update(
            self._store(
                {
                    "Authorization": "Bearer secret\n",
                    "X-Crlf": "value\r\n",
                    " X-Spaced ": " value ",
                }
            )
        )
        self.assertEqual(request.rawHeader(b"Authorization"), b"Bearer secret")
        self.assertEqual(request.rawHeader(b"X-Crlf"), b"value")
        self.assertEqual(request.rawHeader(b"X-Spaced"), b"value")

    def test_line_break_inside_value_fails(self):
        """Qt drops a header whose value contains a line break; the request must not go out silently without it"""
        authcfg = self._store({"Authorization": "Bearer sec\nret"})
        request = QNetworkRequest(QUrl("http://localhost/"))
        self.assertFalse(self.authm.updateNetworkRequest(request, authcfg))
        self.assertFalse(request.hasRawHeader(b"Authorization"))


if __name__ == "__main__":
    unittest.main()
