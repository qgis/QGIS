"""QGIS Unit tests for QgsServer.

From build dir, run: ctest -R PyQgsServerAccessControlWMSGetMapPG -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Stephane Brunner"
__date__ = "28/08/2015"
__copyright__ = "Copyright 2015, The QGIS Project"

import os
import urllib.error
import urllib.parse
import urllib.request

from qgis.core import QgsProviderRegistry
from qgis.server import QgsAccessControlFilter
from qgis.testing import unittest

from test_qgsserver_accesscontrol import TestQgsServerAccessControl


class RestrictedAccessControlPG(QgsAccessControlFilter):
    """Used to have restriction access"""

    # Be able to deactivate the access control to have a reference point
    _active = False

    def __init__(self, server_iface):
        super(QgsAccessControlFilter, self).__init__(server_iface)

    def layerFilterExpression(self, layer):
        """Return an additional expression filter"""

        if not self._active:
            return super().layerFilterExpression(layer)

        if layer.name() == "someData" or layer.name() == "someDataLong":
            return "pk > 2"
        else:
            return None

    def layerFilterSubsetString(self, layer):
        """Return an additional subset string (typically SQL) filter"""

        if not self._active:
            return super().layerFilterSubsetString(layer)

        if layer.name() == "someData" or layer.name() == "someDataLong":
            return "pk > 2"
        else:
            return None

    def layerPermissions(self, layer):
        """Return the layer rights"""

        return super().layerPermissions(layer)

    def authorizedLayerAttributes(self, layer, attributes):
        """Return the authorised layer attributes"""

        return super().authorizedLayerAttributes(layer, attributes)

    def allowToEdit(self, layer, feature):
        """Are we authorise to modify the following geometry"""

        return super().allowToEdit(layer, feature)

    def cacheKey(self):
        return "r" if self._active else "f"


class TestQgsServerAccessControlWMSGetMapPG(TestQgsServerAccessControl):
    """QGIS Server Access Control WMS Tests"""

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

        cls._accesscontrolpg = RestrictedAccessControlPG(cls._server_iface)
        cls._server_iface.registerAccessControl(cls._accesscontrolpg, 100)

    def setUp(self):
        super().setUp()

        self.projectPath = os.path.join(self.testdata_path, "project_postgres.qgs")
        self.assertTrue(
            os.path.isfile(self.projectPath),
            f'Could not find project file "{self.projectPath}"',
        )

    def _handle_request(self, restricted, query_string, **kwargs):
        self._accesscontrolpg._active = restricted
        return super()._handle_request(restricted, query_string, **kwargs)

    def test_wms_getmap(self):
        query_string = "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetMap",
                        "LAYERS": "Country,Hello,someData",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
                        "HEIGHT": "500",
                        "WIDTH": "500",
                        "SRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_PG_GetMap")

        query_string = "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetMap",
                        "LAYERS": "Hello,someData",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
                        "HEIGHT": "500",
                        "WIDTH": "500",
                        "SRS": "EPSG:3857",
                    }.items()
                )
            ]
        )
        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_PG_GetMap")

        query_string = "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetMap",
                        "LAYERS": "Country,Hello,someData",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
                        "HEIGHT": "500",
                        "WIDTH": "500",
                        "SRS": "EPSG:3857",
                        "SELECTION": "someData: 4",
                    }.items()
                )
            ]
        )

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_PG_GetMap_Selection")

        query_string = "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetMap",
                        "LAYERS": "Hello,someData",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
                        "HEIGHT": "500",
                        "WIDTH": "500",
                        "SRS": "EPSG:3857",
                        "SELECTION": "someData: 4",
                    }.items()
                )
            ]
        )
        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_PG_GetMap_Selection")

    def test_wms_getmap_long(self):
        query_string = "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetMap",
                        "LAYERS": "Country,Hello,someDataLong",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
                        "HEIGHT": "500",
                        "WIDTH": "500",
                        "SRS": "EPSG:3857",
                    }.items()
                )
            ]
        )

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_PG_GetMap")

        query_string = "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetMap",
                        "LAYERS": "Hello,someDataLong",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
                        "HEIGHT": "500",
                        "WIDTH": "500",
                        "SRS": "EPSG:3857",
                    }.items()
                )
            ]
        )
        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_PG_GetMap")

        query_string = "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetMap",
                        "LAYERS": "Country,Hello,someDataLong",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
                        "HEIGHT": "500",
                        "WIDTH": "500",
                        "SRS": "EPSG:3857",
                        "SELECTION": "someDataLong: 4",
                    }.items()
                )
            ]
        )

        response, headers = self._get_fullaccess(query_string)
        self._img_diff_error(response, headers, "WMS_PG_GetMap_Selection")

        query_string = "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(self.projectPath),
                        "SERVICE": "WMS",
                        "VERSION": "1.1.1",
                        "REQUEST": "GetMap",
                        "LAYERS": "Hello,someDataLong",
                        "STYLES": "",
                        "FORMAT": "image/png",
                        "BBOX": "-16817707,-6318936.5,5696513,16195283.5",
                        "HEIGHT": "500",
                        "WIDTH": "500",
                        "SRS": "EPSG:3857",
                        "SELECTION": "someDataLong: 4",
                    }.items()
                )
            ]
        )
        response, headers = self._get_restricted(query_string)
        self._img_diff_error(response, headers, "Restricted_WMS_PG_GetMap_Selection")


if __name__ == "__main__":
    unittest.main()
