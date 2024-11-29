"""QGIS Unit tests for QgsServer WMS GetPrint with postgres access control filters.

From build dir, run: ctest -R PyQgsServerAccessControlWMSGetPrintPG -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "25/02/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

import os
import re

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ["QT_HASH_SEED"] = "1"

from qgis.core import QgsProject, QgsProviderRegistry, QgsVectorLayer
from qgis.PyQt.QtCore import QTemporaryDir
from qgis.PyQt.QtGui import QImage
from qgis.server import (
    QgsAccessControlFilter,
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
)
from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase
from utilities import unitTestDataPath


class RestrictedAccessControl(QgsAccessControlFilter):
    """Restricts access to pk1 = 1 AND pk2 = 1"""

    # Be able to deactivate the access control to have a reference point
    active = {
        "layerFilterExpression": False,
        "layerFilterSubsetString": False,
        "authorizedLayerAttributes": False,
        "layerPermissions": False,
    }

    def layerFilterExpression(self, layer):
        """Return an additional expression filter"""

        if not self.active["layerFilterExpression"]:
            return super().layerFilterExpression(layer)

        if layer.name() == "multiple_pks":
            return "pk1 = 1 AND pk2 = 1"
        else:
            return None

    def layerFilterSubsetString(self, layer):
        """Return an additional subset string (typically SQL) filter"""

        if not self.active["layerFilterSubsetString"]:
            return super().layerFilterSubsetString(layer)

        if layer.name() == "multiple_pks":
            return "pk1 = 1 AND pk2 = 1"
        else:
            return None

    def authorizedLayerAttributes(self, layer, attributes):
        """Return the authorised layer attributes"""

        if not self.active["authorizedLayerAttributes"]:
            return super().authorizedLayerAttributes(layer, attributes)

        allowed = []

        for attr in attributes:
            if "name" != attr and "virtual" != attr:  # spellok
                allowed.append(attr)  # spellok

        return allowed

    def layerPermissions(self, layer):
        """Return the layer rights"""

        rights = QgsAccessControlFilter.LayerPermissions()
        rights.canRead = not self.active["layerPermissions"]

        return rights


class TestQgsServerAccessControlWMSGetPrintPG(QgsServerTestBase):
    """QGIS Server WMS Tests for GetPrint request"""

    # Set to True in child classes to re-generate reference files for this class
    # regenerate_reference = True

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

        project_path = os.path.join(
            unitTestDataPath("qgis_server_accesscontrol"), "pg_multiple_pks.qgs"
        )
        cls.temp_dir = QTemporaryDir()
        cls.temp_project_path = os.path.join(cls.temp_dir.path(), "pg_multiple_pks.qgs")

        # Create test layer

        conn.executeSql("DROP TABLE IF EXISTS qgis_test.multiple_pks")
        conn.executeSql(
            "CREATE TABLE qgis_test.multiple_pks ( pk1 bigint not null, pk2 bigint not null, name text not null, geom geometry(POINT,4326), PRIMARY KEY ( pk1, pk2 ) )"
        )
        conn.executeSql(
            "INSERT INTO qgis_test.multiple_pks VALUES ( 1, 1, '1-1', ST_GeomFromText('point(7 45)', 4326))"
        )
        conn.executeSql(
            "INSERT INTO qgis_test.multiple_pks VALUES ( 1, 2, '1-2', ST_GeomFromText('point(8 46)', 4326))"
        )

        cls.layer_uri = (
            uri
            + " sslmode=disable key='pk1,pk2' estimatedmetadata=true srid=4326 type=Point checkPrimaryKeyUnicity='0' table=\"qgis_test\".\"multiple_pks\" (geom)"
        )
        layer = QgsVectorLayer(cls.layer_uri, "multiple_pks", "postgres")

        assert layer.isValid()

        project = open(project_path).read()
        with open(cls.temp_project_path, "w+") as f:
            f.write(
                re.sub(
                    r"<datasource>.*</datasource>",
                    f"<datasource>{cls.layer_uri}</datasource>",
                    project,
                )
            )

        cls.test_project = QgsProject()
        cls.test_project.read(cls.temp_project_path)

        # Setup access control
        cls._server = QgsServer()
        cls._server_iface = cls._server.serverInterface()
        cls._accesscontrol = RestrictedAccessControl(cls._server_iface)
        cls._server_iface.registerAccessControl(cls._accesscontrol, 100)

    def _clear_constraints(self):
        self._accesscontrol.active["authorizedLayerAttributes"] = False
        self._accesscontrol.active["layerFilterExpression"] = False
        self._accesscontrol.active["layerFilterSubsetString"] = False
        self._accesscontrol.active["layerPermissions"] = False

    def setUp(self):
        super().setUp()
        self._clear_constraints()

    def _check_exception(self, qs, exception_text):
        """Check that server throws"""

        req = QgsBufferServerRequest("http://my_server/" + qs)
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 400)
        self.assertIn(exception_text, bytes(res.body()).decode("utf8"))

    def _check_white(self, qs):
        """Check that output is a white image"""

        req = QgsBufferServerRequest("http://my_server/" + qs)
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 200)

        result_path = os.path.join(self.temp_dir.path(), "white.png")
        with open(result_path, "wb+") as f:
            f.write(res.body())

        # A full white image is expected
        image = QImage(result_path)
        self.assertTrue(image.isGrayscale())
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 255)
        self.assertEqual(color.green(), 255)
        self.assertEqual(color.blue(), 255)

    def test_wms_getprint_postgres(self):
        """Test issue GH #41800"""

        # Extent for feature where pk1 = 1, pk2 = 2
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "CRS": "EPSG:4326",
                        "FORMAT": "png",
                        "LAYERS": "multiple_pks",
                        "DPI": 72,
                        "TEMPLATE": "print1",
                        "map0:EXTENT": "45.70487804878048621,7.67926829268292099,46.22987804878049189,8.42479674796748235",
                    }.items()
                )
            ]
        )

        def _check_red():

            req = QgsBufferServerRequest("http://my_server/" + qs)
            res = QgsBufferServerResponse()
            self._server.handleRequest(req, res, self.test_project)
            self.assertEqual(res.statusCode(), 200)

            result_path = os.path.join(self.temp_dir.path(), "red.png")
            with open(result_path, "wb+") as f:
                f.write(res.body())

            # A full red image is expected
            image = QImage(result_path)
            self.assertFalse(image.isGrayscale())
            color = image.pixelColor(100, 100)
            self.assertEqual(color.red(), 255)
            self.assertEqual(color.green(), 0)
            self.assertEqual(color.blue(), 0)

        _check_red()

        # Now activate the rule to exclude the feature where pk1 = 1, pk2 = 2
        # A white image is expected

        self._accesscontrol.active["layerFilterExpression"] = True
        self._check_white(qs)

        # Activate the other rule for subset string

        self._accesscontrol.active["layerFilterExpression"] = False
        self._accesscontrol.active["layerFilterSubsetString"] = True
        self._check_white(qs)

        # Activate the other rule for layer permission

        self._accesscontrol.active["layerFilterSubsetString"] = False
        self._accesscontrol.active["layerPermissions"] = True

        req = QgsBufferServerRequest("http://my_server/" + qs)
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 403)

        # Test attribute table (template print2) with no rule
        self._accesscontrol.active["layerPermissions"] = False

        req = QgsBufferServerRequest(
            "http://my_server/" + qs.replace("print1", "print2")
        )
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 200)

        self._img_diff_error(res.body(), res.headers(), "WMS_GetPrint_postgres_print2")

        # Test attribute table with rule
        self._accesscontrol.active["authorizedLayerAttributes"] = True

        req = QgsBufferServerRequest(
            "http://my_server/" + qs.replace("print1", "print2")
        )
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 200)

        self._img_diff_error(
            res.body(), res.headers(), "WMS_GetPrint_postgres_print2_filtered"
        )

        # Re-Test attribute table (template print2) with no rule
        self._accesscontrol.active["authorizedLayerAttributes"] = False

        req = QgsBufferServerRequest(
            "http://my_server/" + qs.replace("print1", "print2")
        )
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 200)

        self._img_diff_error(res.body(), res.headers(), "WMS_GetPrint_postgres_print2")

        # Test with layer permissions
        self._accesscontrol.active["layerPermissions"] = True
        req = QgsBufferServerRequest(
            "http://my_server/" + qs.replace("print1", "print2")
        )
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 403)

        # Test with subset string
        self._accesscontrol.active["layerPermissions"] = False
        self._accesscontrol.active["layerFilterSubsetString"] = True
        req = QgsBufferServerRequest(
            "http://my_server/" + qs.replace("print1", "print2")
        )
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 200)

        self._img_diff_error(
            res.body(), res.headers(), "WMS_GetPrint_postgres_print2_subset"
        )

        # Test with filter expression
        self._accesscontrol.active["layerFilterExpression"] = True
        self._accesscontrol.active["layerFilterSubsetString"] = False
        req = QgsBufferServerRequest(
            "http://my_server/" + qs.replace("print1", "print2")
        )
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 200)

        self._img_diff_error(
            res.body(), res.headers(), "WMS_GetPrint_postgres_print2_subset"
        )

        # Test attribute table with attribute filter
        self._accesscontrol.active["layerFilterExpression"] = False
        self._accesscontrol.active["authorizedLayerAttributes"] = True

        req = QgsBufferServerRequest(
            "http://my_server/" + qs.replace("print1", "print2")
        )
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 200)

        self._img_diff_error(
            res.body(), res.headers(), "WMS_GetPrint_postgres_print2_filtered"
        )

        # Clear constraints
        self._clear_constraints()
        _check_red()

        req = QgsBufferServerRequest(
            "http://my_server/" + qs.replace("print1", "print2")
        )
        res = QgsBufferServerResponse()
        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 200)

        self._img_diff_error(res.body(), res.headers(), "WMS_GetPrint_postgres_print2")

    def test_atlas(self):
        """Test atlas"""

        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetPrint",
                        "CRS": "EPSG:4326",
                        "FORMAT": "png",
                        "LAYERS": "multiple_pks",
                        "DPI": 72,
                        "TEMPLATE": "print1",
                    }.items()
                )
            ]
        )

        req = QgsBufferServerRequest("http://my_server/" + qs + "&ATLAS_PK=1,2")
        res = QgsBufferServerResponse()

        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 200)

        result_path = os.path.join(self.temp_dir.path(), "atlas_1_2.png")
        with open(result_path, "wb+") as f:
            f.write(res.body())

        # A full red image is expected
        image = QImage(result_path)
        self.assertFalse(image.isGrayscale())
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 255)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 0)

        # Forbid 1-1
        self._accesscontrol.active["layerFilterSubsetString"] = True
        self._check_exception(qs + "&ATLAS_PK=1,2", "Atlas error: empty atlas.")

        self._accesscontrol.active["layerFilterSubsetString"] = False
        self._accesscontrol.active["layerFilterExpression"] = True
        self._check_exception(qs + "&ATLAS_PK=1,2", "Atlas error: empty atlas.")

        # Remove all constraints
        self._clear_constraints()
        req = QgsBufferServerRequest("http://my_server/" + qs + "&ATLAS_PK=1,2")
        res = QgsBufferServerResponse()

        self._server.handleRequest(req, res, self.test_project)
        self.assertEqual(res.statusCode(), 200)

        result_path = os.path.join(self.temp_dir.path(), "atlas_1_2.png")
        with open(result_path, "wb+") as f:
            f.write(res.body())

        # A full red image is expected
        image = QImage(result_path)
        self.assertFalse(image.isGrayscale())
        color = image.pixelColor(100, 100)
        self.assertEqual(color.red(), 255)
        self.assertEqual(color.green(), 0)
        self.assertEqual(color.blue(), 0)


if __name__ == "__main__":
    unittest.main()
