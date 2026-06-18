"""QGIS Unit tests for QgsServer OGC API Features Handler.

From build dir, run: ctest -R PyQgsServerApiFeatures -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "2026-06-3"
__copyright__ = "Copyright 2026, The QGIS Project"

import json
import os
import re
import shutil

from osgeo import gdal, ogr
from provider_python import PyProvider

# Deterministic XML
os.environ["QT_HASH_SEED"] = "1"

from contextlib import contextmanager
from urllib import parse

from qgis.core import (
    Qgis,
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsDataProvider,
    QgsEditorWidgetSetup,
    QgsFeature,
    QgsFeatureRequest,
    QgsField,
    QgsFieldConstraints,
    QgsFields,
    QgsGeometry,
    QgsMemoryProviderUtils,
    QgsProject,
    QgsProviderMetadata,
    QgsProviderRegistry,
    QgsRelation,
    QgsRelationContext,
    QgsVectorLayer,
    QgsVectorLayerServerProperties,
    QgsWkbTypes,
)
from qgis.PyQt import QtCore
from qgis.server import (
    QgsAccessControlFilter,
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
    QgsServerApi,
    QgsServerApiBadRequestException,
    QgsServerApiContext,
    QgsServerApiUtils,
    QgsServerOgcApi,
    QgsServerOgcApiHandler,
    QgsServerQueryStringParameter,
    QgsServiceRegistry,
)
from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase
from test_qgsserver_api import QgsServerAPITestBase
from utilities import unitTestDataPath


class QgsServerOgcApiFeaturesTest(QgsServerAPITestBase):
    """QGIS API server tests"""

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        # Default url has changed in QGIS 4 stick to /wfs3 for the tests
        os.environ.update({"QGIS_SERVER_API_WFS3_ROOT_PATH": "/wfs3"})
        iface = self.server.serverInterface()
        iface.reloadSettings()
        iface.serviceRegistry().cleanUp()
        iface.serviceRegistry().init(QgsApplication.libexecPath() + "server", iface)

    # Set to True in child classes to re-generate reference files for this class
    regenerate_api_reference = False

    def _getJsonResponse(self, url, project):
        request = QgsBufferServerRequest(url)
        response = QgsBufferServerResponse()
        server = QgsServer()
        server.handleRequest(request, response, project)
        self.assertEqual(
            response.statusCode(),
            200,
            f"Request failed with status {response.statusCode()} and message: {bytes(response.body()).decode('utf8')} for URL: {url}",
        )
        response_str = bytes(response.body()).decode("utf8")
        j = json.loads(response_str)
        return j

    def _check_profile(self, profile, project):

        j = self._getJsonResponse(
            f"http://server.qgis.org/wfs3/collections/referencing/items.json?limit=1&profile={profile}",
            project,
        )

        links = [l["href"] for l in j["links"]]

        # No default
        if not profile:
            self.assertNotIn(
                "http://www.opengis.net/def/profile/ogc/0/rel-as-key",
                links,
            )
        else:
            self.assertIn(
                f"http://www.opengis.net/def/profile/ogc/0/{profile}",
                links,
            )
        return j["features"][0]

    def testRelationProfileWithoutPkFk(self):

        # Create two memory layers with a fk:some_value relation
        referenced_layer = QgsVectorLayer(
            "Point?field=some_value:integer", "referenced", "memory"
        )
        feature = QgsFeature(referenced_layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(0 0)"))
        feature.setAttribute("some_value", 123)
        self.assertTrue(referenced_layer.dataProvider().addFeature(feature))
        feature = QgsFeature(referenced_layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(1 1)"))
        feature.setAttribute("some_value", 456)
        self.assertTrue(referenced_layer.dataProvider().addFeature(feature))

        referencing_layer = QgsVectorLayer(
            "Point?field=some_other_value:integer&field=fk:integer",
            "referencing",
            "memory",
        )
        feature = QgsFeature(referencing_layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(2 2)"))
        feature.setAttribute("some_other_value", 1)
        feature.setAttribute("fk", 123)
        self.assertTrue(referencing_layer.dataProvider().addFeature(feature))
        feature = QgsFeature(referencing_layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(3 3)"))
        feature.setAttribute("some_other_value", 2)
        feature.setAttribute("fk", 456)
        self.assertTrue(referencing_layer.dataProvider().addFeature(feature))

        project = QgsProject()
        project.addMapLayer(referenced_layer)
        project.addMapLayer(referencing_layer)

        relation = QgsRelation(QgsRelationContext(project))
        relation.setName("relation1")
        relation.setId("relation1")
        relation.addFieldPair("fk", "some_value")
        relation.setReferencingLayer(referencing_layer.id())
        relation.setReferencedLayer(referenced_layer.id())

        r_manager = project.relationManager()
        r_manager.addRelation(relation)
        self.assertIn("relation1", r_manager.relations())

        referencing_layer.setEditorWidgetSetup(
            1, QgsEditorWidgetSetup("RelationReference", {"Relation": "relation1"})
        )

        # Expose to WFS
        project.writeEntry(
            "WFSLayers", "/", [referencing_layer.id(), referenced_layer.id()]
        )

        # Check rel-as-key profile (default)
        feature = self._check_profile("rel-as-key", project)
        self.assertEqual(feature["properties"]["fk"], 123)

        # Empty profile > empty link
        feature = self._check_profile("", project)
        self.assertEqual(feature["properties"]["fk"], 123)

        # rel-as-uri
        feature = self._check_profile("rel-as-uri", project)
        self.assertEqual(
            feature["properties"]["fk"],
            "http://server.qgis.org/wfs3/collections/referenced/items.json?profile=rel-as-uri&some_value=123",
        )

        # Check that URI responds with the correct feature
        j = self._getJsonResponse(
            feature["properties"]["fk"],
            project,
        )
        self.assertEqual(j["features"][0]["properties"]["some_value"], 123)

        # rel-as-link
        feature = self._check_profile("rel-as-link", project)
        self.assertEqual(
            feature["properties"]["fk"],
            {
                "href": "http://server.qgis.org/wfs3/collections/referenced/items.json?profile=rel-as-link&some_value=123",
                "title": "Related feature from layer 'referenced'",
            },
        )

    def testProfileWithRegularPkFkRelation(self):
        """Nominal case with true PK-FK relation in a GPKG DB"""

        # Create a GPKG with two layers in relation
        temp_dir = QtCore.QTemporaryDir()
        temp_path = temp_dir.path()
        gpkg_path = os.path.join(temp_path, "test.gpkg")
        metadata = QgsProviderRegistry.instance().providerMetadata("ogr")
        ok, err = metadata.createDatabase(gpkg_path)
        self.assertTrue(ok)
        self.assertFalse(err)

        conn = metadata.createConnection(gpkg_path, {})
        fields = QgsFields()
        fields.append(QgsField("name", QtCore.QMetaType.Type.QString))
        fields.append(QgsField("other_id", QtCore.QMetaType.Type.Int))
        options = {}
        crs = QgsCoordinateReferenceSystem.fromEpsgId(4326)
        typ = QgsWkbTypes.Type.LineString
        conn.createVectorTable("", "referenced", fields, typ, crs, True, options)
        fields.append(QgsField("fk", QtCore.QMetaType.Type.Int))
        conn.createVectorTable("", "referencing", fields, typ, crs, True, options)

        referenced_layer = QgsVectorLayer(
            f"{gpkg_path}|layername=referenced", "referenced", "ogr"
        )
        feature = QgsFeature(referenced_layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("LINESTRING(0 0, 1 1)"))
        feature.setAttribute("name", "test")
        feature.setAttribute(
            "other_id", 1
        )  # Same as PK to test value relation not on PK
        self.assertTrue(referenced_layer.dataProvider().addFeature(feature))

        referencing_layer = QgsVectorLayer(
            f"{gpkg_path}|layername=referencing", "referencing", "ogr"
        )
        feature = QgsFeature(referencing_layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("LINESTRING(2 2, 3 3)"))
        feature.setAttribute("name", "points to test 1")
        feature.setAttribute("fk", 1)
        self.assertTrue(referencing_layer.dataProvider().addFeature(feature))

        # Create PK->FK relation
        project = QgsProject()
        project.addMapLayer(referenced_layer)
        project.addMapLayer(referencing_layer)
        relation = QgsRelation(QgsRelationContext(project))
        relation.setName("relation1")
        relation.setId("relation1")
        relation.addFieldPair("fk", "fid")
        relation.setReferencingLayer(referencing_layer.id())
        relation.setReferencedLayer(referenced_layer.id())
        r_manager = project.relationManager()
        r_manager.addRelation(relation)
        self.assertIn("relation1", r_manager.relations())

        # Expose to WFS
        project.writeEntry(
            "WFSLayers", "/", [referencing_layer.id(), referenced_layer.id()]
        )

        # At this point there is no widget set up, so the default profile should use the raw FK value
        # Check rel-as-key profile (default)
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/referencing/items/1.json?profile=rel-as-key",
            project,
        )
        self.assertEqual(j["properties"]["fk"], 1)

        # Setup the widget
        referencing_layer.setEditorWidgetSetup(
            referencing_layer.fields().lookupField("fk"),
            QgsEditorWidgetSetup("RelationReference", {"Relation": "relation1"}),
        )

        # Check rel-as-key profile (default)
        feature = self._check_profile("rel-as-key", project)
        self.assertEqual(feature["properties"]["fk"], 1)

        # Check rel-as-uri
        feature = self._check_profile("rel-as-uri", project)
        self.assertEqual(
            feature["properties"]["fk"],
            "http://server.qgis.org/wfs3/collections/referenced/items/1.json?profile=rel-as-uri",
        )

        # Check that URI responds with the correct feature
        j = self._getJsonResponse(
            feature["properties"]["fk"],
            project,
        )
        self.assertEqual(j["properties"], {"fid": 1, "name": "test", "other_id": 1})

        # Check rel-as-link
        feature = self._check_profile("rel-as-link", project)
        self.assertEqual(
            feature["properties"]["fk"],
            {
                "href": "http://server.qgis.org/wfs3/collections/referenced/items/1.json?profile=rel-as-link",
                "title": "Related feature from layer 'referenced'",
            },
        )

        # Setup a widget with a value relation
        referencing_layer.setEditorWidgetSetup(
            referencing_layer.fields().lookupField("fk"),
            QgsEditorWidgetSetup(
                "ValueRelation",
                {"Layer": referenced_layer.id(), "Key": "other_id", "Value": "name"},
            ),
        )

        # Check rel-as-key profile (default)
        feature = self._check_profile("rel-as-key", project)
        self.assertEqual(feature["properties"]["fk"], 1)

        # Check rel-as-uri
        feature = self._check_profile("rel-as-uri", project)
        self.assertEqual(
            feature["properties"]["fk"],
            "http://server.qgis.org/wfs3/collections/referenced/items.json?profile=rel-as-uri&other_id=1",
        )

        # Check that the referencing feature item responds with the correct link
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/referencing/items/1.json?profile=rel-as-uri",
            project,
        )

        self.assertEqual(
            j["properties"]["fk"],
            "http://server.qgis.org/wfs3/collections/referenced/items.json?profile=rel-as-uri&other_id=1",
        )

        # Check that URI responds with the correct feature
        j = self._getJsonResponse(
            feature["properties"]["fk"],
            project,
        )

        self.assertEqual(
            j["features"][0]["properties"], {"fid": 1, "name": "test", "other_id": 1}
        )


if __name__ == "__main__":
    unittest.main()
