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

# Deterministic XML
os.environ["QT_HASH_SEED"] = "1"

from qgis.core import (
    Qgis,
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsEditorWidgetSetup,
    QgsFeature,
    QgsField,
    QgsFields,
    QgsGeometry,
    QgsProject,
    QgsProviderRegistry,
    QgsRelation,
    QgsRelationContext,
    QgsServerWmsDimensionProperties,
    QgsVectorLayer,
    QgsWkbTypes,
)
from qgis.PyQt import QtCore
from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServer,
)
from qgis.testing import unittest
from test_qgsserver_api import QgsServerAPITestBase


class QgsServerOgcApiFeaturesTest(QgsServerAPITestBase):
    """QGIS API server tests"""

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

    # Set to True in child classes to re-generate reference files for this class
    regenerate_api_reference = False

    def _getJsonResponse(self, url, project, expected_error=None):
        request = QgsBufferServerRequest(url)
        response = QgsBufferServerResponse()
        server = QgsServer()
        server.handleRequest(request, response, project)
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

    def testTemporalProperties(self):

        # Layer with timestamp
        layer = QgsVectorLayer(
            "Point?field=event_id:integer&field=event_date:date&field=event_date_end:date",
            "temporal",
            "memory",
        )
        self.assertTrue(layer.isValid())

        # Add features
        feature = QgsFeature(layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(0 0)"))
        feature.setAttribute("event_id", 1)
        feature.setAttribute("event_date", QtCore.QDate(2024, 1, 1))
        feature.setAttribute("event_date_end", QtCore.QDate(2024, 1, 2))
        self.assertTrue(layer.dataProvider().addFeature(feature))

        # Add to project and expose to WFS
        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        # Retrieve the feature and check temporal properties in the response

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json?profile=json-fg",
            project,
        )
        self.assertNotIn("time", j)

        info = QgsServerWmsDimensionProperties.WmsDimensionInfo("date", "event_date")
        layer.serverProperties().setWmsDimensions([info])

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json?profile=json-fg",
            project,
        )
        self.assertIn("event_date", j["properties"])
        self.assertEqual(j["time"], {"date": "2024-01-01"})

        info = QgsServerWmsDimensionProperties.WmsDimensionInfo(
            "date", "event_date", "event_date_end"
        )
        layer.serverProperties().setWmsDimensions([info])
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json?profile=json-fg",
            project,
        )
        self.assertIn("event_date", j["properties"])
        self.assertEqual(j["time"], {"interval": ["2024-01-01", "2024-01-02"]})

        # NOTE: we cannot test open intevals because the server's wmsDimensions API do not support anything
        #       other than instant and closed intervals.

        layer = QgsVectorLayer(
            "Point?field=event_id:integer&field=event_date:datetime&field=event_date_end:datetime",
            "temporal",
            "memory",
        )
        self.assertTrue(layer.isValid())

        # Add features
        feature = QgsFeature(layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(0 0)"))
        feature.setAttribute("event_id", 1)
        feature.setAttribute(
            "event_date",
            QtCore.QDateTime(
                QtCore.QDate(2024, 1, 1), QtCore.QTime(12, 0), QtCore.QTimeZone(3600)
            ),
        )
        feature.setAttribute(
            "event_date_end",
            QtCore.QDateTime(
                QtCore.QDate(2024, 1, 2), QtCore.QTime(12, 0), QtCore.QTimeZone(3600)
            ),
        )
        self.assertTrue(layer.dataProvider().addFeature(feature))

        info = QgsServerWmsDimensionProperties.WmsDimensionInfo("time", "event_date")
        layer.serverProperties().setWmsDimensions([info])

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json?profile=json-fg",
            project,
        )
        # Notice: the time is returned in UTC, not local time, so we expect 11:00:00Z instead of 12:00:00Z
        self.assertEqual(j["time"], {"timestamp": "2024-01-01T11:00:00Z"})

        info = QgsServerWmsDimensionProperties.WmsDimensionInfo(
            "time", "event_date", "event_date_end"
        )
        layer.serverProperties().setWmsDimensions([info])

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json?profile=json-fg",
            project,
        )
        self.assertEqual(
            j["time"], {"interval": ["2024-01-01T11:00:00Z", "2024-01-02T11:00:00Z"]}
        )

        # Check if a string field can be used as a temporal dimension
        layer = QgsVectorLayer(
            "Point?field=event_id:integer&field=event_date:string", "temporal", "memory"
        )
        self.assertTrue(layer.isValid())

        # Add features
        feature = QgsFeature(layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(0 0)"))
        feature.setAttribute("event_id", 1)
        feature.setAttribute("event_date", "2024-01-01T12:00:00+01:00")
        self.assertTrue(layer.dataProvider().addFeature(feature))

        info = QgsServerWmsDimensionProperties.WmsDimensionInfo("time", "event_date")
        layer.serverProperties().setWmsDimensions([info])

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json?profile=json-fg",
            project,
        )
        self.assertEqual(j["time"], {"timestamp": "2024-01-01T11:00:00Z"})

        # Test Rfc profile does not have any temporal properties
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json",
            project,
        )

    def testSingleFeatureJsonProfiles(self):

        layer = QgsVectorLayer(
            "Point?crs=EPSG:3857&field=event_id:integer&field=name:string",
            "temporal",
            "memory",
        )
        self.assertTrue(layer.isValid())

        # Add features
        feature = QgsFeature(layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(856053 5633001)"))
        feature.setAttribute("name", "test")
        layer.dataProvider().addFeature(feature)

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        # Test with no explicit profile, should return the default Legacy profile (CRS respected)
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json?crs=http://www.opengis.net/def/crs/EPSG/0/3857",
            project,
        )
        self.assertNotIn("place", j)
        self.assertAlmostEqual(j["geometry"]["coordinates"][0], 856053, delta=1)
        self.assertAlmostEqual(j["geometry"]["coordinates"][1], 5633001, delta=1)

        # Test with explicit profile RFC7946 error is returned because CRS is not OGC:CRS84
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json?profile=rfc7946&crs=http://www.opengis.net/def/crs/EPSG/0/3857",
            project,
            expected_error="Requested CRS must be OGC:CRS84 when requested profile is Rfc7946",
        )

        # Test with explicit profile JSON-FG
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json?profile=json-fg&crs=http://www.opengis.net/def/crs/EPSG/0/3857",
            project,
        )
        self.assertIn("place", j)

        # Test with explicit profile JSON-FG-PLUS
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items/1.json?profile=json-fg-plus&crs=http://www.opengis.net/def/crs/EPSG/0/3857",
            project,
        )
        self.assertIn("place", j)

        self.assertAlmostEqual(j["place"]["coordinates"][0], 856053, delta=1)
        self.assertAlmostEqual(j["place"]["coordinates"][1], 5633001, delta=1)
        self.assertAlmostEqual(j["geometry"]["coordinates"][0], 7.6, delta=1)
        self.assertAlmostEqual(j["geometry"]["coordinates"][1], 45.1, delta=1)

    def testCollectionJsonProfiles(self):

        layer = QgsVectorLayer(
            "Point?crs=EPSG:3857&field=event_id:integer&field=name:string",
            "temporal",
            "memory",
        )
        self.assertTrue(layer.isValid())

        # Add features
        feature = QgsFeature(layer.fields())
        feature.setGeometry(QgsGeometry.fromWkt("POINT(856053 5633001)"))
        feature.setAttribute("name", "test")
        layer.dataProvider().addFeature(feature)

        project = QgsProject()
        project.addMapLayer(layer)
        project.writeEntry("WFSLayers", "/", [layer.id()])

        # Test default (Legacy) profile (CRS is respected)
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items.json?crs=http://www.opengis.net/def/crs/EPSG/0/3857",
            project,
        )
        self.assertNotIn("place", j["features"][0])
        self.assertAlmostEqual(
            j["features"][0]["geometry"]["coordinates"][0], 856053, delta=1
        )
        self.assertAlmostEqual(
            j["features"][0]["geometry"]["coordinates"][1], 5633001, delta=1
        )

        # Test with explicit profile JSON-FG
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items.json?profile=json-fg&crs=http://www.opengis.net/def/crs/EPSG/0/3857",
            project,
        )
        self.assertIn("place", j["features"][0])
        self.assertAlmostEqual(
            j["features"][0]["place"]["coordinates"][0], 856053, delta=1
        )
        self.assertAlmostEqual(
            j["features"][0]["place"]["coordinates"][1], 5633001, delta=1
        )

        # Test CRS with explicit profile RFC7946 error is returned because
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items.json?profile=rfc7946&crs=http://www.opengis.net/def/crs/EPSG/0/3857",
            project,
            expected_error="Requested CRS must be OGC:CRS84 when requested profile is Rfc7946",
        )

        # Test with explicit profile JSON-FG-PLUS
        j = self._getJsonResponse(
            "http://server.qgis.org/wfs3/collections/temporal/items.json?profile=json-fg-plus&crs=http://www.opengis.net/def/crs/EPSG/0/3857",
            project,
        )
        self.assertIn("place", j["features"][0])
        self.assertAlmostEqual(
            j["features"][0]["place"]["coordinates"][0], 856053, delta=1
        )
        self.assertAlmostEqual(
            j["features"][0]["place"]["coordinates"][1], 5633001, delta=1
        )
        self.assertAlmostEqual(
            j["features"][0]["geometry"]["coordinates"][0], 7.6, delta=1
        )
        self.assertAlmostEqual(
            j["features"][0]["geometry"]["coordinates"][1], 45.1, delta=1
        )


if __name__ == "__main__":
    unittest.main()
