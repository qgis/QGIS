"""
Tests for auth manager WMS/WFS using QGIS Server through HTTP Basic
enabled qgis_wrapped_server.py.

This is an integration test for QGIS Desktop Auth Manager WFS and WMS provider
and QGIS Server WFS/WMS that check if QGIS can use a stored auth manager auth
configuration to access an HTTP Basic protected endpoint.


From build dir, run from test directory:
LC_ALL=en_US.UTF-8 ctest -R PyQgsAuthManagerPasswordOWSTest -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import random
import re
import string
import subprocess
import sys
import tempfile
import urllib

from functools import partial

__author__ = "Alessandro Pasotti"
__date__ = "18/09/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

from shutil import rmtree

from qgis.core import (
    QgsApplication,
    QgsAuthMethodConfig,
    QgsFileDownloader,
    QgsRasterLayer,
    QgsVectorLayer,
)
from qgis.PyQt.QtCore import QEventLoop, QUrl
import unittest
from qgis.testing import start_app, QgisTestCase
from utilities import unitTestDataPath, waitServer

try:
    QGIS_SERVER_ENDPOINT_PORT = os.environ["QGIS_SERVER_ENDPOINT_PORT"]
except:
    QGIS_SERVER_ENDPOINT_PORT = "0"  # Auto

QGIS_AUTH_DB_DIR_PATH = tempfile.mkdtemp()

os.environ["QGIS_AUTH_DB_DIR_PATH"] = QGIS_AUTH_DB_DIR_PATH

qgis_app = start_app()


class TestAuthManager(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests:
        Creates an auth configuration"""
        super().setUpClass()
        cls.port = QGIS_SERVER_ENDPOINT_PORT
        # Clean env just to be sure
        env_vars = ["QUERY_STRING", "QGIS_PROJECT_FILE"]
        for ev in env_vars:
            try:
                del os.environ[ev]
            except KeyError:
                pass
        cls.testdata_path = unitTestDataPath("qgis_server") + "/"
        cls.project_path = cls.testdata_path + "test_project.qgs"
        # Enable auth
        # os.environ['QGIS_AUTH_PASSWORD_FILE'] = QGIS_AUTH_PASSWORD_FILE
        authm = QgsApplication.authManager()
        assert authm.setMasterPassword("masterpassword", True)
        cls.auth_config = QgsAuthMethodConfig("Basic")
        cls.auth_config.setName("test_auth_config")
        cls.username = "".join(
            random.choice(string.ascii_uppercase + string.digits) for _ in range(6)
        )
        cls.password = cls.username[::-1]  # reversed
        cls.auth_config.setConfig("username", cls.username)
        cls.auth_config.setConfig("password", cls.password)
        assert authm.storeAuthenticationConfig(cls.auth_config)[0]
        cls.hostname = "127.0.0.1"
        cls.protocol = "http"

        os.environ["QGIS_SERVER_HTTP_BASIC_AUTH"] = "1"
        os.environ["QGIS_SERVER_USERNAME"] = cls.username
        os.environ["QGIS_SERVER_PASSWORD"] = cls.password
        os.environ["QGIS_SERVER_PORT"] = str(cls.port)
        os.environ["QGIS_SERVER_HOST"] = cls.hostname

        server_path = (
            os.path.dirname(os.path.realpath(__file__)) + "/qgis_wrapped_server.py"
        )
        cls.server = subprocess.Popen(
            [sys.executable, server_path],
            env=os.environ,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )

        line = cls.server.stdout.readline()
        cls.port = int(re.findall(rb":(\d+)", line)[0])
        assert cls.port != 0
        # Wait for the server process to start
        assert waitServer(
            f"{cls.protocol}://{cls.hostname}:{cls.port}"
        ), f"Server is not responding! {cls.protocol}://{cls.hostname}:{cls.port}"

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.server.terminate()
        rmtree(QGIS_AUTH_DB_DIR_PATH)
        del cls.server
        super().tearDownClass()

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    @classmethod
    def _getWFSLayer(cls, type_name, layer_name=None, authcfg=None):
        """
        WFS layer factory
        """
        if layer_name is None:
            layer_name = "wfs_" + type_name
        parms = {
            "srsname": "EPSG:4326",
            "typename": type_name,
            "url": f"{cls.protocol}://{cls.hostname}:{cls.port}/?map={cls.project_path}",
            "version": "auto",
            "table": "",
        }
        if authcfg is not None:
            parms.update({"authcfg": authcfg})
        uri = " ".join([(f"{k}='{v}'") for k, v in list(parms.items())])
        wfs_layer = QgsVectorLayer(uri, layer_name, "WFS")
        return wfs_layer

    @classmethod
    def _getWMSLayer(cls, layers, layer_name=None, authcfg=None):
        """
        WMS layer factory
        """
        if layer_name is None:
            layer_name = "wms_" + layers.replace(",", "")
        parms = {
            "crs": "EPSG:4326",
            "url": f"{cls.protocol}://{cls.hostname}:{cls.port}/?map={cls.project_path}",
            # This is needed because of a really weird implementation in QGIS Server, that
            # replaces _ in the the real layer name with spaces
            "layers": urllib.parse.quote(layers.replace("_", " ")),
            "styles": "",
            "version": "auto",
            # 'sql': '',
        }
        if authcfg is not None:
            parms.update({"authcfg": authcfg})
        uri = "&".join([f"{k}={v.replace('=', '%3D')}" for k, v in list(parms.items())])
        wms_layer = QgsRasterLayer(uri, layer_name, "wms")
        return wms_layer

    @classmethod
    def _getGeoJsonLayer(cls, type_name, layer_name=None, authcfg=None):
        """
        OGR layer factory
        """
        if layer_name is None:
            layer_name = "geojson_" + type_name
        uri = f"{cls.protocol}://{cls.hostname}:{cls.port}/?MAP={cls.project_path}&SERVICE=WFS&REQUEST=GetFeature&TYPENAME={urllib.parse.quote(type_name)}&VERSION=2.0.0&OUTPUTFORMAT=geojson"
        if authcfg is not None:
            uri += f" authcfg='{authcfg}'"
        geojson_layer = QgsVectorLayer(uri, layer_name, "ogr")
        return geojson_layer

    def testValidAuthAccess(self):
        """
        Access the HTTP Basic protected layer with valid credentials
        """
        wfs_layer = self._getWFSLayer("testlayer_èé", authcfg=self.auth_config.id())
        self.assertTrue(wfs_layer.isValid())
        wms_layer = self._getWMSLayer("testlayer_èé", authcfg=self.auth_config.id())
        self.assertTrue(wms_layer.isValid())
        geojson_layer = self._getGeoJsonLayer(
            "testlayer_èé", authcfg=self.auth_config.id()
        )
        self.assertTrue(geojson_layer.isValid())

    def testInvalidAuthAccess(self):
        """
        Access the HTTP Basic protected layer with no credentials
        """
        wfs_layer = self._getWFSLayer("testlayer èé")
        self.assertFalse(wfs_layer.isValid())
        wms_layer = self._getWMSLayer("testlayer_èé")
        self.assertFalse(wms_layer.isValid())
        geojson_layer = self._getGeoJsonLayer("testlayer_èé")
        self.assertFalse(geojson_layer.isValid())

    def testInvalidAuthFileDownload(self):
        """
        Download a protected map tile without authcfg
        """
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project_path),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetMap",
                        "LAYERS": "testlayer_èé".replace("_", "%20"),
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "-16817707,-4710778,5696513,14587125",
                        "HEIGHT": "500",
                        "WIDTH": "500",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )
        url = f"{self.protocol}://{self.hostname}:{self.port}/{qs}"

        destination = tempfile.mktemp()
        loop = QEventLoop()

        downloader = QgsFileDownloader(QUrl(url), destination, None, False)
        downloader.downloadCompleted.connect(partial(self._set_slot, "completed"))
        downloader.downloadExited.connect(partial(self._set_slot, "exited"))
        downloader.downloadCanceled.connect(partial(self._set_slot, "canceled"))
        downloader.downloadError.connect(partial(self._set_slot, "error"))
        downloader.downloadProgress.connect(partial(self._set_slot, "progress"))

        downloader.downloadExited.connect(loop.quit)

        loop.exec()

        self.assertTrue(self.error_was_called)
        self.assertIn(
            "Download failed: Host requires authentication", str(self.error_args)
        )

    def testValidAuthFileDownload(self):
        """
        Download a map tile with valid authcfg
        """
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.project_path),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetMap",
                        "LAYERS": "testlayer_èé".replace("_", "%20"),
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "-16817707,-4710778,5696513,14587125",
                        "HEIGHT": "500",
                        "WIDTH": "500",
                        "CRS": "EPSG:3857",
                    }.items()
                )
            ]
        )
        url = f"{self.protocol}://{self.hostname}:{self.port}/{qs}"

        destination = tempfile.mktemp()
        loop = QEventLoop()

        downloader = QgsFileDownloader(
            QUrl(url), destination, self.auth_config.id(), False
        )
        downloader.downloadCompleted.connect(partial(self._set_slot, "completed"))
        downloader.downloadExited.connect(partial(self._set_slot, "exited"))
        downloader.downloadCanceled.connect(partial(self._set_slot, "canceled"))
        downloader.downloadError.connect(partial(self._set_slot, "error"))
        downloader.downloadProgress.connect(partial(self._set_slot, "progress"))

        downloader.downloadExited.connect(loop.quit)

        loop.exec()

        # Check the we've got a likely PNG image
        self.assertTrue(self.completed_was_called)
        self.assertGreater(os.path.getsize(destination), 2000)  # > 1MB
        with open(destination, "rb") as f:
            self.assertTrue(b"PNG" in f.read())  # is a PNG

    def _set_slot(self, *args, **kwargs):
        # print('_set_slot(%s) called' % args[0])
        setattr(self, args[0] + "_was_called", True)
        setattr(self, args[0] + "_args", args)


if __name__ == "__main__":
    unittest.main()
