"""QGIS Unit tests for the OGC SensorThings API provider.

From build dir, run: ctest -R PyQgsSensorThingsProvider -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2023-11-08"

import hashlib
import tempfile
import unittest

from qgis.core import (
    Qgis,
    QgsFeatureRequest,
    QgsProviderRegistry,
    QgsRectangle,
    QgsSensorThingsExpansionDefinition,
    QgsSensorThingsUtils,
    QgsSettings,
    QgsVectorLayer,
)
from qgis.PyQt.QtCore import QDate, QDateTime, Qt, QTime, QVariant
from qgis.testing import QgisTestCase, start_app


def sanitize(endpoint, x):
    for prefix in (
        "/Locations",
        "/HistoricalLocations",
        "/Things",
        "/FeaturesOfInterest",
        "/MultiDatastreams",
    ):
        if x.startswith(prefix):
            x = x[len(prefix) :]
            endpoint = endpoint + "_" + prefix[1:]

    if len(endpoint + x) > 150:
        ret = endpoint + hashlib.md5(x.encode()).hexdigest()
        # print('Before: ' + endpoint + x)
        # print('After:  ' + ret)
        return ret
    return endpoint + x.replace("?", "_").replace("&", "_").replace("<", "_").replace(
        ">", "_"
    ).replace('"', "_").replace("'", "_").replace(" ", "_").replace(":", "_").replace(
        "/", "_"
    ).replace("\n", "_").replace("$", "_")


class TestPyQgsSensorThingsProvider(QgisTestCase):  # , ProviderTestCase):
    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        start_app()

        # On Windows we must make sure that any backslash in the path is
        # replaced by a forward slash so that QUrl can process it
        cls.basetestpath = tempfile.mkdtemp().replace("\\", "/")

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        # shutil.rmtree(cls.basetestpath, True)
        cls.vl = (
            None  # so as to properly close the provider and remove any temporary file
        )
        super().tearDownClass()

    def test_filter_for_wkb_type(self):
        """
        Test constructing a valid filter string which will return only
        features with a desired WKB type
        """
        self.assertEqual(
            QgsSensorThingsUtils.filterForWkbType(
                Qgis.SensorThingsEntity.Location, Qgis.WkbType.Point
            ),
            "location/type eq 'Point' or location/geometry/type eq 'Point'",
        )
        self.assertEqual(
            QgsSensorThingsUtils.filterForWkbType(
                Qgis.SensorThingsEntity.Location, Qgis.WkbType.PointZ
            ),
            "location/type eq 'Point' or location/geometry/type eq 'Point'",
        )
        self.assertEqual(
            QgsSensorThingsUtils.filterForWkbType(
                Qgis.SensorThingsEntity.FeatureOfInterest, Qgis.WkbType.Polygon
            ),
            "feature/type eq 'Polygon' or feature/geometry/type eq 'Polygon'",
        )
        self.assertEqual(
            QgsSensorThingsUtils.filterForWkbType(
                Qgis.SensorThingsEntity.Location, Qgis.WkbType.LineString
            ),
            "location/type eq 'LineString' or location/geometry/type eq 'LineString'",
        )
        self.assertEqual(
            QgsSensorThingsUtils.filterForWkbType(
                Qgis.SensorThingsEntity.MultiDatastream, Qgis.WkbType.Polygon
            ),
            "observedArea/type eq 'Polygon' or observedArea/geometry/type eq 'Polygon'",
        )

    def test_utils_string_to_entity(self):
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity("x"), Qgis.SensorThingsEntity.Invalid
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" thing "),
            Qgis.SensorThingsEntity.Thing,
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" LOCATION "),
            Qgis.SensorThingsEntity.Location,
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" HistoricalLocation "),
            Qgis.SensorThingsEntity.HistoricalLocation,
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" datastream "),
            Qgis.SensorThingsEntity.Datastream,
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" Sensor "),
            Qgis.SensorThingsEntity.Sensor,
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" ObservedProperty "),
            Qgis.SensorThingsEntity.ObservedProperty,
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" Observation "),
            Qgis.SensorThingsEntity.Observation,
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" FeatureOfInterest "),
            Qgis.SensorThingsEntity.FeatureOfInterest,
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" MultiDataStream "),
            Qgis.SensorThingsEntity.MultiDatastream,
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" Feature "),
            Qgis.SensorThingsEntity.Feature,
        )
        self.assertEqual(
            QgsSensorThingsUtils.stringToEntity(" FeatureType "),
            Qgis.SensorThingsEntity.FeatureType,
        )

    def test_utils_string_to_entityset(self):
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity("x"),
            Qgis.SensorThingsEntity.Invalid,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" things "),
            Qgis.SensorThingsEntity.Thing,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" LOCATIONs "),
            Qgis.SensorThingsEntity.Location,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" HistoricalLocations "),
            Qgis.SensorThingsEntity.HistoricalLocation,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" datastreams "),
            Qgis.SensorThingsEntity.Datastream,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" Sensors "),
            Qgis.SensorThingsEntity.Sensor,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" ObservedProperties "),
            Qgis.SensorThingsEntity.ObservedProperty,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" Observations "),
            Qgis.SensorThingsEntity.Observation,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" FeaturesOfInterest "),
            Qgis.SensorThingsEntity.FeatureOfInterest,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" Features "),
            Qgis.SensorThingsEntity.Feature,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" FeatureTypes "),
            Qgis.SensorThingsEntity.FeatureType,
        )
        self.assertEqual(
            QgsSensorThingsUtils.entitySetStringToEntity(" MultidataStreams "),
            Qgis.SensorThingsEntity.MultiDatastream,
        )

    def test_utils_entity_to_set_string(self):
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(Qgis.SensorThingsEntity.Invalid),
            "",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(Qgis.SensorThingsEntity.Thing),
            "Things",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(Qgis.SensorThingsEntity.Location),
            "Locations",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(
                Qgis.SensorThingsEntity.HistoricalLocation
            ),
            "HistoricalLocations",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(Qgis.SensorThingsEntity.Datastream),
            "Datastreams",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(Qgis.SensorThingsEntity.Sensor),
            "Sensors",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(
                Qgis.SensorThingsEntity.ObservedProperty
            ),
            "ObservedProperties",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(Qgis.SensorThingsEntity.Observation),
            "Observations",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(
                Qgis.SensorThingsEntity.FeatureOfInterest
            ),
            "FeaturesOfInterest",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(Qgis.SensorThingsEntity.Feature),
            "Features",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(Qgis.SensorThingsEntity.FeatureType),
            "FeatureTypes",
        )
        self.assertEqual(
            QgsSensorThingsUtils.entityToSetString(
                Qgis.SensorThingsEntity.MultiDatastream
            ),
            "MultiDatastreams",
        )

    def test_expansion_definition(self):
        """
        Test QgsSensorThingsExpansionDefinition
        """
        expansion = QgsSensorThingsExpansionDefinition()
        self.assertFalse(expansion.isValid())
        self.assertFalse(expansion.asQueryString(Qgis.SensorThingsEntity.Invalid))

        # test getters/setters
        expansion = QgsSensorThingsExpansionDefinition(
            Qgis.SensorThingsEntity.ObservedProperty
        )
        self.assertTrue(expansion.isValid())
        self.assertEqual(
            expansion.childEntity(), Qgis.SensorThingsEntity.ObservedProperty
        )
        self.assertEqual(expansion.limit(), 100)
        self.assertFalse(expansion.filter())
        self.assertEqual(
            repr(expansion),
            "<QgsSensorThingsExpansionDefinition: ObservedProperty limit 100>",
        )
        self.assertEqual(
            expansion.asQueryString(Qgis.SensorThingsEntity.Datastream),
            "$expand=ObservedProperty($top=100)",
        )
        self.assertEqual(
            expansion.asQueryString(
                Qgis.SensorThingsEntity.Datastream, ["$expand=Locations($top=101)"]
            ),
            "$expand=ObservedProperty($top=100;$expand=Locations($top=101))",
        )

        expansion.setChildEntity(Qgis.SensorThingsEntity.Location)
        self.assertEqual(expansion.childEntity(), Qgis.SensorThingsEntity.Location)
        self.assertEqual(
            repr(expansion), "<QgsSensorThingsExpansionDefinition: Location limit 100>"
        )
        self.assertEqual(
            expansion.asQueryString(Qgis.SensorThingsEntity.Thing),
            "$expand=Locations($top=100)",
        )
        self.assertEqual(
            expansion.asQueryString(
                Qgis.SensorThingsEntity.Thing, ["$expand=Datastreams($top=101)"]
            ),
            "$expand=Locations($top=100;$expand=Datastreams($top=101))",
        )

        expansion.setLimit(-1)
        self.assertEqual(expansion.limit(), -1)
        self.assertEqual(
            repr(expansion), "<QgsSensorThingsExpansionDefinition: Location>"
        )
        self.assertEqual(
            expansion.asQueryString(Qgis.SensorThingsEntity.Thing), "$expand=Locations"
        )
        self.assertEqual(
            expansion.asQueryString(
                Qgis.SensorThingsEntity.Thing, ["$expand=Datastreams($top=101)"]
            ),
            "$expand=Locations($expand=Datastreams($top=101))",
        )

        expansion.setOrderBy("id")
        self.assertEqual(expansion.orderBy(), "id")
        self.assertEqual(
            repr(expansion),
            "<QgsSensorThingsExpansionDefinition: Location by id (asc)>",
        )
        self.assertEqual(
            expansion.asQueryString(Qgis.SensorThingsEntity.Thing),
            "$expand=Locations($orderby=id)",
        )
        self.assertEqual(
            expansion.asQueryString(
                Qgis.SensorThingsEntity.Thing, ["$expand=Datastreams($top=101)"]
            ),
            "$expand=Locations($orderby=id;$expand=Datastreams($top=101))",
        )
        expansion.setSortOrder(Qt.SortOrder.DescendingOrder)
        self.assertEqual(expansion.sortOrder(), Qt.SortOrder.DescendingOrder)
        self.assertEqual(
            repr(expansion),
            "<QgsSensorThingsExpansionDefinition: Location by id (desc)>",
        )
        self.assertEqual(
            expansion.asQueryString(Qgis.SensorThingsEntity.Thing),
            "$expand=Locations($orderby=id desc)",
        )
        self.assertEqual(
            expansion.asQueryString(
                Qgis.SensorThingsEntity.Thing, ["$expand=Datastreams($top=101)"]
            ),
            "$expand=Locations($orderby=id desc;$expand=Datastreams($top=101))",
        )

        expansion.setLimit(3)
        self.assertEqual(
            repr(expansion),
            "<QgsSensorThingsExpansionDefinition: Location by id (desc), limit 3>",
        )
        self.assertEqual(
            expansion.asQueryString(Qgis.SensorThingsEntity.Thing),
            "$expand=Locations($orderby=id desc;$top=3)",
        )
        self.assertEqual(
            expansion.asQueryString(
                Qgis.SensorThingsEntity.Thing, ["$expand=Datastreams($top=101)"]
            ),
            "$expand=Locations($orderby=id desc;$top=3;$expand=Datastreams($top=101))",
        )

        expansion.setFilter("result eq 1")
        self.assertEqual(expansion.filter(), "result eq 1")
        self.assertEqual(
            repr(expansion),
            "<QgsSensorThingsExpansionDefinition: Location by id (desc), limit 3, filter 'result eq 1'>",
        )
        self.assertEqual(
            expansion.asQueryString(Qgis.SensorThingsEntity.Thing),
            "$expand=Locations($orderby=id desc;$top=3;$filter=result eq 1)",
        )
        self.assertEqual(
            expansion.asQueryString(
                Qgis.SensorThingsEntity.Thing, ["$expand=Datastreams($top=101)"]
            ),
            "$expand=Locations($orderby=id desc;$top=3;$filter=result eq 1;$expand=Datastreams($top=101))",
        )

        # test equality
        expansion1 = QgsSensorThingsExpansionDefinition(
            Qgis.SensorThingsEntity.ObservedProperty
        )
        expansion2 = QgsSensorThingsExpansionDefinition(
            Qgis.SensorThingsEntity.ObservedProperty
        )
        self.assertEqual(expansion1, expansion2)
        self.assertNotEqual(expansion1, QgsSensorThingsExpansionDefinition())
        self.assertNotEqual(QgsSensorThingsExpansionDefinition(), expansion2)
        self.assertEqual(
            QgsSensorThingsExpansionDefinition(), QgsSensorThingsExpansionDefinition()
        )

        expansion2.setChildEntity(Qgis.SensorThingsEntity.Sensor)
        self.assertNotEqual(expansion1, expansion2)
        expansion2.setChildEntity(Qgis.SensorThingsEntity.ObservedProperty)
        self.assertEqual(expansion1, expansion2)

        expansion2.setOrderBy("x")
        self.assertNotEqual(expansion1, expansion2)
        expansion2.setOrderBy("")
        self.assertEqual(expansion1, expansion2)

        expansion2.setSortOrder(Qt.SortOrder.DescendingOrder)
        self.assertNotEqual(expansion1, expansion2)
        expansion2.setSortOrder(Qt.SortOrder.AscendingOrder)
        self.assertEqual(expansion1, expansion2)

        expansion2.setLimit(33)
        self.assertNotEqual(expansion1, expansion2)
        expansion2.setLimit(100)
        self.assertEqual(expansion1, expansion2)

        expansion2.setFilter("result eq 1")
        self.assertNotEqual(expansion1, expansion2)
        expansion2.setFilter("")
        self.assertEqual(expansion1, expansion2)

        # test to/from string
        expansion = QgsSensorThingsExpansionDefinition()
        string = expansion.toString()
        self.assertFalse(string)
        res = QgsSensorThingsExpansionDefinition.fromString(string)
        self.assertFalse(res.isValid())

        expansion.setChildEntity(Qgis.SensorThingsEntity.Sensor)
        expansion.setLimit(-1)
        string = expansion.toString()
        res = QgsSensorThingsExpansionDefinition.fromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.childEntity(), Qgis.SensorThingsEntity.Sensor)
        self.assertFalse(res.orderBy())
        self.assertEqual(res.limit(), -1)

        expansion.setOrderBy("test")
        string = expansion.toString()
        res = QgsSensorThingsExpansionDefinition.fromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.childEntity(), Qgis.SensorThingsEntity.Sensor)
        self.assertEqual(res.orderBy(), "test")
        self.assertEqual(res.sortOrder(), Qt.SortOrder.AscendingOrder)
        self.assertEqual(res.limit(), -1)

        expansion.setSortOrder(Qt.SortOrder.DescendingOrder)
        string = expansion.toString()
        res = QgsSensorThingsExpansionDefinition.fromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.childEntity(), Qgis.SensorThingsEntity.Sensor)
        self.assertEqual(res.orderBy(), "test")
        self.assertEqual(res.sortOrder(), Qt.SortOrder.DescendingOrder)
        self.assertEqual(res.limit(), -1)

        expansion.setLimit(5)
        string = expansion.toString()
        res = QgsSensorThingsExpansionDefinition.fromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.childEntity(), Qgis.SensorThingsEntity.Sensor)
        self.assertEqual(res.orderBy(), "test")
        self.assertEqual(res.sortOrder(), Qt.SortOrder.DescendingOrder)
        self.assertEqual(res.limit(), 5)

        expansion.setOrderBy("")
        string = expansion.toString()
        res = QgsSensorThingsExpansionDefinition.fromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.childEntity(), Qgis.SensorThingsEntity.Sensor)
        self.assertFalse(res.orderBy())
        self.assertEqual(res.limit(), 5)

        expansion.setFilter("request eq 1:2")
        string = expansion.toString()
        res = QgsSensorThingsExpansionDefinition.fromString(string)
        self.assertTrue(res.isValid())
        self.assertEqual(res.filter(), expansion.filter())

    def test_expansions_as_query_string(self):
        """
        Test constructing query strings from a list of expansions
        """
        self.assertFalse(
            QgsSensorThingsUtils.asQueryString(Qgis.SensorThingsEntity.Invalid, [])
        )
        self.assertEqual(
            QgsSensorThingsUtils.asQueryString(
                Qgis.SensorThingsEntity.Thing,
                [
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.Location, orderBy="id", limit=3
                    )
                ],
            ),
            "$expand=Locations($orderby=id;$top=3)",
        )
        self.assertEqual(
            QgsSensorThingsUtils.asQueryString(
                Qgis.SensorThingsEntity.Thing,
                [
                    QgsSensorThingsExpansionDefinition(),
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.Datastream, orderBy="id", limit=3
                    ),
                ],
            ),
            "$expand=Datastreams($orderby=id;$top=3)",
        )
        self.assertEqual(
            QgsSensorThingsUtils.asQueryString(
                Qgis.SensorThingsEntity.Thing,
                [
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.Datastream, orderBy="id", limit=3
                    )
                ],
            ),
            "$expand=Datastreams($orderby=id;$top=3)",
        )
        self.assertEqual(
            QgsSensorThingsUtils.asQueryString(
                Qgis.SensorThingsEntity.Observation,
                [
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.Datastream, orderBy="id", limit=3
                    )
                ],
            ),
            "$expand=Datastream($orderby=id;$top=3)",
        )

        self.assertEqual(
            QgsSensorThingsUtils.asQueryString(
                Qgis.SensorThingsEntity.Thing,
                [
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.Location, orderBy="id", limit=3
                    ),
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.Sensor, orderBy="description", limit=30
                    ),
                ],
            ),
            "$expand=Locations($orderby=id;$top=3;$expand=Sensors($orderby=description;$top=30))",
        )
        self.assertEqual(
            QgsSensorThingsUtils.asQueryString(
                Qgis.SensorThingsEntity.Location,
                [
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.Thing, orderBy="id", limit=3
                    ),
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.Datastream,
                        orderBy="description",
                        limit=30,
                    ),
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.ObservedProperty,
                        orderBy="name",
                        limit=-1,
                    ),
                ],
            ),
            "$expand=Things($orderby=id;$top=3;$expand=Datastreams($orderby=description;$top=30;$expand=ObservedProperty($orderby=name)))",
        )

    def test_fields_for_entity_type(self):
        """
        Test calculating fields for an entity type.
        """
        fields = QgsSensorThingsUtils.fieldsForEntityType(
            Qgis.SensorThingsEntity.Datastream
        )
        self.assertEqual(
            [field.name() for field in fields],
            [
                "id",
                "selfLink",
                "name",
                "description",
                "unitOfMeasurement",
                "observationType",
                "properties",
                "phenomenonTimeStart",
                "phenomenonTimeEnd",
                "resultTimeStart",
                "resultTimeEnd",
            ],
        )

        # hide proxy fields for missing interval field type
        fields = QgsSensorThingsUtils.fieldsForEntityType(
            Qgis.SensorThingsEntity.Datastream, False
        )
        self.assertEqual(
            [field.name() for field in fields],
            [
                "id",
                "selfLink",
                "name",
                "description",
                "unitOfMeasurement",
                "observationType",
                "properties",
            ],
        )

    def test_fields_for_expanded_entity(self):
        """
        Test calculating fields for an expanded entity
        """
        fields = QgsSensorThingsUtils.fieldsForExpandedEntityType(
            Qgis.SensorThingsEntity.Location, []
        )
        self.assertEqual(
            [field.name() for field in fields],
            ["id", "selfLink", "name", "description", "properties"],
        )
        fields = QgsSensorThingsUtils.fieldsForExpandedEntityType(
            Qgis.SensorThingsEntity.Location, [Qgis.SensorThingsEntity.Thing]
        )
        self.assertEqual(
            [field.name() for field in fields],
            [
                "id",
                "selfLink",
                "name",
                "description",
                "properties",
                "Thing_id",
                "Thing_selfLink",
                "Thing_name",
                "Thing_description",
                "Thing_properties",
            ],
        )
        fields = QgsSensorThingsUtils.fieldsForExpandedEntityType(
            Qgis.SensorThingsEntity.Location,
            [Qgis.SensorThingsEntity.Thing, Qgis.SensorThingsEntity.Datastream],
        )
        self.assertEqual(
            [field.name() for field in fields],
            [
                "id",
                "selfLink",
                "name",
                "description",
                "properties",
                "Thing_id",
                "Thing_selfLink",
                "Thing_name",
                "Thing_description",
                "Thing_properties",
                "Thing_Datastream_id",
                "Thing_Datastream_selfLink",
                "Thing_Datastream_name",
                "Thing_Datastream_description",
                "Thing_Datastream_unitOfMeasurement",
                "Thing_Datastream_observationType",
                "Thing_Datastream_properties",
                "Thing_Datastream_phenomenonTimeStart",
                "Thing_Datastream_phenomenonTimeEnd",
                "Thing_Datastream_resultTimeStart",
                "Thing_Datastream_resultTimeEnd",
            ],
        )

    def test_expandable_targets(self):
        """
        Test valid expansion targets for entity types
        """
        self.assertEqual(
            QgsSensorThingsUtils.expandableTargets(Qgis.SensorThingsEntity.Thing),
            [
                Qgis.SensorThingsEntity.HistoricalLocation,
                Qgis.SensorThingsEntity.Datastream,
                Qgis.SensorThingsEntity.MultiDatastream,
            ],
        )

    def test_filter_for_extent(self):
        """
        Test constructing valid filter strings for features which intersect
        an extent
        """
        self.assertFalse(QgsSensorThingsUtils.filterForExtent("", QgsRectangle()))
        self.assertFalse(QgsSensorThingsUtils.filterForExtent("test", QgsRectangle()))
        self.assertFalse(
            QgsSensorThingsUtils.filterForExtent("", QgsRectangle(1, 2, 3, 4))
        )
        self.assertEqual(
            QgsSensorThingsUtils.filterForExtent("test", QgsRectangle(1, 2, 3, 4)),
            "geo.intersects(test, geography'Polygon ((1 2, 3 2, 3 4, 1 4, 1 2))')",
        )

    def test_combine_filters(self):
        """
        Test combining multiple filter strings into one
        """
        self.assertFalse(QgsSensorThingsUtils.combineFilters([]))
        self.assertFalse(QgsSensorThingsUtils.combineFilters([""]))
        self.assertEqual(QgsSensorThingsUtils.combineFilters(["", "a eq 1"]), "a eq 1")
        self.assertEqual(
            QgsSensorThingsUtils.combineFilters(["a eq 1", "b eq 2"]),
            "(a eq 1) and (b eq 2)",
        )
        self.assertEqual(
            QgsSensorThingsUtils.combineFilters(["a eq 1", "", "b eq 2", "c eq 3"]),
            "(a eq 1) and (b eq 2) and (c eq 3)",
        )

    def test_invalid_layer(self):
        """
        Test construction of layers using bad URLs
        """
        vl = QgsVectorLayer(
            "url='http://fake.com/fake_qgis_http_endpoint'", "test", "sensorthings"
        )
        self.assertFalse(vl.isValid())
        self.assertEqual(
            vl.dataProvider().error().summary()[:32], "Connection failed: Error opening"
        )
        self.assertEqual(
            vl.dataProvider().error().summary()[-25:], "No such file or directory"
        )

    def test_layer_invalid_json(self):
        """
        Test that connecting to services which return non-parsable JSON
        are cleanly handled (i.e. no crashes!)
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": ["""
                )

            vl = QgsVectorLayer(f"url='http://{endpoint}'", "test", "sensorthings")
            self.assertFalse(vl.isValid())
            self.assertIn("parse error", vl.dataProvider().error().summary())

    def test_layer(self):
        """
        Test construction of a basic layer using a valid SensorThings endpoint
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Datastreams",
      "url": "endpoint/Datastreams"
    },
    {
      "name": "MultiDatastreams",
      "url": "endpoint/MultiDatastreams"
    },
    {
      "name": "FeaturesOfInterest",
      "url": "endpoint/FeaturesOfInterest"
    },
    {
      "name": "HistoricalLocations",
      "url": "endpoint/HistoricalLocations"
    },
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    },
    {
      "name": "Observations",
      "url": "endpoint/Observations"
    },
    {
      "name": "ObservedProperties",
      "url": "endpoint/ObservedProperties"
    },
    {
      "name": "Sensors",
      "url": "endpoint/Sensors"
    },
    {
      "name": "Things",
      "url": "endpointThings"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write("""{"@iot.count":4962,"value":[]}""")

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            # pessimistic "worst case" extent should be used
            self.assertEqual(vl.extent(), QgsRectangle(-180, -90, 180, 90))
            self.assertEqual(vl.featureCount(), 4962)
            self.assertIn("Entity Type</td><td>Location</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"', vl.htmlMetadata())
            # we assume version 1.1 if we can't determine exactly
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 1.1)

            # As multipoint
            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=MultiPointZ entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.wkbType(), Qgis.WkbType.MultiPointZ)

            # As line
            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=MultiLineStringZ entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.wkbType(), Qgis.WkbType.MultiLineStringZ)

            # As polygon
            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=MultiPolygonZ entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.wkbType(), Qgis.WkbType.MultiPolygonZ)

    def test_layer_conformance_1_1(self):
        """
        Test construction of a basic layer using a with conformance array,
        for a version 1.1 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Datastreams",
      "url": "endpoint/Datastreams"
    },
    {
      "name": "MultiDatastreams",
      "url": "endpoint/MultiDatastreams"
    },
    {
      "name": "FeaturesOfInterest",
      "url": "endpoint/FeaturesOfInterest"
    },
    {
      "name": "HistoricalLocations",
      "url": "endpoint/HistoricalLocations"
    },
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    },
    {
      "name": "Observations",
      "url": "endpoint/Observations"
    },
    {
      "name": "ObservedProperties",
      "url": "endpoint/ObservedProperties"
    },
    {
      "name": "Sensors",
      "url": "endpoint/Sensors"
    },
    {
      "name": "Things",
      "url": "endpointThings"
    }
  ],
  "serverSettings": {
"conformance": [
  "http://www.opengis.net/spec/iot_sensing/1.1/req/batch-request/batch-request",
  "http://www.opengis.net/spec/iot_sensing/1.1/req/create-observations-via-mqtt/observations-creation",
  "http://www.opengis.net/spec/iot_sensing/1.1/req/create-update-delete",
  "http://www.opengis.net/spec/iot_sensing/1.1/req/data-array/data-array",
  "http://www.opengis.net/spec/iot_sensing/1.1/req/datamodel",
  "http://www.opengis.net/spec/iot_sensing/1.1/req/multi-datastream",
  "http://www.opengis.net/spec/iot_sensing/1.1/req/receive-updates-via-mqtt/receive-updates",
  "http://www.opengis.net/spec/iot_sensing/1.1/req/request-data",
  "http://www.opengis.net/spec/iot_sensing/1.1/req/resource-path/resource-path-to-entities"
]
}
}""".replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write("""{"@iot.count":4962,"value":[]}""")

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertIn("SensorThings Version</td><td>1.1</td>", vl.htmlMetadata())
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 1.1)

    def test_layer_conformance_2(self):
        """
        Test construction of a basic layer using a with conformance array,
        for a version 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Datastreams",
      "url": "endpoint/Datastreams"
    },
    {
      "name": "MultiDatastreams",
      "url": "endpoint/MultiDatastreams"
    },
    {
      "name": "FeaturesOfInterest",
      "url": "endpoint/FeaturesOfInterest"
    },
    {
      "name": "HistoricalLocations",
      "url": "endpoint/HistoricalLocations"
    },
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    },
    {
      "name": "Observations",
      "url": "endpoint/Observations"
    },
    {
      "name": "ObservedProperties",
      "url": "endpoint/ObservedProperties"
    },
    {
      "name": "Sensors",
      "url": "endpoint/Sensors"
    },
    {
      "name": "Things",
      "url": "endpointThings"
    }
  ],
  "serverSettings": {
"conformance": [
"http://www.opengis.net/spec/iot_sensing/1.1/req/batch-request/batch-request",
      "http://www.opengis.net/spec/iot_sensing/1.1/req/data-array/data-array",
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/api/cud",
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/api/read",
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/binding/http",
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/binding/mqtt",
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core",
      "http://www.opengis.net/spec/sensorthings/2.0/req/api/cud/deep_update",
      "http://www.opengis.net/spec/sensorthings/2.0/req/api/cud/json_patch",
      "http://www.opengis.net/spec/sensorthings/2.0/req/api/cud/replace",
      "http://www.opengis.net/spec/sensorthings/2.0/req/api/read/options/select_distinct",
      "http://www.opengis.net/spec/sensorthings/2.0/req/binding/http/request_response",
      "http://www.opengis.net/spec/sensorthings/2.0/req/binding/mqtt/pub_sub",
      "http://www.opengis.net/spec/sensorthings/2.0/req/binding/mqtt/pub_sub/expand",
      "http://www.opengis.net/spec/sensorthings/2.0/req/binding/mqtt/pub_sub/select",
      "http://www.opengis.net/spec/sensorthings/2.0/req/binding/mqtt/request_response",
      "http://www.opengis.net/spec/sensorthings/2.0/req/binding/mqtt/simple_create"
]
}
}""".replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write("""{"@count":4962,"value":[]}""")

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertIn("SensorThings Version</td><td>2.0</td>", vl.htmlMetadata())
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

    def test_thing(self):
        """
        Test a layer retrieving 'Thing' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Things",
      "url": "endpoint/Things"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/Things?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/Things?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Things(1)",
      "@iot.id": 1,
      "name": "Thing 1",
      "description": "Desc 1",
      "properties": {
        "owner": "owner 1"
      },
      "Locations@iot.navigationLink": "endpoint/Things(1)/Locations",
      "HistoricalLocations@iot.navigationLink": "endpoint/Things(1)/HistoricalLocations",
      "Datastreams@iot.navigationLink": "endpoint/Things(1)/Datastreams",
      "MultiDatastreams@iot.navigationLink": "endpoint/Things(1)/MultiDatastreams"
    },
    {
      "@iot.selfLink": "endpoint/Things(2)",
      "@iot.id": 2,
      "name": "Thing 2",
      "description": "Desc 2",
      "properties": {
        "owner": "owner 2"
      },
      "Locations@iot.navigationLink": "endpoint/Things(2)/Locations",
      "HistoricalLocations@iot.navigationLink": "endpoint/Things(2)/HistoricalLocations",
      "Datastreams@iot.navigationLink": "endpoint/Things(2)/Datastreams",
      "MultiDatastreams@iot.navigationLink": "endpoint/Things(2)/MultiDatastreams"
    }
  ],
  "@iot.nextLink": "endpoint/Things?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/Things?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/Things(3)",
                  "@iot.id": 3,
                  "name": "Thing 3",
                  "description": "Desc 3",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Locations@iot.navigationLink": "endpoint/Things(3)/Locations",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Things(3)/HistoricalLocations",
                  "Datastreams@iot.navigationLink": "endpoint/Things(3)/Datastreams",
                  "MultiDatastreams@iot.navigationLink": "endpoint/Things(3)/MultiDatastreams"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='Thing'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertTrue(vl.extent().isNull())
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn("Entity Type</td><td>Thing</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Things"', vl.htmlMetadata())
            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-10:] for f in features],
                ["/Things(1)", "/Things(2)", "/Things(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features], ["Thing 1", "Thing 2", "Thing 3"]
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

    def test_thing_2_0(self):
        """
        Test a layer retrieving 'Thing' entities from a service, 2.0 version
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Things",
      "url": "endpoint/Things"
    }
  ],
  "serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/Things?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Things","@count":3,"value":[]}"""
                )

            with open(
                sanitize(endpoint, "/Things?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Things",
  "value": [
    {
      "@id": "endpoint/Things(1)",
      "id": 1,
      "name": "Thing 1",
      "description": "Desc 1",
      "properties": {
        "owner": "owner 1"
      },
      "Locations@navigationLink": "endpoint/Things(1)/Locations",
      "HistoricalLocations@navigationLink": "endpoint/Things(1)/HistoricalLocations",
      "Datastreams@navigationLink": "endpoint/Things(1)/Datastreams",
      "MultiDatastreams@navigationLink": "endpoint/Things(1)/MultiDatastreams"
    },
    {
      "@id": "endpoint/Things(2)",
      "id": 2,
      "name": "Thing 2",
      "description": "Desc 2",
      "properties": {
        "owner": "owner 2"
      },
      "Locations@navigationLink": "endpoint/Things(2)/Locations",
      "HistoricalLocations@navigationLink": "endpoint/Things(2)/HistoricalLocations",
      "Datastreams@navigationLink": "endpoint/Things(2)/Datastreams",
      "MultiDatastreams@navigationLink": "endpoint/Things(2)/MultiDatastreams"
    }
  ],
  "@nextLink": "endpoint/Things?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/Things?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Things",
              "value": [
                {
                  "@id": "endpoint/Things(3)",
                  "id": 3,
                  "name": "Thing 3",
                  "description": "Desc 3",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Locations@iot.navigationLink": "endpoint/Things(3)/Locations",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Things(3)/HistoricalLocations",
                  "Datastreams@iot.navigationLink": "endpoint/Things(3)/Datastreams",
                  "MultiDatastreams@iot.navigationLink": "endpoint/Things(3)/MultiDatastreams"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='Thing'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)
            self.assertTrue(vl.extent().isNull())
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn("Entity Type</td><td>Thing</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Things"', vl.htmlMetadata())
            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-10:] for f in features],
                ["/Things(1)", "/Things(2)", "/Things(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features], ["Thing 1", "Thing 2", "Thing 3"]
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

    def test_location(self):
        """
        Test a layer retrieving 'Location' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Locations(1)",
      "@iot.id": 1,
      "name": "Location 1",
      "description": "Desc 1",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          11.6,
          52.1
        ]
      },
      "properties": {
        "owner": "owner 1"
      },
      "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/Locations(2)",
      "@iot.id": 2,
      "name": "Location 2",
      "description": "Desc 2",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          12.6,
          53.1
        ]
      },
      "properties": {
        "owner": "owner 2"
      },
      "Things@iot.navigationLink": "endpoint/Locations(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/Locations(3)",
                  "@iot.id": 3,
                  "name": "Location 3",
                  "description": "Desc 3",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      13.6,
                      55.1
                    ]
                  },
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@iot.navigationLink": "endpoint/Locations(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            # pessimistic "worst case" extent should initially be used
            self.assertEqual(vl.extent(), QgsRectangle(-180, -90, 180, 90))
            self.assertEqual(vl.featureCount(), 3)
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Location</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(2)", "/Locations(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1", "Location 2", "Location 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (11.6 52.1)", "Point (12.6 53.1)", "Point (13.6 55.1)"],
            )

            # all features fetched, accurate extent should be returned
            self.assertEqual(vl.extent(), QgsRectangle(11.6, 52.1, 13.6, 55.1))

    def test_location_2_0(self):
        """
        Test a layer retrieving 'Location' entities from a version 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    }
  ],
  "serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Locations","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Locations",
  "value": [
    {
      "@id": "endpoint/Locations(1)",
      "id": 1,
      "name": "Location 1",
      "description": "Desc 1",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          11.6,
          52.1
        ]
      },
      "properties": {
        "owner": "owner 1"
      },
      "Things@navigationLink": "endpoint/Locations(1)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Locations(1)/HistoricalLocations"
    },
    {
      "@id": "endpoint/Locations(2)",
      "id": 2,
      "name": "Location 2",
      "description": "Desc 2",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          12.6,
          53.1
        ]
      },
      "properties": {
        "owner": "owner 2"
      },
      "Things@navigationLink": "endpoint/Locations(2)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Locations(2)/HistoricalLocations"

    }
  ],
  "@nextLink": "endpoint/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Locations",
              "value": [
                {
                  "@id": "endpoint/Locations(3)",
                  "id": 3,
                  "name": "Location 3",
                  "description": "Desc 3",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      13.6,
                      55.1
                    ]
                  },
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@navigationLink": "endpoint/Locations(3)/Things",
                  "HistoricalLocations@navigationLink": "endpoint/Locations(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)
            # pessimistic "worst case" extent should initially be used
            self.assertEqual(vl.extent(), QgsRectangle(-180, -90, 180, 90))
            self.assertEqual(vl.featureCount(), 3)
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Location</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(2)", "/Locations(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1", "Location 2", "Location 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (11.6 52.1)", "Point (12.6 53.1)", "Point (13.6 55.1)"],
            )

            # all features fetched, accurate extent should be returned
            self.assertEqual(vl.extent(), QgsRectangle(11.6, 52.1, 13.6, 55.1))

    def test_location_formalism(self):
        """
        Test https://github.com/qgis/QGIS/issues/56732
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Locations(1)",
      "@iot.id": 1,
      "name": "Location 1",
      "description": "Desc 1",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Feature",
        "geometry": {
          "type": "Point",
          "coordinates": [
            11.6,
            52.1
          ]
        }
      },
      "properties": {
        "owner": "owner 1"
      },
      "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/Locations(2)",
      "@iot.id": 2,
      "name": "Location 2",
      "description": "Desc 2",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Feature",
        "geometry": {
          "type": "Point",
          "coordinates": [
            12.6,
            53.1
          ]
        }
      },
      "properties": {
        "owner": "owner 2"
      },
      "Things@iot.navigationLink": "endpoint/Locations(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/Locations(3)",
                  "@iot.id": 3,
                  "name": "Location 3",
                  "description": "Desc 3",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "feature",
                    "geometry": {
                      "type": "Point",
                      "coordinates": [
                        13.6,
                        55.1
                      ]
                    }
                  },
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@iot.navigationLink": "endpoint/Locations(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            # pessimistic "worst case" extent should initially be used
            self.assertEqual(vl.extent(), QgsRectangle(-180, -90, 180, 90))
            self.assertEqual(vl.featureCount(), 3)
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Location</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(2)", "/Locations(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1", "Location 2", "Location 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (11.6 52.1)", "Point (12.6 53.1)", "Point (13.6 55.1)"],
            )

            # all features fetched, accurate extent should be returned
            self.assertEqual(vl.extent(), QgsRectangle(11.6, 52.1, 13.6, 55.1))

    def test_filter_rect(self):
        """
        Test retrieving features using feature requests with filter
        rectangles set
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Locations(1)",
      "@iot.id": 1,
      "name": "Location 1",
      "description": "Desc 1",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          1.623373,
          52.132017
        ]
      },
      "properties": {
        "owner": "owner 1"
      },
      "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/Locations(2)",
      "@iot.id": 2,
      "name": "Location 2",
      "description": "Desc 2",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          12.623373,
          53.132017
        ]
      },
      "properties": {
        "owner": "owner 2"
      },
      "Things@iot.navigationLink": "endpoint/Locations(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/Locations(3)",
                  "@iot.id": 3,
                  "name": "Location 3",
                  "description": "Desc 3",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      3.623373,
                      55.132017
                    ]
                  },
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@iot.navigationLink": "endpoint/Locations(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((1 0, 10 0, 10 80, 1 80, 1 0))'))",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                          {
                  "@iot.selfLink": "endpoint/Locations(1)",
                  "@iot.id": 1,
                  "name": "Location 1",
                  "description": "Desc 1",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      1.623373,
                      52.132017
                    ]
                  },
                  "properties": {
                    "owner": "owner 1"
                  },
                  "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
                },
                {
                  "@iot.selfLink": "endpoint/Locations(3)",
                  "@iot.id": 3,
                  "name": "Location 3",
                  "description": "Desc 3",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      3.623373,
                      55.132017
                    ]
                  },
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@iot.navigationLink": "endpoint/Locations(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(3)/HistoricalLocations"
                }
              ]
            }""".replace("endpoint", "http://" + endpoint)
                    )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((10 0, 20 0, 20 80, 10 80, 10 0))'))",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
            {
              "value": [
                              {
      "@iot.selfLink": "endpoint/Locations(2)",
      "@iot.id": 2,
      "name": "Location 2",
      "description": "Desc 2",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          12.623373,
          53.132017
        ]
      },
      "properties": {
        "owner": "owner 2"
      },
      "Things@iot.navigationLink": "endpoint/Locations(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(2)/HistoricalLocations"

    }
              ]
            }""".replace("endpoint", "http://" + endpoint)
                )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.featureCount(), 3)
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Location</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"', vl.htmlMetadata())

            # test retrieving subset of features from a filter rect only
            request = QgsFeatureRequest()
            request.setFilterRect(QgsRectangle(1, 0, 10, 80))

            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["1", "3"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1", "Location 3"],
            )
            self.assertEqual([f["description"] for f in features], ["Desc 1", "Desc 3"])
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 3"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (1.6 52.1)", "Point (3.6 55.1)"],
            )

            # test retrieving a different subset with a different extent
            request = QgsFeatureRequest()
            request.setFilterRect(QgsRectangle(10, 0, 20, 80))

            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["2"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(2)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 2"],
            )
            self.assertEqual([f["description"] for f in features], ["Desc 2"])
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 2"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (12.6 53.1)"],
            )

            # a filter rect which covers all features
            request = QgsFeatureRequest()
            request.setFilterRect(QgsRectangle(0, 0, 20, 80))

            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["1", "3", "2"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(3)", "/Locations(2)"],
            )

    def test_filter_rect_2_0(self):
        """
        Test retrieving features using feature requests with filter
        rectangles set, server version 2.0
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    }
  ],
"serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Locations","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Locations",
  "value": [
    {
      "@id": "endpoint/Locations(1)",
      "id": 1,
      "name": "Location 1",
      "description": "Desc 1",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          1.623373,
          52.132017
        ]
      },
      "properties": {
        "owner": "owner 1"
      },
      "Things@navigationLink": "endpoint/Locations(1)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Locations(1)/HistoricalLocations"
    },
    {
      "@id": "endpoint/Locations(2)",
      "id": 2,
      "name": "Location 2",
      "description": "Desc 2",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          12.623373,
          53.132017
        ]
      },
      "properties": {
        "owner": "owner 2"
      },
      "Things@navigationLink": "endpoint/Locations(2)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Locations(2)/HistoricalLocations"

    }
  ],
  "@nextLink": "endpoint/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Locations",
              "value": [
                {
                  "@id": "endpoint/Locations(3)",
                  "id": 3,
                  "name": "Location 3",
                  "description": "Desc 3",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      3.623373,
                      55.132017
                    ]
                  },
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@navigationLink": "endpoint/Locations(3)/Things",
                  "HistoricalLocations@navigationLink": "endpoint/Locations(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((1 0, 10 0, 10 80, 1 80, 1 0))'))",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Locations",
              "value": [
                          {
                  "@id": "endpoint/Locations(1)",
                  "id": 1,
                  "name": "Location 1",
                  "description": "Desc 1",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      1.623373,
                      52.132017
                    ]
                  },
                  "properties": {
                    "owner": "owner 1"
                  },
                  "Things@navigationLink": "endpoint/Locations(1)/Things",
                  "HistoricalLocations@navigationLink": "endpoint/Locations(1)/HistoricalLocations"
                },
                {
                  "@id": "endpoint/Locations(3)",
                  "id": 3,
                  "name": "Location 3",
                  "description": "Desc 3",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      3.623373,
                      55.132017
                    ]
                  },
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@navigationLink": "endpoint/Locations(3)/Things",
                  "HistoricalLocations@navigationLink": "endpoint/Locations(3)/HistoricalLocations"
                }
              ]
            }""".replace("endpoint", "http://" + endpoint)
                    )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((10 0, 20 0, 20 80, 10 80, 10 0))'))",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
            {
              "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Locations",
              "value": [
                              {
      "@id": "endpoint/Locations(2)",
      "id": 2,
      "name": "Location 2",
      "description": "Desc 2",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          12.623373,
          53.132017
        ]
      },
      "properties": {
        "owner": "owner 2"
      },
      "Things@navigationLink": "endpoint/Locations(2)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Locations(2)/HistoricalLocations"

    }
              ]
            }""".replace("endpoint", "http://" + endpoint)
                )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.featureCount(), 3)
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Location</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"', vl.htmlMetadata())

            # test retrieving subset of features from a filter rect only
            request = QgsFeatureRequest()
            request.setFilterRect(QgsRectangle(1, 0, 10, 80))

            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["1", "3"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1", "Location 3"],
            )
            self.assertEqual([f["description"] for f in features], ["Desc 1", "Desc 3"])
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 3"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (1.6 52.1)", "Point (3.6 55.1)"],
            )

            # test retrieving a different subset with a different extent
            request = QgsFeatureRequest()
            request.setFilterRect(QgsRectangle(10, 0, 20, 80))

            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["2"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(2)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 2"],
            )
            self.assertEqual([f["description"] for f in features], ["Desc 2"])
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 2"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (12.6 53.1)"],
            )

            # a filter rect which covers all features
            request = QgsFeatureRequest()
            request.setFilterRect(QgsRectangle(0, 0, 20, 80))

            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["1", "3", "2"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(3)", "/Locations(2)"],
            )

    def test_extent_limit(self):
        """
        Test a layer with a hardcoded extent limit set at the provider level
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((1 0, 10 0, 10 80, 1 80, 1 0))'))",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":2,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((1 0, 10 0, 10 80, 1 80, 1 0))'))",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Locations(1)",
      "@iot.id": 1,
      "name": "Location 1",
      "description": "Desc 1",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          1.623373,
          52.132017
        ]
      },
      "properties": {
        "owner": "owner 1"
      },
      "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
    },
    {
                  "@iot.selfLink": "endpoint/Locations(3)",
                  "@iot.id": 3,
                  "name": "Location 3",
                  "description": "Desc 3",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      3.623373,
                      55.132017
                    ]
                  },
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@iot.navigationLink": "endpoint/Locations(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(3)/HistoricalLocations"
                }
  ]
}
                """.replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((1 0, 3 0, 3 50, 1 50, 1 0))'))",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
            {
              "value": [
                             {
                  "@iot.selfLink": "endpoint/Locations(1)",
                  "@iot.id": 1,
                  "name": "Location 1",
                  "description": "Desc 1",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      1.623373,
                      52.132017
                    ]
                  },
                  "properties": {
                    "owner": "owner 1"
                  },
                  "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
                }
              ]
            }""".replace("endpoint", "http://" + endpoint)
                )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' bbox='1,0,10,80' type=PointZ pageSize=2 entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.featureCount(), 2)
            # should use the hardcoded extent limit as the initial guess, not global extents
            self.assertEqual(vl.extent(), QgsRectangle(1, 0, 10, 80))
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Location</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"', vl.htmlMetadata())

            # test retrieving a subset of the features from the layer,
            # using a filter rect which only covers a part of the hardcoded
            # provider's extent
            request = QgsFeatureRequest()
            request.setFilterRect(QgsRectangle(1, 0, 3, 50))

            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["1"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1"],
            )
            self.assertEqual([f["description"] for f in features], ["Desc 1"])
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (1.6 52.1)"],
            )

            # test retrieving all features from layer -- the hardcoded
            # provider level extent filter should still apply
            request = QgsFeatureRequest()
            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["1", "3"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1", "Location 3"],
            )
            self.assertEqual([f["description"] for f in features], ["Desc 1", "Desc 3"])
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 3"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (1.6 52.1)", "Point (3.6 55.1)"],
            )

            # should have accurate layer extent now
            self.assertEqual(
                vl.extent(),
                QgsRectangle(
                    1.62337299999999995,
                    52.13201699999999761,
                    3.62337299999999995,
                    55.13201699999999761,
                ),
            )

    def test_subset_string(self):
        """
        Test a layer with a hardcoded user-defined filter string
        at the provider level
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":2,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (name eq 'Location 1')",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":1,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((1 0, 3 0, 3 50, 1 50, 1 0))')) and (name eq 'Location 1')",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Locations(1)",
      "@iot.id": 1,
      "name": "Location 1",
      "description": "Desc 1",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          1.623373,
          52.132017
        ]
      },
      "properties": {
        "owner": "owner 1"
      },
      "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
    }
  ]
}
                """.replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((1 0, 3 0, 3 50, 1 50, 1 0))'))",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
            {
              "value": [
                             {
                  "@iot.selfLink": "endpoint/Locations(1)",
                  "@iot.id": 1,
                  "name": "Location 1",
                  "description": "Desc 1",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      1.623373,
                      52.132017
                    ]
                  },
                  "properties": {
                    "owner": "owner 1"
                  },
                  "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
                }
              ]
            }""".replace("endpoint", "http://" + endpoint)
                )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.featureCount(), 2)

            vl.setSubsetString("name eq 'Location 1'")
            self.assertEqual(vl.subsetString(), "name eq 'Location 1'")
            self.assertEqual(
                vl.source(),
                f" type=PointZ entity='Location' pageSize='2' url='http://{endpoint}' sql=name eq 'Location 1'",
            )
            self.assertEqual(vl.featureCount(), 1)

            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Location</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"', vl.htmlMetadata())

            # test retrieving a subset of features, using a request which
            # must be combined with the layer's subset filter
            request = QgsFeatureRequest()
            request.setFilterRect(QgsRectangle(1, 0, 3, 50))

            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["1"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1"],
            )
            self.assertEqual([f["description"] for f in features], ["Desc 1"])
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (1.6 52.1)"],
            )

            # test retrieving all features from layer, only a subset
            # which matches the layer's subset string should still be
            # returned
            request = QgsFeatureRequest()
            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["1"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1"],
            )
            self.assertEqual([f["description"] for f in features], ["Desc 1"])
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (1.6 52.1)"],
            )

            # should have accurate layer extent now
            self.assertEqual(
                vl.extent(),
                QgsRectangle(
                    1.62337299999999995,
                    52.13201699999999761,
                    1.62337299999999995,
                    52.13201699999999761,
                ),
            )

    def test_feature_limit(self):
        """
        Test a layer with a hardcoded maximum number of features to retrieve
        from the service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (name eq 'Location 1')",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":1,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((1 0, 3 0, 3 50, 1 50, 1 0))'))",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
            {
              "value": [
                             {
                  "@iot.selfLink": "endpoint/Locations(1)",
                  "@iot.id": 1,
                  "name": "Location 1",
                  "description": "Desc 1",
                  "encodingType": "application/geo+json",
                  "location": {
                    "type": "Point",
                    "coordinates": [
                      1.623373,
                      52.132017
                    ]
                  },
                  "properties": {
                    "owner": "owner 1"
                  },
                  "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
                }
              ]
            }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((0 0, 100 0, 100 150, 0 150, 0 0))'))",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Locations(1)",
      "@iot.id": 1,
      "name": "Location 1",
      "description": "Desc 1",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          1.623373,
          52.132017
        ]
      },
      "properties": {
        "owner": "owner 1"
      },
      "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
    },
        {
      "@iot.selfLink": "endpoint/Locations(2)",
      "@iot.id": 2,
      "name": "Location 2",
      "description": "Desc 2",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          81,
          52
        ]
      },
      "properties": {
        "owner": "owner 2"
      },
      "Things@iot.navigationLink": "endpoint/Locations(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(2)/HistoricalLocations"
    }
  ],
  "@iot.nextLink": "endpoint/Locations?$top=2&$skip=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((0 0, 100 0, 100 150, 0 150, 0 0))'))"
}
                """.replace("endpoint", "http://" + endpoint)
                )

            # Note -- top param here should be replaced by "top=1", NOT be the "top=2" parameter from the previous page's iot.nextLink url!
            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=1&$skip=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'Polygon ((0 0, 100 0, 100 150, 0 150, 0 0))'))",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Locations(3)",
      "@iot.id": 3,
      "name": "Location 3",
      "description": "Desc 3",
      "encodingType": "application/geo+json",
      "location": {
        "type": "Point",
        "coordinates": [
          82,
          53
        ]
      },
      "properties": {
        "owner": "owner 3"
      },
      "Things@iot.navigationLink": "endpoint/Locations(3)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(3)/HistoricalLocations"
    }
  ],
  "@iot.nextLink": "endpoint/Locations?$top=1&$skip=3"
}
                """.replace("endpoint", "http://" + endpoint)
                )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 featureLimit=3 entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.featureCount(), 3)

            # test retrieving a subset of the 3 features by using
            # a request with a filter rect only matching one of the features
            request = QgsFeatureRequest()
            request.setFilterRect(QgsRectangle(1, 0, 3, 50))

            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["1"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1"],
            )
            self.assertEqual([f["description"] for f in features], ["Desc 1"])
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (1.6 52.1)"],
            )

            # test retrieving all features from layer using a filter rect
            # which matches all features -- this is actually testing that
            # the provider is correctly constructing a url with the right
            # skip/limit values (if it isn't, then we'll get no features
            # back since the dummy endpoint address used above won't match)
            request = QgsFeatureRequest()
            request.setFilterRect(QgsRectangle(0, 0, 100, 150))
            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(2)", "/Locations(3)"],
            )

            # should have accurate layer extent now
            self.assertEqual(vl.extent(), QgsRectangle(1.62337299999999995, 52, 82, 53))

    def test_historical_location(self):
        """
        Test a layer retrieving 'Historical Location' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "HistoricalLocations",
      "url": "endpoint/HistoricalLocations"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/HistoricalLocations?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/HistoricalLocations?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/HistoricalLocations(1)",
      "@iot.id": 1,
      "time": "2020-03-20T16:35:23.383586Z",
      "Things@iot.navigationLink": "endpoint/HistoricalLocations(1)/Things",
      "Locations@iot.navigationLink": "endpoint/HistoricalLocations(1)/Locations"
    },
    {
      "@iot.selfLink": "endpoint/HistoricalLocations(2)",
      "@iot.id": 2,
      "time": "2021-03-20T16:35:23.383586Z",
      "Things@iot.navigationLink": "endpoint/HistoricalLocations(2)/Things",
      "Locations@iot.navigationLink": "endpoint/HistoricalLocations(2)/Locations"

    }
  ],
  "@iot.nextLink": "endpoint/HistoricalLocations?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/HistoricalLocations?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/HistoricalLocations(3)",
                  "@iot.id": 3,
                  "time": "2022-03-20T16:35:23.383586Z",
                  "Things@iot.navigationLink": "endpoint/HistoricalLocations(3)/Things",
                  "Locations@iot.navigationLink": "endpoint/HistoricalLocations(3)/Locations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='HistoricalLocation'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn(
                "Entity Type</td><td>HistoricalLocation</td>", vl.htmlMetadata()
            )
            self.assertIn(
                f'href="http://{endpoint}/HistoricalLocations"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "time",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-23:] for f in features],
                [
                    "/HistoricalLocations(1)",
                    "/HistoricalLocations(2)",
                    "/HistoricalLocations(3)",
                ],
            )
            self.assertEqual(
                [f["time"] for f in features],
                [
                    QDateTime(
                        QDate(2020, 3, 20), QTime(16, 35, 23, 384), Qt.TimeSpec(1)
                    ),
                    QDateTime(
                        QDate(2021, 3, 20), QTime(16, 35, 23, 384), Qt.TimeSpec(1)
                    ),
                    QDateTime(
                        QDate(2022, 3, 20), QTime(16, 35, 23, 384), Qt.TimeSpec(1)
                    ),
                ],
            )

    def test_historical_location_2_0(self):
        """
        Test a layer retrieving 'Historical Location' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "HistoricalLocations",
      "url": "endpoint/HistoricalLocations"
    }
  ],
  "serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/HistoricalLocations?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#HistoricalLocations","@count":3,"value":[]}"""
                )

            with open(
                sanitize(endpoint, "/HistoricalLocations?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#HistoricalLocations",
  "value": [
    {
      "@id": "endpoint/HistoricalLocations(1)",
      "id": 1,
      "time": "2020-03-20T16:35:23.383586Z",
      "Things@navigationLink": "endpoint/HistoricalLocations(1)/Things",
      "Locations@navigationLink": "endpoint/HistoricalLocations(1)/Locations"
    },
    {
      "@id": "endpoint/HistoricalLocations(2)",
      "id": 2,
      "time": "2021-03-20T16:35:23.383586Z",
      "Things@navigationLink": "endpoint/HistoricalLocations(2)/Things",
      "Locations@navigationLink": "endpoint/HistoricalLocations(2)/Locations"

    }
  ],
  "@nextLink": "endpoint/HistoricalLocations?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/HistoricalLocations?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#HistoricalLocations",
              "value": [
                {
                  "@id": "endpoint/HistoricalLocations(3)",
                  "id": 3,
                  "time": "2022-03-20T16:35:23.383586Z",
                  "Things@navigationLink": "endpoint/HistoricalLocations(3)/Things",
                  "Locations@navigationLink": "endpoint/HistoricalLocations(3)/Locations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='HistoricalLocation'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn(
                "Entity Type</td><td>HistoricalLocation</td>", vl.htmlMetadata()
            )
            self.assertIn(
                f'href="http://{endpoint}/HistoricalLocations"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "time",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-23:] for f in features],
                [
                    "/HistoricalLocations(1)",
                    "/HistoricalLocations(2)",
                    "/HistoricalLocations(3)",
                ],
            )
            self.assertEqual(
                [f["time"] for f in features],
                [
                    QDateTime(
                        QDate(2020, 3, 20), QTime(16, 35, 23, 384), Qt.TimeSpec(1)
                    ),
                    QDateTime(
                        QDate(2021, 3, 20), QTime(16, 35, 23, 384), Qt.TimeSpec(1)
                    ),
                    QDateTime(
                        QDate(2022, 3, 20), QTime(16, 35, 23, 384), Qt.TimeSpec(1)
                    ),
                ],
            )

    def test_datastream(self):
        """
        Test a layer retrieving 'Datastream' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Datastreams",
      "url": "endpoint/Datastreams"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/Datastreams?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/Datastreams?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Datastreams(1)",
      "@iot.id": 1,
      "name": "Datastream 1",
      "description": "Desc 1",
      "unitOfMeasurement": {
        "name": "ug.m-3",
        "symbol": "ug.m-3",
        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
      },
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "phenomenonTime": "2017-12-31T23:00:00Z/2018-01-12T04:00:00Z",
      "resultTime": "2017-12-31T23:30:00Z/2017-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 1"
      },
      "Things@iot.navigationLink": "endpoint/Datastreams(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/Datastreams(2)",
      "@iot.id": 2,
      "name": "Datastream 2",
      "description": "Desc 2",
      "unitOfMeasurement": {
        "name": "ug.m-3",
        "symbol": "ug.m-3",
        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
      },
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "phenomenonTime": "2018-12-31T23:00:00Z/2019-01-12T04:00:00Z",
      "resultTime": "2018-12-31T23:30:00Z/2018-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 2"
      },
      "Things@iot.navigationLink": "endpoint/Datastreams(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/Datastreams?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/Datastreams?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/Datastreams(3)",
                  "@iot.id": 3,
                  "name": "Datastream 3",
                  "description": "Desc 3",
                  "unitOfMeasurement": {
                    "name": "ug.m-3",
                    "symbol": "ug.m-3",
                    "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
                  },
                  "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                  "phenomenonTime": "2020-12-31T23:00:00Z/2021-01-12T04:00:00Z",
                  "resultTime": "2020-12-31T23:30:00Z/2020-12-31T23:31:00Z",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@iot.navigationLink": "endpoint/Datastreams(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=NoGeometry entity='Datastream'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn("Entity Type</td><td>Datastream</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Datastreams"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "unitOfMeasurement",
                    "observationType",
                    "properties",
                    "phenomenonTimeStart",
                    "phenomenonTimeEnd",
                    "resultTimeStart",
                    "resultTimeEnd",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-15:] for f in features],
                ["/Datastreams(1)", "/Datastreams(2)", "/Datastreams(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Datastream 1", "Datastream 2", "Datastream 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["unitOfMeasurement"] for f in features],
                [
                    {
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    },
                    {
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    },
                    {
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    },
                ],
            )
            self.assertEqual(
                [f["observationType"] for f in features],
                [
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2018, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2019, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2021, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

    def test_datastream_2_0(self):
        """
        Test a layer retrieving 'Datastream' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Datastreams",
      "url": "endpoint/Datastreams"
    }
  ],
  "serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/Datastreams?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Datastreams","@count":3,"value":[]}"""
                )

            with open(
                sanitize(endpoint, "/Datastreams?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Datastreams",
  "value": [
    {
      "@id": "endpoint/Datastreams(1)",
      "id": 1,
      "name": "Datastream 1",
      "description": "Desc 1",
      "unitOfMeasurement": {
        "name": "ug.m-3",
        "symbol": "ug.m-3",
        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
      },
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "phenomenonTime": "2017-12-31T23:00:00Z/2018-01-12T04:00:00Z",
      "resultTime": "2017-12-31T23:30:00Z/2017-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 1"
      },
      "Things@navigationLink": "endpoint/Datastreams(1)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Datastreams(1)/HistoricalLocations"
    },
    {
      "@id": "endpoint/Datastreams(2)",
      "id": 2,
      "name": "Datastream 2",
      "description": "Desc 2",
      "unitOfMeasurement": {
        "name": "ug.m-3",
        "symbol": "ug.m-3",
        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
      },
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "phenomenonTime": "2018-12-31T23:00:00Z/2019-01-12T04:00:00Z",
      "resultTime": "2018-12-31T23:30:00Z/2018-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 2"
      },
      "Things@navigationLink": "endpoint/Datastreams(2)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Datastreams(2)/HistoricalLocations"

    }
  ],
  "@nextLink": "endpoint/Datastreams?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/Datastreams?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Datastreams",
              "value": [
                {
                  "@id": "endpoint/Datastreams(3)",
                  "id": 3,
                  "name": "Datastream 3",
                  "description": "Desc 3",
                  "unitOfMeasurement": {
                    "name": "ug.m-3",
                    "symbol": "ug.m-3",
                    "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
                  },
                  "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                  "phenomenonTime": "2020-12-31T23:00:00Z/2021-01-12T04:00:00Z",
                  "resultTime": "2020-12-31T23:30:00Z/2020-12-31T23:31:00Z",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@navigationLink": "endpoint/Datastreams(3)/Things",
                  "HistoricalLocations@navigationLink": "endpoint/Datastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=NoGeometry entity='Datastream'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn("Entity Type</td><td>Datastream</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Datastreams"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "unitOfMeasurement",
                    "observationType",
                    "properties",
                    "phenomenonTimeStart",
                    "phenomenonTimeEnd",
                    "resultTimeStart",
                    "resultTimeEnd",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-15:] for f in features],
                ["/Datastreams(1)", "/Datastreams(2)", "/Datastreams(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Datastream 1", "Datastream 2", "Datastream 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["unitOfMeasurement"] for f in features],
                [
                    {
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    },
                    {
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    },
                    {
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    },
                ],
            )
            self.assertEqual(
                [f["observationType"] for f in features],
                [
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2018, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2019, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2021, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

    def test_sensor(self):
        """
        Test a layer retrieving 'Sensor' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Sensors",
      "url": "endpoint/Sensors"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/Sensors?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/Sensors?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Sensors(1)",
      "@iot.id": 1,
      "name": "Datastream 1",
      "description": "Desc 1",
      "encodingType": "application/pdf",
      "metadata": "http://www.a.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
      "properties": {
        "owner": "owner 1"
      },
      "Things@iot.navigationLink": "endpoint/Datastreams(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/Sensors(2)",
      "@iot.id": 2,
      "name": "Datastream 2",
      "description": "Desc 2",
      "encodingType": "application/pdf",
      "metadata": "http://www.b.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
      "properties": {
        "owner": "owner 2"
      },
      "Things@iot.navigationLink": "endpoint/Datastreams(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/Sensors?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/Sensors?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/Sensors(3)",
                  "@iot.id": 3,
                  "name": "Datastream 3",
                  "description": "Desc 3",
                  "encodingType": "application/pdf",
                  "metadata": "http://www.c.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@iot.navigationLink": "endpoint/Datastreams(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=PointZ entity='Sensor'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn("Entity Type</td><td>Sensor</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Sensors"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "metadata",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-11:] for f in features],
                ["/Sensors(1)", "/Sensors(2)", "/Sensors(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Datastream 1", "Datastream 2", "Datastream 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["metadata"] for f in features],
                [
                    "http://www.a.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
                    "http://www.b.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
                    "http://www.c.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

    def test_sensor_2_0(self):
        """
        Test a layer retrieving 'Sensor' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Sensors",
      "url": "endpoint/Sensors"
    }
  ],
  "serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/Sensors?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Sensors","@count":3,"value":[]}"""
                )

            with open(
                sanitize(endpoint, "/Sensors?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Sensors",
  "value": [
    {
      "@id": "endpoint/Sensors(1)",
      "id": 1,
      "name": "Datastream 1",
      "description": "Desc 1",
      "encodingType": "application/pdf",
      "metadata": "http://www.a.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
      "properties": {
        "owner": "owner 1"
      },
      "Things@navigationLink": "endpoint/Datastreams(1)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Datastreams(1)/HistoricalLocations"
    },
    {
      "@id": "endpoint/Sensors(2)",
      "id": 2,
      "name": "Datastream 2",
      "description": "Desc 2",
      "encodingType": "application/pdf",
      "metadata": "http://www.b.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
      "properties": {
        "owner": "owner 2"
      },
      "Things@navigationLink": "endpoint/Datastreams(2)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Datastreams(2)/HistoricalLocations"

    }
  ],
  "@nextLink": "endpoint/Sensors?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/Sensors?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
            "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Sensors",
              "value": [
                {
                  "@id": "endpoint/Sensors(3)",
                  "id": 3,
                  "name": "Datastream 3",
                  "description": "Desc 3",
                  "encodingType": "application/pdf",
                  "metadata": "http://www.c.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@navigationLink": "endpoint/Datastreams(3)/Things",
                  "HistoricalLocations@navigationLink": "endpoint/Datastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=PointZ entity='Sensor'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn("Entity Type</td><td>Sensor</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Sensors"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "metadata",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-11:] for f in features],
                ["/Sensors(1)", "/Sensors(2)", "/Sensors(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Datastream 1", "Datastream 2", "Datastream 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["metadata"] for f in features],
                [
                    "http://www.a.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
                    "http://www.b.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
                    "http://www.c.at/fileadmin/site/umweltthemen/luft/PM_Aequivalenz_Dokumentation.pdf",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

    def test_observed_property(self):
        """
        Test a layer retrieving 'Observed Property' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "ObservedProperties",
      "url": "endpoint/ObservedProperties"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/ObservedProperties?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/ObservedProperties?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/ObservedProperties(1)",
      "@iot.id": 1,
      "name": "Datastream 1",
      "description": "Desc 1",
      "definition": "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/1",
      "properties": {
        "owner": "owner 1"
      },
      "Things@iot.navigationLink": "endpoint/Datastreams(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/ObservedProperties(2)",
      "@iot.id": 2,
      "name": "Datastream 2",
      "definition": "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/2",
      "description": "Desc 2",
      "properties": {
        "owner": "owner 2"
      },
      "Things@iot.navigationLink": "endpoint/Datastreams(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/ObservedProperties?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/ObservedProperties?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/ObservedProperties(3)",
                  "@iot.id": 3,
                  "name": "Datastream 3",
                  "description": "Desc 3",
                  "definition": "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/3",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@iot.navigationLink": "endpoint/Datastreams(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='ObservedProperty'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn(
                "Entity Type</td><td>ObservedProperty</td>", vl.htmlMetadata()
            )
            self.assertIn(
                f'href="http://{endpoint}/ObservedProperties"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "definition",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-22:] for f in features],
                [
                    "/ObservedProperties(1)",
                    "/ObservedProperties(2)",
                    "/ObservedProperties(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Datastream 1", "Datastream 2", "Datastream 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["definition"] for f in features],
                [
                    "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/1",
                    "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/2",
                    "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/3",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

    def test_observed_property_2_0(self):
        """
        Test a layer retrieving 'Observed Property' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "ObservedProperties",
      "url": "endpoint/ObservedProperties"
    }
  ],
  "serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/ObservedProperties?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ObservedProperties","@count":3,"value":[]}"""
                )

            with open(
                sanitize(endpoint, "/ObservedProperties?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ObservedProperties",
  "value": [
    {
      "@id": "endpoint/ObservedProperties(1)",
      "id": 1,
      "name": "Datastream 1",
      "description": "Desc 1",
      "definition": "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/1",
      "properties": {
        "owner": "owner 1"
      },
      "Things@navigationLink": "endpoint/Datastreams(1)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Datastreams(1)/HistoricalLocations"
    },
    {
      "@id": "endpoint/ObservedProperties(2)",
      "id": 2,
      "name": "Datastream 2",
      "definition": "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/2",
      "description": "Desc 2",
      "properties": {
        "owner": "owner 2"
      },
      "Things@navigationLink": "endpoint/Datastreams(2)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Datastreams(2)/HistoricalLocations"

    }
  ],
  "@nextLink": "endpoint/ObservedProperties?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/ObservedProperties?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
            "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ObservedProperties",
              "value": [
                {
                  "@id": "endpoint/ObservedProperties(3)",
                  "id": 3,
                  "name": "Datastream 3",
                  "description": "Desc 3",
                  "definition": "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/3",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@navigationLink": "endpoint/Datastreams(3)/Things",
                  "HistoricalLocations@navigationLink": "endpoint/Datastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ pageSize=2 entity='ObservedProperty'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn(
                "Entity Type</td><td>ObservedProperty</td>", vl.htmlMetadata()
            )
            self.assertIn(
                f'href="http://{endpoint}/ObservedProperties"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "definition",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-22:] for f in features],
                [
                    "/ObservedProperties(1)",
                    "/ObservedProperties(2)",
                    "/ObservedProperties(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Datastream 1", "Datastream 2", "Datastream 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["definition"] for f in features],
                [
                    "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/1",
                    "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/2",
                    "http://dd.eionet.europa.eu/vocabulary/aq/pollutant/3",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

    def test_observation(self):
        """
        Test a layer retrieving 'Observation' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Observations",
      "url": "endpoint/Observations"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/Observations?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/Observations?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Observations(1)",
      "@iot.id": 1,
      "phenomenonTime": "2017-12-31T23:00:00Z/2018-01-01T00:00:00Z",
      "result": 12.5962142944,
      "resultTime": "2017-12-31T23:00:30Z",
      "resultQuality": "good",
      "validTime": "2017-12-31T23:00:00Z/2018-12-31T00:00:00Z",
      "parameters":{
      "a":1,
      "b":2
      },
      "Things@iot.navigationLink": "endpoint/Datastreams(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/Observations(2)",
      "@iot.id": 2,
      "phenomenonTime": "2018-01-01T00:00:00Z/2018-01-01T01:00:00Z",
      "result": 7.7946872711,
      "resultTime": "2018-01-01T00:30:00Z",
      "validTime": "2018-12-31T23:00:00Z/2019-12-31T00:00:00Z",
      "resultQuality": ["good", "fair"],
      "parameters":{
      "a":3,
      "b":4
      },
      "Things@iot.navigationLink": "endpoint/Datastreams(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/Observations?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/Observations?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/Observations(3)",
                  "@iot.id": 3,
                  "phenomenonTime": "2018-01-01T02:00:00Z/2018-01-01T02:30:00Z",
                  "result": 4.1779522896,
                  "resultTime": "2018-01-01T02:30:00Z",
                  "validTime": "2019-12-31T23:00:00Z/2020-12-31T00:00:00Z",
                  "Things@iot.navigationLink": "endpoint/Datastreams(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=PointZ entity='Observation'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn("Entity Type</td><td>Observation</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Observations"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "phenomenonTimeStart",
                    "phenomenonTimeEnd",
                    "result",
                    "resultTime",
                    "resultQuality",
                    "validTimeStart",
                    "validTimeEnd",
                    "parameters",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.String,
                    QVariant.DateTime,
                    QVariant.StringList,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-16:] for f in features],
                ["/Observations(1)", "/Observations(2)", "/Observations(3)"],
            )
            self.assertEqual(
                [f["phenomenonTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(0, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(2, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2018, 1, 1), QTime(0, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(1, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(2, 30, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            # TODO -- these should be doubles
            self.assertEqual(
                [f["result"] for f in features],
                ["12.5962", "7.79469", "4.17795"],
            )
            self.assertEqual(
                [f["resultTime"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 30, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(0, 30, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(2, 30, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultQuality"] for f in features],
                [["good"], ["good", "fair"], None],
            )
            self.assertEqual(
                [f["validTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2019, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["validTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2018, 12, 31), QTime(0, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2019, 12, 31), QTime(0, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(0, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["parameters"] for f in features],
                [{"a": 1, "b": 2}, {"a": 3, "b": 4}, None],
            )

    def test_observation_2_0(self):
        """
        Test a layer retrieving 'Observation' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Observations",
      "url": "endpoint/Observations"
    }
  ],
"serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/Observations?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Observations","@count":3,"value":[]}"""
                )

            with open(
                sanitize(endpoint, "/Observations?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Observations",
  "value": [
    {
      "@id": "endpoint/Observations(1)",
      "id": 1,
      "phenomenonTime": "2017-12-31T23:00:00Z/2018-01-01T00:00:00Z",
      "result": 12.5962142944,
      "resultTime": "2017-12-31T23:00:30Z",
      "resultQuality": "good",
      "validTime": "2017-12-31T23:00:00Z/2018-12-31T00:00:00Z",
      "parameters":{
      "a":1,
      "b":2
      },
      "Things@navigationLink": "endpoint/Datastreams(1)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Datastreams(1)/HistoricalLocations"
    },
    {
      "@id": "endpoint/Observations(2)",
      "id": 2,
      "phenomenonTime": "2018-01-01T00:00:00Z/2018-01-01T01:00:00Z",
      "result": 7.7946872711,
      "resultTime": "2018-01-01T00:30:00Z",
      "validTime": "2018-12-31T23:00:00Z/2019-12-31T00:00:00Z",
      "resultQuality": ["good", "fair"],
      "parameters":{
      "a":3,
      "b":4
      },
      "Things@navigationLink": "endpoint/Datastreams(2)/Things",
      "HistoricalLocations@navigationLink": "endpoint/Datastreams(2)/HistoricalLocations"

    }
  ],
  "@nextLink": "endpoint/Observations?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/Observations?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
            "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Observations",
              "value": [
                {
                  "@id": "endpoint/Observations(3)",
                  "id": 3,
                  "phenomenonTime": "2018-01-01T02:00:00Z/2018-01-01T02:30:00Z",
                  "result": 4.1779522896,
                  "resultTime": "2018-01-01T02:30:00Z",
                  "validTime": "2019-12-31T23:00:00Z/2020-12-31T00:00:00Z",
                  "Things@navigationLink": "endpoint/Datastreams(3)/Things",
                  "HistoricalLocations@navigationLink": "endpoint/Datastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=PointZ entity='Observation'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)
            self.assertFalse(vl.crs().isValid())
            self.assertIn("Entity Type</td><td>Observation</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Observations"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "phenomenonTimeStart",
                    "phenomenonTimeEnd",
                    "result",
                    "resultTime",
                    "resultQuality",
                    "validTimeStart",
                    "validTimeEnd",
                    "parameters",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.String,
                    QVariant.DateTime,
                    QVariant.StringList,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-16:] for f in features],
                ["/Observations(1)", "/Observations(2)", "/Observations(3)"],
            )
            self.assertEqual(
                [f["phenomenonTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(0, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(2, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2018, 1, 1), QTime(0, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(1, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(2, 30, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            # TODO -- these should be doubles
            self.assertEqual(
                [f["result"] for f in features],
                ["12.5962", "7.79469", "4.17795"],
            )
            self.assertEqual(
                [f["resultTime"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 30, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(0, 30, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 1, 1), QTime(2, 30, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultQuality"] for f in features],
                [["good"], ["good", "fair"], None],
            )
            self.assertEqual(
                [f["validTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2019, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["validTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2018, 12, 31), QTime(0, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2019, 12, 31), QTime(0, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(0, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["parameters"] for f in features],
                [{"a": 1, "b": 2}, {"a": 3, "b": 4}, None],
            )

    def test_feature_of_interest(self):
        """
        Test a layer retrieving 'Features of Interest' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "FeaturesOfInterest",
      "url": "endpoint/FeaturesOfInterest"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/FeaturesOfInterest?$top=0&$count=true&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/FeaturesOfInterest?$top=2&$count=false&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/FeaturesOfInterest(1)",
      "@iot.id": 1,
      "description": "Air quality sample SAM.09.LAA.822.7.1",
      "encodingType": "application/geo+json",
      "feature": {
        "type": "Point",
        "coordinates": [
          16.3929202777778,
          48.1610363888889
        ]
      },
      "name": "SAM.09.LAA.822.7.1",
      "properties": {
        "localId": "SAM.09.LAA.822.7.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
      "Things@iot.navigationLink": "endpoint/Datastreams(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/FeaturesOfInterest(2)",
      "@iot.id": 2,
      "encodingType": "application/geo+json",
      "feature": {
        "type": "Point",
        "coordinates": [
          16.5256138888889,
          48.1620694444444
        ]
      },
      "name": "SAM.09.LOB.823.7.1",
      "properties": {
        "localId": "SAM.09.LOB.823.7.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
      "Things@iot.navigationLink": "endpoint/Datastreams(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/FeaturesOfInterest?$top=2&$skip=2&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/FeaturesOfInterest?$top=2&$skip=2&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/FeaturesOfInterest(3)",
                  "@iot.id": 3,
"description": "Air quality sample SAM.09.LOB.824.1.1",
      "encodingType": "application/geo+json",
      "feature": {
        "type": "Point",
        "coordinates": [
          16.5256138888889,
          48.1620694444444
        ]
      },
      "name": "SAM.09.LOB.824.1.1",
      "properties": {
        "localId": "SAM.09.LOB.824.1.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
                        "Things@iot.navigationLink": "endpoint/Datastreams(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Datastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=PointZ entity='FeatureOfInterest'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.featureCount(), 3)
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn(
                "Entity Type</td><td>FeatureOfInterest</td>", vl.htmlMetadata()
            )
            self.assertIn(
                f'href="http://{endpoint}/FeaturesOfInterest"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-22:] for f in features],
                [
                    "/FeaturesOfInterest(1)",
                    "/FeaturesOfInterest(2)",
                    "/FeaturesOfInterest(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["SAM.09.LAA.822.7.1", "SAM.09.LOB.823.7.1", "SAM.09.LOB.824.1.1"],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "Air quality sample SAM.09.LAA.822.7.1",
                    None,
                    "Air quality sample SAM.09.LOB.824.1.1",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (16.4 48.2)", "Point (16.5 48.2)", "Point (16.5 48.2)"],
            )

    def test_features_2_0(self):
        """
        Test a layer retrieving 'Features' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Features",
      "url": "endpoint/Features"
    }
  ],
"serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Features?$top=0&$count=true&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Features","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/Features?$top=2&$count=false&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Features",
  "value": [
    {
      "@id": "endpoint/Features(1)",
      "id": 1,
      "description": "Central Basin Testing Site",
      "encodingType": "application/geo+json",
      "feature": {
        "type": "Point",
        "coordinates": [
          16.3929202777778,
          48.1610363888889
        ]
      },
      "name": "Lake Burley Griffin",
      "properties": {
        "localId": "SAM.09.LAA.822.7.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
      "FeatureTypes@navigationLink": "endpoint/Features(1)/FeatureTypes",
      "Observations@navigationLink": "endpoint/Features(1)/Observations"
    },
    {
      "@id": "endpoint/Features(2)",
      "id": 2,
      "encodingType": "application/geo+json",
      "feature": {
        "type": "Point",
        "coordinates": [
          16.5256138888889,
          48.1620694444444
        ]
      },
      "name": "Lake Burley Griffin - West Basin",
      "properties": {
        "localId": "SAM.09.LOB.823.7.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
      "FeatureTypes@navigationLink": "endpoint/Features(2)/FeatureTypes",
      "Observations@navigationLink": "endpoint/Features(2)/Observations"
    }
  ],
  "@nextLink": "endpoint/Features?$top=2&$skip=2&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Features?$top=2&$skip=2&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
            "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Features",
              "value": [
                {
                  "@id": "endpoint/Features(3)",
                  "id": 3,
"description": "Air quality sample SAM.09.LOB.824.1.1",
      "encodingType": "application/geo+json",
      "feature": {
        "type": "Point",
        "coordinates": [
          16.5256138888889,
          48.1620694444444
        ]
      },
      "name": "Molonglo River Reach",
      "properties": {
        "localId": "SAM.09.LOB.824.1.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
       "FeatureTypes@navigationLink": "endpoint/Features(3)/FeatureTypes",
      "Observations@navigationLink": "endpoint/Features(3)/Observations"
                     }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=PointZ entity='Features'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.featureCount(), 3)
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Feature</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Features"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-12:] for f in features],
                [
                    "/Features(1)",
                    "/Features(2)",
                    "/Features(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                [
                    "Lake Burley Griffin",
                    "Lake Burley Griffin - West Basin",
                    "Molonglo River Reach",
                ],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "Central Basin Testing Site",
                    None,
                    "Air quality sample SAM.09.LOB.824.1.1",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (16.4 48.2)", "Point (16.5 48.2)", "Point (16.5 48.2)"],
            )

    def test_featuretypes_2_0(self):
        """
        Test a layer retrieving 'FeatureTypes' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "FeatureTypes",
      "url": "endpoint/FeatureTypes"
    }
  ],
"serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/FeatureTypes?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#FeatureTypes","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/FeatureTypes?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#FeatureTypes",
  "value": [
    {
      "@id": "endpoint/FeatureTypes(1)",
      "id": 1,
      "description": "A physical volume of water extracted for testing, typically stored in a vial or bottle.",
      "name": "Physical Water Sample",
      "properties": {
        "localId": "SAM.09.LAA.822.7.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
      "Features@navigationLink": "endpoint/FeatureTypes(1)/Features"
    },
    {
      "@id": "endpoint/FeatureTypes(2)",
      "id": 2,
      "name": "Point Geometry",
      "properties": {
        "localId": "SAM.09.LOB.823.7.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
      "Features@navigationLink": "endpoint/FeatureTypes(2)/Features"
    }
  ],
  "@nextLink": "endpoint/FeatureTypes?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/FeatureTypes?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
            "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#FeatureTypes",
              "value": [
                {
                  "@id": "endpoint/FeatureTypes(3)",
                  "id": 3,
"description": "Features that are represented by a single spatial coordinate (Point).",
      "name": "Point Geometry 2",
      "properties": {
        "localId": "SAM.09.LOB.824.1.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
      "Features@navigationLink": "endpoint/FeatureTypes(3)/Features"
                     }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='FeatureTypes'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn("Entity Type</td><td>FeatureType</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/FeatureTypes"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-16:] for f in features],
                [
                    "/FeatureTypes(1)",
                    "/FeatureTypes(2)",
                    "/FeatureTypes(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Physical Water Sample", "Point Geometry", "Point Geometry 2"],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "A physical volume of water extracted for testing, typically stored in a vial or bottle.",
                    None,
                    "Features that are represented by a single spatial coordinate (Point).",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

    def test_multidatastream_no_geometry(self):
        """
        Test a layer retrieving 'MultiDatastream' entities from a service without geometry
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "MultiDatastreams",
      "url": "endpoint/MultiDatastreams"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(endpoint, "/MultiDatastreams?$top=0&$count=true"),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/MultiDatastreams?$top=2&$count=false"),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/MultiDatastreams(1)",
      "@iot.id": 1,
      "name": "MultiDatastream 1",
      "description": "Desc 1",
      "unitOfMeasurements": [
          {
            "name": "ug.m-3",
            "symbol": "ug.m-3",
            "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
          }
      ],
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
      "phenomenonTime": "2017-12-31T23:00:00Z/2018-01-12T04:00:00Z",
      "resultTime": "2017-12-31T23:30:00Z/2017-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 1"
      },
      "Things@iot.navigationLink": "endpoint/MultiDatastreams(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/MultiDatastreams(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/MultiDatastreams(2)",
      "@iot.id": 2,
      "name": "MultiDatastream 2",
      "description": "Desc 2",
      "unitOfMeasurements": [
      {
        "name": "ug.m-3",
        "symbol": "ug.m-3",
        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
      }],
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
      "phenomenonTime": "2018-12-31T23:00:00Z/2019-01-12T04:00:00Z",
      "resultTime": "2018-12-31T23:30:00Z/2018-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 2"
      },
      "Things@iot.navigationLink": "endpoint/MultiDatastreams(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/MultiDatastreams(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/MultiDatastreams?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(endpoint, "/MultiDatastreams?$top=2&$skip=2"),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/MultiDatastreams(3)",
                  "@iot.id": 3,
                  "name": "MultiDatastream 3",
                  "description": "Desc 3",
                  "unitOfMeasurements": [{
                    "name": "ug.m-3",
                    "symbol": "ug.m-3",
                    "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
                  }],
                  "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                  "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
                  "phenomenonTime": "2020-12-31T23:00:00Z/2021-01-12T04:00:00Z",
                  "resultTime": "2020-12-31T23:30:00Z/2020-12-31T23:31:00Z",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things@iot.navigationLink": "endpoint/MultiDatastreams(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/MultiDatastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='MultiDatastream'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertFalse(vl.crs().isValid())
            self.assertIn("Entity Type</td><td>MultiDatastream</td>", vl.htmlMetadata())
            self.assertIn(
                f'href="http://{endpoint}/MultiDatastreams"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "unitOfMeasurements",
                    "observationType",
                    "multiObservationDataTypes",
                    "properties",
                    "phenomenonTimeStart",
                    "phenomenonTimeEnd",
                    "resultTimeStart",
                    "resultTimeEnd",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                    QVariant.StringList,
                    QVariant.Map,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-20:] for f in features],
                [
                    "/MultiDatastreams(1)",
                    "/MultiDatastreams(2)",
                    "/MultiDatastreams(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["MultiDatastream 1", "MultiDatastream 2", "MultiDatastream 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["unitOfMeasurements"] for f in features],
                [
                    [
                        {
                            "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                            "name": "ug.m-3",
                            "symbol": "ug.m-3",
                        }
                    ],
                    [
                        {
                            "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                            "name": "ug.m-3",
                            "symbol": "ug.m-3",
                        }
                    ],
                    [
                        {
                            "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                            "name": "ug.m-3",
                            "symbol": "ug.m-3",
                        }
                    ],
                ],
            )
            self.assertEqual(
                [f["observationType"] for f in features],
                [
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                ],
            )
            self.assertEqual(
                [f["multiObservationDataTypes"] for f in features],
                [
                    [
                        "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"
                    ],
                    [
                        "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"
                    ],
                    [
                        "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"
                    ],
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2018, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2019, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2021, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )

    def test_multidatastream_polygons(self):
        """
        Test a layer retrieving 'MultiDatastream' entities from a service using polygons
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "MultiDatastreams",
      "url": "endpoint/MultiDatastreams"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/MultiDatastreams?$top=0&$count=true&$filter=observedArea/type eq 'Polygon' or observedArea/geometry/type eq 'Polygon'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/MultiDatastreams?$top=2&$count=false&$filter=observedArea/type eq 'Polygon' or observedArea/geometry/type eq 'Polygon'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/MultiDatastreams(1)",
      "@iot.id": 1,
      "name": "MultiDatastream 1",
      "description": "Desc 1",
      "unitOfMeasurements": [
          {
            "name": "ug.m-3",
            "symbol": "ug.m-3",
            "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
          }
      ],
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
      "phenomenonTime": "2017-12-31T23:00:00Z/2018-01-12T04:00:00Z",
      "resultTime": "2017-12-31T23:30:00Z/2017-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 1"
      },
      "observedArea": {
            "type": "Polygon",
            "coordinates": [
              [
                [100, 0], [101, 0], [101, 1], [100, 1], [100, 0]
              ]
            ]
          },
      "Things@iot.navigationLink": "endpoint/MultiDatastreams(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/MultiDatastreams(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/MultiDatastreams(2)",
      "@iot.id": 2,
      "name": "MultiDatastream 2",
      "description": "Desc 2",
      "unitOfMeasurements": [
      {
        "name": "ug.m-3",
        "symbol": "ug.m-3",
        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
      }],
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
      "phenomenonTime": "2018-12-31T23:00:00Z/2019-01-12T04:00:00Z",
      "resultTime": "2018-12-31T23:30:00Z/2018-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 2"
      },
            "observedArea": {
            "type": "Polygon",
            "coordinates": [
              [
                [102, 0], [103, 0], [103, 1], [102, 1], [102, 0]
              ]
            ]
          },
      "Things@iot.navigationLink": "endpoint/MultiDatastreams(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/MultiDatastreams(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/MultiDatastreams?$top=2&$skip=2&$filter=observedArea/type eq 'Polygon' or observedArea/geometry/type eq 'Polygon'"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/MultiDatastreams?$top=2&$skip=2&$filter=observedArea/type eq 'Polygon' or observedArea/geometry/type eq 'Polygon'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/MultiDatastreams(3)",
                  "@iot.id": 3,
                  "name": "MultiDatastream 3",
                  "description": "Desc 3",
                  "unitOfMeasurements": [{
                    "name": "ug.m-3",
                    "symbol": "ug.m-3",
                    "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
                  }],
                  "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                  "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
                  "phenomenonTime": "2020-12-31T23:00:00Z/2021-01-12T04:00:00Z",
                  "resultTime": "2020-12-31T23:30:00Z/2020-12-31T23:31:00Z",
                  "properties": {
                    "owner": "owner 3"
                  },
                        "observedArea": {
            "type": "Polygon",
            "coordinates": [
              [
                [103, 0], [104, 0], [104, 1], [103, 1], [103, 0]
              ]
            ]
          },
                  "Things@iot.navigationLink": "endpoint/MultiDatastreams(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/MultiDatastreams(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=MultiPolygonZ entity='MultiDatastream'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.MultiPolygonZ)
            self.assertEqual(vl.featureCount(), 3)
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>MultiDatastream</td>", vl.htmlMetadata())
            self.assertIn(
                f'href="http://{endpoint}/MultiDatastreams"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "unitOfMeasurements",
                    "observationType",
                    "multiObservationDataTypes",
                    "properties",
                    "phenomenonTimeStart",
                    "phenomenonTimeEnd",
                    "resultTimeStart",
                    "resultTimeEnd",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                    QVariant.StringList,
                    QVariant.Map,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-20:] for f in features],
                [
                    "/MultiDatastreams(1)",
                    "/MultiDatastreams(2)",
                    "/MultiDatastreams(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["MultiDatastream 1", "MultiDatastream 2", "MultiDatastream 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["unitOfMeasurements"] for f in features],
                [
                    [
                        {
                            "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                            "name": "ug.m-3",
                            "symbol": "ug.m-3",
                        }
                    ],
                    [
                        {
                            "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                            "name": "ug.m-3",
                            "symbol": "ug.m-3",
                        }
                    ],
                    [
                        {
                            "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                            "name": "ug.m-3",
                            "symbol": "ug.m-3",
                        }
                    ],
                ],
            )
            self.assertEqual(
                [f["observationType"] for f in features],
                [
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                    "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                ],
            )
            self.assertEqual(
                [f["multiObservationDataTypes"] for f in features],
                [
                    [
                        "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"
                    ],
                    [
                        "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"
                    ],
                    [
                        "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"
                    ],
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2018, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2019, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2021, 1, 12), QTime(4, 0, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 30, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 31, 0, 0), Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )
            self.assertEqual(
                [f.geometry().asWkt() for f in features],
                [
                    "Polygon ((100 0, 101 0, 101 1, 100 1, 100 0))",
                    "Polygon ((102 0, 103 0, 103 1, 102 1, 102 0))",
                    "Polygon ((103 0, 104 0, 104 1, 103 1, 103 0))",
                ],
            )

    def test_deployments_2_0(self):
        """
        Test a layer retrieving 'Deployments' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Deployments",
      "url": "endpoint/Deployments"
    }
  ],
"serverSettings": {
  "conformance": [
  "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
  ]
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Deployments?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Deployments","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/Deployments?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Deployments",
  "value": [
    {
      "@id": "endpoint/Deployments(1)",
      "id": 1,
      "description": "Mounting the digital pH probe to the primary water quality buoy for the summer monitoring campaign.",
      "name": "Summer 2026 Buoy Deployment",
      "properties": {
        "localId": "SAM.09.LAA.822.7.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
      "time": {
        "start": "2026-05-01T00:00:00Z",
        "end": "2026-09-01T00:00:00Z"
      },
      "Datastreams@navigationLink": "endpoint/Deployments(1)/Datastreams"
    },
    {
      "@id": "endpoint/Deployments(2)",
      "id": 2,
      "name": "Summer 2025 Buoy Deployment",
      "properties": {
        "localId": "SAM.09.LOB.823.7.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
            "time": {
        "start": "2025-05-01T00:00:00Z",
        "end": "2025-09-01T00:00:00Z"
      },
      "Datastreams@navigationLink": "endpoint/Deployments(2)/Datastreams"
    }
  ],
  "@nextLink": "endpoint/Deployments?$top=2&$skip=2"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Deployments?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
            "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Deployments",
              "value": [
                {
                  "@id": "endpoint/Deployments(3)",
                  "id": 3,
"description": "2024 summer period.",
      "name": "Summer 2024 Buoy Deployment",
      "properties": {
        "localId": "SAM.09.LOB.824.1.1",
        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
        "namespace": "AT.0008.20.AQ",
        "owner": "http://luft.umweltbundesamt.at"
      },
            "time": {
        "start": "2024-05-01T00:00:00Z",
        "end": "2024-09-01T00:00:00Z"
      },
      "Datastreams@navigationLink": "endpoint/Deployments(3)/Datastreams"
                     }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='Deployments'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn("Entity Type</td><td>Deployment</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Deployments"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                    "timeStart",
                    "timeEnd",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.DateTime,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-15:] for f in features],
                [
                    "/Deployments(1)",
                    "/Deployments(2)",
                    "/Deployments(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                [
                    "Summer 2026 Buoy Deployment",
                    "Summer 2025 Buoy Deployment",
                    "Summer 2024 Buoy Deployment",
                ],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "Mounting the digital pH probe to the primary water quality buoy for the summer monitoring campaign.",
                    None,
                    "2024 summer period.",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

    def test_observing_procedures_2_0(self):
        """
        Test a layer retrieving 'ObservingProcedures' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": [
        {
          "name": "ObservingProcedures",
          "url": "endpoint/ObservingProcedures"
        }
      ],
    "serverSettings": {
      "conformance": [
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
      ]
      }
    }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/ObservingProcedures?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ObservingProcedures","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/ObservingProcedures?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
    {
      "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ObservingProcedures",
      "value": [
        {
          "@id": "endpoint/ObservingProcedures(1)",
          "id": 1,
          "description": "Electrometric measurement of pH in drinking, surface, and saline waters.",
          "name": "Standard EPA Method 150.1",
          "definition": "https://www.epa.gov/sites/default/files/2015-08/documents/method_150-1_1982.pdf",
          "properties": {
            "localId": "SAM.09.LAA.822.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "Sensors@navigationLink": "endpoint/ObservingProcedures(1)/Sensors"
        },
        {
          "@id": "endpoint/ObservingProcedures(2)",
          "id": 2,
          "name": "Standard EPA Method 160.1",
          "properties": {
            "localId": "SAM.09.LOB.823.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "Sensors@navigationLink": "endpoint/ObservingProcedures(2)/Sensors"
        }
      ],
      "@nextLink": "endpoint/ObservingProcedures?$top=2&$skip=2"
    }
                    """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/ObservingProcedures?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                {
                "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ObservingProcedures",
                  "value": [
                    {
                      "@id": "endpoint/ObservingProcedures(3)",
                      "id": 3,
    "description": "Description 2",
          "name": "Standard EPA Method 170.1",
          "definition": "other pdf",
          "properties": {
            "localId": "SAM.09.LOB.824.1.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "Sensors@navigationLink": "endpoint/ObservingProcedures(3)/Sensors"
                         }
                  ]
                }
                                """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='ObservingProcedures'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn(
                "Entity Type</td><td>ObservingProcedure</td>", vl.htmlMetadata()
            )
            self.assertIn(
                f'href="http://{endpoint}/ObservingProcedures"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                ["id", "selfLink", "name", "definition", "description", "properties"],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-23:] for f in features],
                [
                    "/ObservingProcedures(1)",
                    "/ObservingProcedures(2)",
                    "/ObservingProcedures(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                [
                    "Standard EPA Method 150.1",
                    "Standard EPA Method 160.1",
                    "Standard EPA Method 170.1",
                ],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "Electrometric measurement of pH in drinking, surface, and saline waters.",
                    None,
                    "Description 2",
                ],
            )
            self.assertEqual(
                [f["definition"] for f in features],
                [
                    "https://www.epa.gov/sites/default/files/2015-08/documents/method_150-1_1982.pdf",
                    None,
                    "other pdf",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

    def test_sampler_2_0(self):
        """
        Test a layer retrieving 'Samplers' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": [
        {
          "name": "Samplers",
          "url": "endpoint/Samplers"
        }
      ],
    "serverSettings": {
      "conformance": [
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
      ]
      }
    }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Samplers?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Samplers","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/Samplers?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
    {
      "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Samplers",
      "value": [
        {
          "@id": "endpoint/Samplers(1)",
          "id": 1,
          "description": "A standard 1-Liter vertical water sampler.",
          "name": "Kemmerer Bottle",
          "properties": {
            "localId": "SAM.09.LAA.822.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "SamplingProcedures@navigationLink": "endpoint/Samplers(1)/SamplingProcedures"
        },
        {
          "@id": "endpoint/Samplers(2)",
          "id": 2,
          "name": "Field Technician Alice",
          "properties": {
            "localId": "SAM.09.LOB.823.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "SamplingProcedures@navigationLink": "endpoint/Samplers(2)/SamplingProcedures"
        }
      ],
      "@nextLink": "endpoint/Samplers?$top=2&$skip=2"
    }
                    """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Samplers?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                {
                "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Samplers",
                  "value": [
                    {
                      "@id": "endpoint/Samplers(3)",
                      "id": 3,
    "description": "Description 2",
          "name": "Field Technician Bob",
          "description": "Senior field technician responsible for manual grab samples.",
          "samplerType": "human",
          "properties": {
            "localId": "SAM.09.LOB.824.1.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "SamplingProcedures@navigationLink": "endpoint/Samplers(3)/SamplingProcedures"
                         }
                  ]
                }
                                """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='Samplers'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn("Entity Type</td><td>Sampler</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Samplers"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "definition",
                    "description",
                    "properties",
                    "samplerType",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-12:] for f in features],
                [
                    "/Samplers(1)",
                    "/Samplers(2)",
                    "/Samplers(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Kemmerer Bottle", "Field Technician Alice", "Field Technician Bob"],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "A standard 1-Liter vertical water sampler.",
                    None,
                    "Senior field technician responsible for manual grab samples.",
                ],
            )
            self.assertEqual(
                [f["samplerType"] for f in features],
                [None, None, "human"],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

    def test_sampling_procedures_2_0(self):
        """
        Test a layer retrieving 'SamplingProcedures' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": [
        {
          "name": "SamplingProcedures",
          "url": "endpoint/SamplingProcedures"
        }
      ],
    "serverSettings": {
      "conformance": [
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
      ]
      }
    }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/SamplingProcedures?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#SamplingProcedures","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/SamplingProcedures?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
    {
      "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#SamplingProcedures",
      "value": [
        {
          "@id": "endpoint/SamplingProcedures(1)",
          "id": 1,
          "description": "Standard operating procedure for collecting surface water into sterile vials.",
          "name": "EPA Surface Water Grab Sampling",
          "definition": "https://example.com/epa-grab-sample-sop.pdf",
          "properties": {
            "localId": "SAM.09.LAA.822.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "Samplers@navigationLink": "endpoint/SamplingProcedures(1)/Samplers"
        },
        {
          "@id": "endpoint/SamplingProcedures(2)",
          "id": 2,
          "name": "EPA Surface Water Net Sampling",
          "properties": {
            "localId": "SAM.09.LOB.823.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "Samplers@navigationLink": "endpoint/SamplingProcedures(2)/Samplers"
        }
      ],
      "@nextLink": "endpoint/SamplingProcedures?$top=2&$skip=2"
    }
                    """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/SamplingProcedures?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                {
                "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#SamplingProcedures",
                  "value": [
                    {
                      "@id": "endpoint/SamplingProcedures(3)",
                      "id": 3,
          "description": "Another description.",
          "name": "EPA Surface Air Grab Sampling",
          "definition": "A pdf",
          "properties": {
            "localId": "SAM.09.LOB.824.1.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "Samplers@navigationLink": "endpoint/SamplingProcedures(3)/Samplers"
                         }
                  ]
                }
                                """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='SamplingProcedures'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn(
                "Entity Type</td><td>SamplingProcedure</td>", vl.htmlMetadata()
            )
            self.assertIn(
                f'href="http://{endpoint}/SamplingProcedures"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                ["id", "selfLink", "name", "definition", "description", "properties"],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-22:] for f in features],
                [
                    "/SamplingProcedures(1)",
                    "/SamplingProcedures(2)",
                    "/SamplingProcedures(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                [
                    "EPA Surface Water Grab Sampling",
                    "EPA Surface Water Net Sampling",
                    "EPA Surface Air Grab Sampling",
                ],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "Standard operating procedure for collecting surface water into sterile vials.",
                    None,
                    "Another description.",
                ],
            )
            self.assertEqual(
                [f["definition"] for f in features],
                ["https://example.com/epa-grab-sample-sop.pdf", None, "A pdf"],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

    def test_sampling_2_0(self):
        """
        Test a layer retrieving 'Samplings' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": [
        {
          "name": "Samplings",
          "url": "endpoint/Samplings"
        }
      ],
    "serverSettings": {
      "conformance": [
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
      ]
      }
    }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Samplings?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Samplings","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/Samplings?$top=2&$count=false&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
    {
      "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Samplings",
      "value": [
        {
          "@id": "endpoint/Samplings(1)",
          "id": 1,
          "description": "The physical act of drawing the water sample from the lake.",
          "name": "Extraction of Vial #001",
          "time": {
        "start": "2026-05-21T08:00:00Z"
      },
      "location": {
        "type": "Point",
        "coordinates": [148.124, -34.294]
      },
          "properties": {
            "localId": "SAM.09.LAA.822.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "Samplers@navigationLink": "endpoint/Samplings(1)/Samplers"
        },
        {
          "@id": "endpoint/Samplings(2)",
          "id": 2,
          "name": "Morning Grab Sample Event",
          "description": "Pulling the 8:00 AM sample from the central basin.",
          "location": {
        "type": "Point",
        "coordinates": [149.124, -35.294]
      },
      "time": {
        "start": "2026-05-22T08:00:00Z"
      },
          "properties": {
            "localId": "SAM.09.LOB.823.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "Samplers@navigationLink": "endpoint/Samplings(2)/Samplers"
        }
      ],
      "@nextLink": "endpoint/Samplings?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"
    }
                    """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Samplings?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                {
                "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#Samplings",
                  "value": [
                    {
                      "@id": "endpoint/Samplings(3)",
                      "id": 3,
          "description": "Another description.",
          "name": "Extraction of Vial #002",
          "definition": "A pdf",
          "time": {
        "start": "2026-05-22T08:00:00Z"
      },
        "location": {
        "type": "Point",
        "coordinates": [149.224, -35.394]
      },
          "properties": {
            "localId": "SAM.09.LOB.824.1.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "Samplers@navigationLink": "endpoint/Samplings(3)/Samplers"
                         }
                  ]
                }
                                """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=PointZ entity='Samplings'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn("Entity Type</td><td>Sampling</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Samplings"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "definition",
                    "description",
                    "properties",
                    "timeStart",
                    "timeEnd",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.DateTime,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                [
                    "/Samplings(1)",
                    "/Samplings(2)",
                    "/Samplings(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                [
                    "Extraction of Vial #001",
                    "Morning Grab Sample Event",
                    "Extraction of Vial #002",
                ],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "The physical act of drawing the water sample from the lake.",
                    "Pulling the 8:00 AM sample from the central basin.",
                    "Another description.",
                ],
            )
            self.assertEqual(
                [f["definition"] for f in features],
                [None, None, "A pdf"],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (148.1 -34.3)", "Point (149.1 -35.3)", "Point (149.2 -35.4)"],
            )

    def test_preparation_procedures_2_0(self):
        """
        Test a layer retrieving 'PreparationProcedures' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": [
        {
          "name": "PreparationProcedures",
          "url": "endpoint/PreparationProcedures"
        }
      ],
    "serverSettings": {
      "conformance": [
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
      ]
      }
    }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/PreparationProcedures?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#PreparationProcedures","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/PreparationProcedures?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
    {
      "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#PreparationProcedures",
      "value": [
        {
          "@id": "endpoint/PreparationProcedures(1)",
          "id": 1,
          "description": "Filtering the sample through a 0.45 micron membrane to remove suspended solids prior to analysis.",
          "name": "0.45µm Membrane Filtration",
          "definition": "https://example.com/lab-filtration-method.pdf",
          "properties": {
            "localId": "SAM.09.LAA.822.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "PreparationSteps@navigationLink": "endpoint/PreparationProcedures(1)/PreparationSteps"
        },
        {
          "@id": "endpoint/PreparationProcedures(2)",
          "id": 2,
          "name": "0.65µm Membrane Filtration",
          "properties": {
            "localId": "SAM.09.LOB.823.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "PreparationSteps@navigationLink": "endpoint/PreparationProcedures(2)/PreparationSteps"
        }
      ],
      "@nextLink": "endpoint/PreparationProcedures?$top=2&$skip=2"
    }
                    """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/PreparationProcedures?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                {
                "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#PreparationProcedures",
                  "value": [
                    {
                      "@id": "endpoint/PreparationProcedures(3)",
                      "id": 3,
          "description": "Filtering the sample through a 0.95 micron membrane to remove suspended solids prior to analysis.",
          "name": "0.95µm Membrane Filtration",
          "definition": "https://example.com/lab-filtration-method95.pdf",
          "properties": {
            "localId": "SAM.09.LOB.824.1.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "PreparationSteps@navigationLink": "endpoint/PreparationProcedures(3)/PreparationSteps"
                         }
                  ]
                }
                                """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='PreparationProcedures'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn(
                "Entity Type</td><td>PreparationProcedure</td>", vl.htmlMetadata()
            )
            self.assertIn(
                f'href="http://{endpoint}/PreparationProcedures"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                ["id", "selfLink", "name", "definition", "description", "properties"],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-25:] for f in features],
                [
                    "/PreparationProcedures(1)",
                    "/PreparationProcedures(2)",
                    "/PreparationProcedures(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                [
                    "0.45µm Membrane Filtration",
                    "0.65µm Membrane Filtration",
                    "0.95µm Membrane Filtration",
                ],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "Filtering the sample through a 0.45 micron membrane to remove suspended solids prior to analysis.",
                    None,
                    "Filtering the sample through a 0.95 micron membrane to remove suspended solids prior to analysis.",
                ],
            )
            self.assertEqual(
                [f["definition"] for f in features],
                [
                    "https://example.com/lab-filtration-method.pdf",
                    None,
                    "https://example.com/lab-filtration-method95.pdf",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

    def test_preparation_steps_2_0(self):
        """
        Test a layer retrieving 'PreparationSteps' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": [
        {
          "name": "PreparationSteps",
          "url": "endpoint/PreparationSteps"
        }
      ],
    "serverSettings": {
      "conformance": [
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
      ]
      }
    }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/PreparationSteps?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#PreparationSteps","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/PreparationSteps?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
    {
      "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#PreparationSteps",
      "value": [
        {
          "@id": "endpoint/PreparationSteps(1)",
          "id": 1,
          "description": "Filtering the morning lake water sample in the lab.",
          "name": "Filtration of Vial #001",
          "definition": "https://example.com/lab-filtration-method.pdf",
           "time": "2026-05-22T10:30:00Z",
          "properties": {
            "localId": "SAM.09.LAA.822.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "PreparedSample@navigationLink": "endpoint/PreparationSteps(1)/PreparedSample"
        },
        {
          "@id": "endpoint/PreparationSteps(2)",
          "id": 2,
          "name": "Filtration of Vial #002",
          "properties": {
            "localId": "SAM.09.LOB.823.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "PreparedSample@navigationLink": "endpoint/PreparationSteps(2)/PreparedSample"
        }
      ],
      "@nextLink": "endpoint/PreparationSteps?$top=2&$skip=2"
    }
                    """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/PreparationSteps?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                {
                "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#PreparationSteps",
                  "value": [
                    {
                      "@id": "endpoint/PreparationSteps(3)",
                      "id": 3,
          "description": "Filtering the afternoon lake water sample in the lab.",
          "name": "Filtration of Vial #003",
           "time": "2026-05-23T10:30:00Z",
          "definition": "https://example.com/lab-filtration-method95.pdf",
          "properties": {
            "localId": "SAM.09.LOB.824.1.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "PreparedSample@navigationLink": "endpoint/PreparationSteps(3)/PreparedSample"
                         }
                  ]
                }
                                """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='PreparationSteps'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn("Entity Type</td><td>PreparationStep</td>", vl.htmlMetadata())
            self.assertIn(
                f'href="http://{endpoint}/PreparationSteps"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "definition",
                    "description",
                    "properties",
                    "timeStart",
                    "timeEnd",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.DateTime,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-20:] for f in features],
                [
                    "/PreparationSteps(1)",
                    "/PreparationSteps(2)",
                    "/PreparationSteps(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                [
                    "Filtration of Vial #001",
                    "Filtration of Vial #002",
                    "Filtration of Vial #003",
                ],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "Filtering the morning lake water sample in the lab.",
                    None,
                    "Filtering the afternoon lake water sample in the lab.",
                ],
            )
            self.assertEqual(
                [f["definition"] for f in features],
                [
                    "https://example.com/lab-filtration-method.pdf",
                    None,
                    "https://example.com/lab-filtration-method95.pdf",
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

    def test_relation_role_2_0(self):
        """
        Test a layer retrieving 'RelationRoles' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": [
        {
          "name": "RelationRoles",
          "url": "endpoint/RelationRoles"
        }
      ],
    "serverSettings": {
      "conformance": [
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
      ]
      }
    }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/RelationRoles?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#RelationRoles","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/RelationRoles?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
    {
      "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#RelationRoles",
      "value": [
        {
          "@id": "endpoint/RelationRoles(1)",
          "id": 1,
            "name": "Sub-component",
            "inverseName": "Parent",
            "description": "Indicates that the subject is a physical sub-component or part of the object.",
          "properties": {
            "localId": "SAM.09.LAA.822.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "FeatureRelations@navigationLink": "endpoint/RelationRoles(1)/FeatureRelations"
        },
        {
          "@id": "endpoint/RelationRoles(2)",
          "id": 2,
"name": "Upstream Of",
          "properties": {
            "localId": "SAM.09.LOB.823.7.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "FeatureRelations@navigationLink": "endpoint/RelationRoles(2)/FeatureRelations"
        }
      ],
      "@nextLink": "endpoint/RelationRoles?$top=2&$skip=2"
    }
                    """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/RelationRoles?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                {
                "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#RelationRoles",
                  "value": [
                    {
                      "@id": "endpoint/RelationRoles(3)",
                      "id": 3,
 "name": "Derived From",
      "inverseName": "Source Of",
      "description": "Indicates that the subject data was calculated, corrected, or derived from the object data.",
          "properties": {
            "localId": "SAM.09.LOB.824.1.1",
            "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
            "namespace": "AT.0008.20.AQ",
            "owner": "http://luft.umweltbundesamt.at"
          },
          "FeatureRelations@navigationLink": "endpoint/RelationRoles(3)/FeatureRelations"
                         }
                  ]
                }
                                """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='RelationRoles'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn("Entity Type</td><td>RelationRole</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/RelationRoles"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "definition",
                    "inverseName",
                    "inverseDefinition",
                    "description",
                    "properties",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-17:] for f in features],
                [
                    "/RelationRoles(1)",
                    "/RelationRoles(2)",
                    "/RelationRoles(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Sub-component", "Upstream Of", "Derived From"],
            )
            self.assertEqual(
                [f["description"] for f in features],
                [
                    "Indicates that the subject is a physical sub-component or part of the object.",
                    None,
                    "Indicates that the subject data was calculated, corrected, or derived from the object data.",
                ],
            )
            self.assertEqual(
                [f["inverseName"] for f in features],
                ["Parent", None, "Source Of"],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {
                        "localId": "SAM.09.LAA.822.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.823.7.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                    {
                        "localId": "SAM.09.LOB.824.1.1",
                        "metadata": "http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample",
                        "namespace": "AT.0008.20.AQ",
                        "owner": "http://luft.umweltbundesamt.at",
                    },
                ],
            )

    def test_thing_relation(self):
        """
        Test a layer retrieving 'ThingRelations' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
        {
          "value": [
            {
              "name": "ThingRelations",
              "url": "endpoint/ThingRelations"
            }
          ],
        "serverSettings": {
          "conformance": [
          "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
          ]
          }
        }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/ThingRelations?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ThingRelations","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/ThingRelations?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
        {
          "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ThingRelations",
          "value": [
            {
              "@id": "endpoint/ThingRelations(1)",
              "id": 1,
               "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-8842",
              "RelationRole@navigationLink": "endpoint/ThingRelations(1)/RelationRole"
            },
            {
              "@id": "endpoint/ThingRelations(2)",
              "id": 2,
              "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-111",
              "RelationRole@navigationLink": "endpoint/ThingRelations(2)/RelationRole"
            }
          ],
          "@nextLink": "endpoint/ThingRelations?$top=2&$skip=2"
        }
                        """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/ThingRelations?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                    {
                    "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ThingRelations",
                      "value": [
                        {
                          "@id": "endpoint/ThingRelations(3)",
                          "id": 3,
     "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-333",
              "RelationRole@navigationLink": "endpoint/ThingRelations(3)/RelationRole"
                             }
                      ]
                    }
                                    """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='ThingRelations'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn("Entity Type</td><td>ThingRelation</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/ThingRelations"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                ["id", "selfLink", "externalTarget"],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-18:] for f in features],
                [
                    "/ThingRelations(1)",
                    "/ThingRelations(2)",
                    "/ThingRelations(3)",
                ],
            )
            self.assertEqual(
                [f["externalTarget"] for f in features],
                [
                    "https://national-buoy-registry.example.gov/equipment/buoy-8842",
                    "https://national-buoy-registry.example.gov/equipment/buoy-111",
                    "https://national-buoy-registry.example.gov/equipment/buoy-333",
                ],
            )

    def test_datastream_relation(self):
        """
        Test a layer retrieving 'DatastreamRelations' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": [
        {
          "name": "DatastreamRelations",
          "url": "endpoint/DatastreamRelations"
        }
      ],
    "serverSettings": {
      "conformance": [
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
      ]
      }
    }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/DatastreamRelations?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#DatastreamRelations","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/DatastreamRelations?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
    {
      "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#DatastreamRelations",
      "value": [
        {
          "@id": "endpoint/DatastreamRelations(1)",
          "id": 1,
           "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-8842",
          "RelationRole@navigationLink": "endpoint/DatastreamRelations(1)/RelationRole"
        },
        {
          "@id": "endpoint/DatastreamRelations(2)",
          "id": 2,
          "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-111",
          "RelationRole@navigationLink": "endpoint/DatastreamRelations(2)/RelationRole"
        }
      ],
      "@nextLink": "endpoint/DatastreamRelations?$top=2&$skip=2"
    }
                    """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/DatastreamRelations?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                {
                "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#DatastreamRelations",
                  "value": [
                    {
                      "@id": "endpoint/DatastreamRelations(3)",
                      "id": 3,
 "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-333",
          "RelationRole@navigationLink": "endpoint/DatastreamRelations(3)/RelationRole"
                         }
                  ]
                }
                                """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='DatastreamRelations'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn(
                "Entity Type</td><td>DatastreamRelation</td>", vl.htmlMetadata()
            )
            self.assertIn(
                f'href="http://{endpoint}/DatastreamRelations"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                ["id", "selfLink", "externalTarget"],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-23:] for f in features],
                [
                    "/DatastreamRelations(1)",
                    "/DatastreamRelations(2)",
                    "/DatastreamRelations(3)",
                ],
            )
            self.assertEqual(
                [f["externalTarget"] for f in features],
                [
                    "https://national-buoy-registry.example.gov/equipment/buoy-8842",
                    "https://national-buoy-registry.example.gov/equipment/buoy-111",
                    "https://national-buoy-registry.example.gov/equipment/buoy-333",
                ],
            )

    def test_feature_relation(self):
        """
        Test a layer retrieving 'FeatureRelations' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": [
        {
          "name": "FeatureRelations",
          "url": "endpoint/FeatureRelations"
        }
      ],
    "serverSettings": {
      "conformance": [
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
      ]
      }
    }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/FeatureRelations?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#FeatureRelations","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/FeatureRelations?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
    {
      "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#FeatureRelations",
      "value": [
        {
          "@id": "endpoint/FeatureRelations(1)",
          "id": 1,
           "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-8842",
          "RelationRole@navigationLink": "endpoint/FeatureRelations(1)/RelationRole"
        },
        {
          "@id": "endpoint/FeatureRelations(2)",
          "id": 2,
          "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-111",
          "RelationRole@navigationLink": "endpoint/FeatureRelations(2)/RelationRole"
        }
      ],
      "@nextLink": "endpoint/FeatureRelations?$top=2&$skip=2"
    }
                    """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/FeatureRelations?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                {
                "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#FeatureRelations",
                  "value": [
                    {
                      "@id": "endpoint/FeatureRelations(3)",
                      "id": 3,
 "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-333",
          "RelationRole@navigationLink": "endpoint/FeatureRelations(3)/RelationRole"
                         }
                  ]
                }
                                """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='FeatureRelations'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn("Entity Type</td><td>FeatureRelation</td>", vl.htmlMetadata())
            self.assertIn(
                f'href="http://{endpoint}/FeatureRelations"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                ["id", "selfLink", "externalTarget"],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-20:] for f in features],
                [
                    "/FeatureRelations(1)",
                    "/FeatureRelations(2)",
                    "/FeatureRelations(3)",
                ],
            )
            self.assertEqual(
                [f["externalTarget"] for f in features],
                [
                    "https://national-buoy-registry.example.gov/equipment/buoy-8842",
                    "https://national-buoy-registry.example.gov/equipment/buoy-111",
                    "https://national-buoy-registry.example.gov/equipment/buoy-333",
                ],
            )

    def test_observation_relation(self):
        """
        Test a layer retrieving 'ObservationRelations' entities from a 2.0 service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
    {
      "value": [
        {
          "name": "ObservationRelations",
          "url": "endpoint/ObservationRelations"
        }
      ],
    "serverSettings": {
      "conformance": [
      "http://www.opengis.net/spec/sensorthings/2.0/req-class/datamodel/core"
      ]
      }
    }""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/ObservationRelations?$top=0&$count=true",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """{"@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ObservationRelations","@count":3,"value":[]}"""
                )

            with open(
                sanitize(
                    endpoint,
                    "/ObservationRelations?$top=2&$count=false",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
    {
      "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ObservationRelations",
      "value": [
        {
          "@id": "endpoint/ObservationRelations(1)",
          "id": 1,
           "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-8842",
          "RelationRole@navigationLink": "endpoint/ObservationRelations(1)/RelationRole"
        },
        {
          "@id": "endpoint/ObservationRelations(2)",
          "id": 2,
          "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-111",
          "RelationRole@navigationLink": "endpoint/ObservationRelations(2)/RelationRole"
        }
      ],
      "@nextLink": "endpoint/ObservationRelations?$top=2&$skip=2"
    }
                    """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/ObservationRelations?$top=2&$skip=2",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
                {
                "@context":"https://ogc-demo.xxx.de/yyy/v2.0/$metadata#ObservationRelations",
                  "value": [
                    {
                      "@id": "endpoint/ObservationRelations(3)",
                      "id": 3,
 "externalTarget": "https://national-buoy-registry.example.gov/equipment/buoy-333",
          "RelationRole@navigationLink": "endpoint/ObservationRelations(3)/RelationRole"
                         }
                  ]
                }
                                """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 entity='ObservationRelations'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.dataProvider().metadata()["SensorThingsVersion"], 2.0)

            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 3)
            self.assertIn(
                "Entity Type</td><td>ObservationRelation</td>", vl.htmlMetadata()
            )
            self.assertIn(
                f'href="http://{endpoint}/ObservationRelations"', vl.htmlMetadata()
            )

            self.assertEqual(
                [f.name() for f in vl.fields()],
                ["id", "selfLink", "externalTarget"],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-24:] for f in features],
                [
                    "/ObservationRelations(1)",
                    "/ObservationRelations(2)",
                    "/ObservationRelations(3)",
                ],
            )
            self.assertEqual(
                [f["externalTarget"] for f in features],
                [
                    "https://national-buoy-registry.example.gov/equipment/buoy-8842",
                    "https://national-buoy-registry.example.gov/equipment/buoy-111",
                    "https://national-buoy-registry.example.gov/equipment/buoy-333",
                ],
            )

    def test_feature_expansion(self):
        """
        Test a layer using feature expansion
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=location/type eq 'Polygon' or location/geometry/type eq 'Polygon'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$expand=Things($expand=Datastreams)&$filter=location/type eq 'Polygon' or location/geometry/type eq 'Polygon'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Locations(1)",
      "@iot.id": 1,
      "name": "Location 1",
      "description": "Desc 1",
      "unitOfMeasurements": [
          {
            "name": "ug.m-3",
            "symbol": "ug.m-3",
            "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
          }
      ],
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
      "phenomenonTime": "2017-12-31T23:00:00Z/2018-01-12T04:00:00Z",
      "resultTime": "2017-12-31T23:30:00Z/2017-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 1"
      },
      "Things":
        {
          "@iot.selfLink": "endpoint/Things(1)",
          "@iot.id": 1,
          "name": "Thing 1",
          "description": "Description Thing 1",
          "properties": {
            "countryCode": "AT"
          },
          "Datastreams": [
            {
              "@iot.selfLink": "endpoint/Datastreams(45)",
              "@iot.id": 45,
              "name": "Datastream 45",
              "description": "Description datastream 45",
              "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
              "unitOfMeasurement": {
                "name": "ug.m-3",
                "symbol": "ug.m-3",
                "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
              },
              "phenomenonTime": "2017-12-31T23:00:00Z/2024-03-25T04:00:00Z",
              "properties": {
                "owner": "someone"
              }
            },
            {
              "@iot.selfLink": "endpoint/Datastreams(46)",
              "@iot.id": 46,
              "name": "Datastream 46",
              "description": "Description datastream 46",
              "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
              "unitOfMeasurement": {
                "name": "ug.m-3",
                "symbol": "ug.m-3",
                "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
              },
              "phenomenonTime": "2018-12-31T23:00:00Z/2024-03-25T04:00:00Z",
              "properties": {
                "owner": "someone"
              }
            }
            ]
        },
      "location": {
            "type": "Polygon",
            "coordinates": [
              [
                [100, 0], [101, 0], [101, 1], [100, 1], [100, 0]
              ]
            ]
          },
      "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/Locations(2)",
      "@iot.id": 2,
      "name": "Location 2",
      "description": "Desc 2",
      "unitOfMeasurements": [
      {
        "name": "ug.m-3",
        "symbol": "ug.m-3",
        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
      }],
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
      "phenomenonTime": "2018-12-31T23:00:00Z/2019-01-12T04:00:00Z",
      "resultTime": "2018-12-31T23:30:00Z/2018-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 2"
      },
      "Things": [
        {
          "@iot.selfLink": "endpoint/Things(2)",
          "@iot.id": 2,
          "name": "Thing 2",
          "description": "Description Thing 2",
          "properties": {
            "countryCode": "AT"
          },
          "Datastreams": [
            {
              "@iot.selfLink": "endpoint/Datastreams(51)",
              "@iot.id": 51,
              "name": "Datastream 51",
              "description": "Description datastream 51",
              "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
              "unitOfMeasurement": {
                "name": "ug.m-3",
                "symbol": "ug.m-3",
                "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
              },
              "phenomenonTime": "2017-12-31T23:00:00Z/2024-03-25T04:00:00Z",
              "properties": {
                "owner": "someone"
              }
            }
            ]
        },
        {
          "@iot.selfLink": "endpoint/Things(3)",
          "@iot.id": 3,
          "name": "Thing 3",
          "description": "Description Thing 3",
          "properties": {
            "countryCode": "AT"
          },
          "Datastreams": [
            {
              "@iot.selfLink": "endpoint/Datastreams(52)",
              "@iot.id": 52,
              "name": "Datastream 52",
              "description": "Description datastream 52",
              "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
              "unitOfMeasurement": {
                "name": "ug.m-3",
                "symbol": "ug.m-3",
                "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
              },
              "phenomenonTime": "2017-12-31T23:00:00Z/2024-03-25T04:00:00Z",
              "properties": {
                "owner": "someone"
              }
            }
            ]
        }
       ],
            "location": {
            "type": "Polygon",
            "coordinates": [
              [
                [102, 0], [103, 0], [103, 1], [102, 1], [102, 0]
              ]
            ]
          },
      "Things@iot.navigationLink": "endpoint/Locations(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/Locations?$top=2&$skip=2&$expand=Things($expand=Datastreams)&$filter=location/type eq 'Polygon' or location/geometry/type eq 'Polygon'"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=2&$skip=2&$expand=Things($expand=Datastreams)&$filter=location/type eq 'Polygon' or location/geometry/type eq 'Polygon'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/Locations(3)",
                  "@iot.id": 3,
                  "name": "Location 3",
                  "description": "Desc 3",
                  "unitOfMeasurements": [{
                    "name": "ug.m-3",
                    "symbol": "ug.m-3",
                    "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
                  }],
                  "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                  "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
                  "phenomenonTime": "2020-12-31T23:00:00Z/2021-01-12T04:00:00Z",
                  "resultTime": "2020-12-31T23:30:00Z/2020-12-31T23:31:00Z",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things": [
        {
          "@iot.selfLink": "endpoint/Things(8)",
          "@iot.id": 8,
          "name": "Thing 8",
          "description": "Description Thing 8",
          "properties": {
            "countryCode": "AT"
          },
          "Datastreams": [
            {
              "@iot.selfLink": "endpoint/Datastreams(59)",
              "@iot.id": 59,
              "name": "Datastream 59",
              "description": "Description datastream 59",
              "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
              "unitOfMeasurement": {
                "name": "ug.m-3",
                "symbol": "ug.m-3",
                "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
              },
              "phenomenonTime": "2017-12-31T23:00:00Z/2024-03-25T04:00:00Z",
              "properties": {
                "owner": "someone"
              }
            },
            {
              "@iot.selfLink": "endpoint/Datastreams(60)",
              "@iot.id": 60,
              "name": "Datastream 60",
              "description": "Description datastream 60",
              "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
              "unitOfMeasurement": {
                "name": "ug.m-3",
                "symbol": "ug.m-3",
                "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
              },
              "phenomenonTime": "2017-12-31T23:00:00Z/2024-03-25T04:00:00Z",
              "properties": {
                "owner": "someone"
              }
            }
            ]
        }
       ],
                        "location": {
            "type": "Polygon",
            "coordinates": [
              [
                [103, 0], [104, 0], [104, 1], [103, 1], [103, 0]
              ]
            ]
          },
                  "Things@iot.navigationLink": "endpoint/Locations(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=MultiPolygonZ entity='Location' expandTo='Thing;Datastream'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.MultiPolygonZ)

            self.assertEqual(vl.featureCount(), -1)
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Location</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                    "Thing_id",
                    "Thing_selfLink",
                    "Thing_name",
                    "Thing_description",
                    "Thing_properties",
                    "Thing_Datastream_id",
                    "Thing_Datastream_selfLink",
                    "Thing_Datastream_name",
                    "Thing_Datastream_description",
                    "Thing_Datastream_unitOfMeasurement",
                    "Thing_Datastream_observationType",
                    "Thing_Datastream_properties",
                    "Thing_Datastream_phenomenonTimeStart",
                    "Thing_Datastream_phenomenonTimeEnd",
                    "Thing_Datastream_resultTimeStart",
                    "Thing_Datastream_resultTimeEnd",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2, 3, 4, 5])
            self.assertEqual(
                [f["id"] for f in features], ["1", "1", "2", "2", "3", "3"]
            )
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                [
                    "/Locations(1)",
                    "/Locations(1)",
                    "/Locations(2)",
                    "/Locations(2)",
                    "/Locations(3)",
                    "/Locations(3)",
                ],
            )
            self.assertEqual(
                [f["name"] for f in features],
                [
                    "Location 1",
                    "Location 1",
                    "Location 2",
                    "Location 2",
                    "Location 3",
                    "Location 3",
                ],
            )
            self.assertEqual(
                [f["description"] for f in features],
                ["Desc 1", "Desc 1", "Desc 2", "Desc 2", "Desc 3", "Desc 3"],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [
                    {"owner": "owner 1"},
                    {"owner": "owner 1"},
                    {"owner": "owner 2"},
                    {"owner": "owner 2"},
                    {"owner": "owner 3"},
                    {"owner": "owner 3"},
                ],
            )
            self.assertEqual(
                [f["Thing_id"] for f in features], ["1", "1", "2", "3", "8", "8"]
            )
            self.assertEqual(
                [f["Thing_selfLink"][-10:] for f in features],
                [
                    "/Things(1)",
                    "/Things(1)",
                    "/Things(2)",
                    "/Things(3)",
                    "/Things(8)",
                    "/Things(8)",
                ],
            )
            self.assertEqual(
                [f["Thing_name"] for f in features],
                ["Thing 1", "Thing 1", "Thing 2", "Thing 3", "Thing 8", "Thing 8"],
            )
            self.assertEqual(
                [f["Thing_description"] for f in features],
                [
                    "Description Thing 1",
                    "Description Thing 1",
                    "Description Thing 2",
                    "Description Thing 3",
                    "Description Thing 8",
                    "Description Thing 8",
                ],
            )
            self.assertEqual(
                [f["Thing_properties"] for f in features],
                [
                    {"countryCode": "AT"},
                    {"countryCode": "AT"},
                    {"countryCode": "AT"},
                    {"countryCode": "AT"},
                    {"countryCode": "AT"},
                    {"countryCode": "AT"},
                ],
            )
            self.assertEqual(
                [f["Thing_Datastream_id"] for f in features],
                ["45", "46", "51", "52", "59", "60"],
            )
            self.assertEqual(
                [f["Thing_Datastream_selfLink"][-16:] for f in features],
                [
                    "/Datastreams(45)",
                    "/Datastreams(46)",
                    "/Datastreams(51)",
                    "/Datastreams(52)",
                    "/Datastreams(59)",
                    "/Datastreams(60)",
                ],
            )
            self.assertEqual(
                [f["Thing_Datastream_name"] for f in features],
                [
                    "Datastream 45",
                    "Datastream 46",
                    "Datastream 51",
                    "Datastream 52",
                    "Datastream 59",
                    "Datastream 60",
                ],
            )
            self.assertEqual(
                [f["Thing_Datastream_description"] for f in features],
                [
                    "Description datastream 45",
                    "Description datastream 46",
                    "Description datastream 51",
                    "Description datastream 52",
                    "Description datastream 59",
                    "Description datastream 60",
                ],
            )
            self.assertEqual(
                [f["Thing_Datastream_properties"] for f in features],
                [
                    {"owner": "someone"},
                    {"owner": "someone"},
                    {"owner": "someone"},
                    {"owner": "someone"},
                    {"owner": "someone"},
                    {"owner": "someone"},
                ],
            )

            self.assertEqual(
                [f.geometry().asWkt() for f in features],
                [
                    "Polygon ((100 0, 101 0, 101 1, 100 1, 100 0))",
                    "Polygon ((100 0, 101 0, 101 1, 100 1, 100 0))",
                    "Polygon ((102 0, 103 0, 103 1, 102 1, 102 0))",
                    "Polygon ((102 0, 103 0, 103 1, 102 1, 102 0))",
                    "Polygon ((103 0, 104 0, 104 1, 103 1, 103 0))",
                    "Polygon ((103 0, 104 0, 104 1, 103 1, 103 0))",
                ],
            )

    def test_feature_expansion_with_limit(self):
        """
        Test a layer using feature expansion with limited child features
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "w", encoding="utf8") as f:
                f.write(
                    """
{
  "value": [
    {
      "name": "Locations",
      "url": "endpoint/Locations"
    }
  ],
  "serverSettings": {
  }
}""".replace("endpoint", "http://" + endpoint)
                )

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=0&$count=true&$filter=location/type eq 'Polygon' or location/geometry/type eq 'Polygon'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(
                    endpoint,
                    "/Locations?$top=2&$count=false&$expand=Things($expand=Datastreams($top=1))&$filter=location/type eq 'Polygon' or location/geometry/type eq 'Polygon'",
                ),
                "w",
                encoding="utf8",
            ) as f:
                f.write(
                    """
{
  "value": [
    {
      "@iot.selfLink": "endpoint/Locations(1)",
      "@iot.id": 1,
      "name": "Location 1",
      "description": "Desc 1",
      "unitOfMeasurements": [
          {
            "name": "ug.m-3",
            "symbol": "ug.m-3",
            "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
          }
      ],
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
      "phenomenonTime": "2017-12-31T23:00:00Z/2018-01-12T04:00:00Z",
      "resultTime": "2017-12-31T23:30:00Z/2017-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 1"
      },
      "Things": [
        {
          "@iot.selfLink": "endpoint/Things(1)",
          "@iot.id": 1,
          "name": "Thing 1",
          "description": "Description Thing 1",
          "properties": {
            "countryCode": "AT"
          },
          "Datastreams": [
            {
              "@iot.selfLink": "endpoint/Datastreams(45)",
              "@iot.id": 45,
              "name": "Datastream 45",
              "description": "Description datastream 45",
              "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
              "unitOfMeasurement": {
                "name": "ug.m-3",
                "symbol": "ug.m-3",
                "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
              },
              "phenomenonTime": "2017-12-31T23:00:00Z/2024-03-25T04:00:00Z",
              "properties": {
                "owner": "someone"
              }
            }
            ]
        }
       ],
      "location": {
            "type": "Polygon",
            "coordinates": [
              [
                [100, 0], [101, 0], [101, 1], [100, 1], [100, 0]
              ]
            ]
          },
      "Things@iot.navigationLink": "endpoint/Locations(1)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(1)/HistoricalLocations"
    },
    {
      "@iot.selfLink": "endpoint/Locations(2)",
      "@iot.id": 2,
      "name": "Location 2",
      "description": "Desc 2",
      "unitOfMeasurements": [
      {
        "name": "ug.m-3",
        "symbol": "ug.m-3",
        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
      }],
      "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
      "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
      "phenomenonTime": "2018-12-31T23:00:00Z/2019-01-12T04:00:00Z",
      "resultTime": "2018-12-31T23:30:00Z/2018-12-31T23:31:00Z",
      "properties": {
        "owner": "owner 2"
      },
      "Things": [
        {
          "@iot.selfLink": "endpoint/Things(2)",
          "@iot.id": 2,
          "name": "Thing 2",
          "description": "Description Thing 2",
          "properties": {
            "countryCode": "AT"
          },
          "Datastreams": [
            {
              "@iot.selfLink": "endpoint/Datastreams(51)",
              "@iot.id": 51,
              "name": "Datastream 51",
              "description": "Description datastream 51",
              "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
              "unitOfMeasurement": {
                "name": "ug.m-3",
                "symbol": "ug.m-3",
                "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
              },
              "phenomenonTime": "2017-12-31T23:00:00Z/2024-03-25T04:00:00Z",
              "properties": {
                "owner": "someone"
              }
            }
            ]
        }
       ],
            "location": {
            "type": "Polygon",
            "coordinates": [
              [
                [102, 0], [103, 0], [103, 1], [102, 1], [102, 0]
              ]
            ]
          },
      "Things@iot.navigationLink": "endpoint/Locations(2)/Things",
      "HistoricalLocations@iot.navigationLink": "endpoint/Locations(2)/HistoricalLocations"

    }
  ],
  "@iot.nextLink": "endpoint/Locations?$top=2&$skip=2&$expand=Things($expand=Datastreams($top=1))&$filter=location/type eq 'Polygon' or location/geometry/type eq 'Polygon'"
}
                """.replace("endpoint", "http://" + endpoint)
                )

                with open(
                    sanitize(
                        endpoint,
                        "/Locations?$top=2&$skip=2&$expand=Things($expand=Datastreams($top=1))&$filter=location/type eq 'Polygon' or location/geometry/type eq 'Polygon'",
                    ),
                    "w",
                    encoding="utf8",
                ) as f:
                    f.write(
                        """
            {
              "value": [
                {
                  "@iot.selfLink": "endpoint/Locations(3)",
                  "@iot.id": 3,
                  "name": "Location 3",
                  "description": "Desc 3",
                  "unitOfMeasurements": [{
                    "name": "ug.m-3",
                    "symbol": "ug.m-3",
                    "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
                  }],
                  "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
                  "multiObservationDataTypes": ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
                  "phenomenonTime": "2020-12-31T23:00:00Z/2021-01-12T04:00:00Z",
                  "resultTime": "2020-12-31T23:30:00Z/2020-12-31T23:31:00Z",
                  "properties": {
                    "owner": "owner 3"
                  },
                  "Things": [
        {
          "@iot.selfLink": "endpoint/Things(8)",
          "@iot.id": 8,
          "name": "Thing 8",
          "description": "Description Thing 8",
          "properties": {
            "countryCode": "AT"
          },
          "Datastreams": [
            {
              "@iot.selfLink": "endpoint/Datastreams(59)",
              "@iot.id": 59,
              "name": "Datastream 59",
              "description": "Description datastream 59",
              "observationType": "http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement",
              "unitOfMeasurement": {
                "name": "ug.m-3",
                "symbol": "ug.m-3",
                "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3"
              },
              "phenomenonTime": "2017-12-31T23:00:00Z/2024-03-25T04:00:00Z",
              "properties": {
                "owner": "someone"
              }
            }
            ]
        }
       ],
                        "location": {
            "type": "Polygon",
            "coordinates": [
              [
                [103, 0], [104, 0], [104, 1], [103, 1], [103, 0]
              ]
            ]
          },
                  "Things@iot.navigationLink": "endpoint/Locations(3)/Things",
                  "HistoricalLocations@iot.navigationLink": "endpoint/Locations(3)/HistoricalLocations"
                }
              ]
            }
                            """.replace("endpoint", "http://" + endpoint)
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=MultiPolygonZ entity='Location' expandTo='Thing;Datastream:limit=1'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            # basic layer properties tests
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.MultiPolygonZ)

            self.assertEqual(vl.featureCount(), -1)
            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Location</td>", vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"', vl.htmlMetadata())

            self.assertEqual(
                [f.name() for f in vl.fields()],
                [
                    "id",
                    "selfLink",
                    "name",
                    "description",
                    "properties",
                    "Thing_id",
                    "Thing_selfLink",
                    "Thing_name",
                    "Thing_description",
                    "Thing_properties",
                    "Thing_Datastream_id",
                    "Thing_Datastream_selfLink",
                    "Thing_Datastream_name",
                    "Thing_Datastream_description",
                    "Thing_Datastream_unitOfMeasurement",
                    "Thing_Datastream_observationType",
                    "Thing_Datastream_properties",
                    "Thing_Datastream_phenomenonTimeStart",
                    "Thing_Datastream_phenomenonTimeEnd",
                    "Thing_Datastream_resultTimeStart",
                    "Thing_Datastream_resultTimeEnd",
                ],
            )
            self.assertEqual(
                [f.type() for f in vl.fields()],
                [
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.String,
                    QVariant.Map,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                    QVariant.DateTime,
                ],
            )

            # test retrieving all features from layer
            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ["1", "2", "3"])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(2)", "/Locations(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["Location 1", "Location 2", "Location 3"],
            )
            self.assertEqual(
                [f["description"] for f in features], ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"}, {"owner": "owner 3"}],
            )
            self.assertEqual([f["Thing_id"] for f in features], ["1", "2", "8"])
            self.assertEqual(
                [f["Thing_selfLink"][-10:] for f in features],
                ["/Things(1)", "/Things(2)", "/Things(8)"],
            )
            self.assertEqual(
                [f["Thing_name"] for f in features], ["Thing 1", "Thing 2", "Thing 8"]
            )
            self.assertEqual(
                [f["Thing_description"] for f in features],
                ["Description Thing 1", "Description Thing 2", "Description Thing 8"],
            )
            self.assertEqual(
                [f["Thing_properties"] for f in features],
                [{"countryCode": "AT"}, {"countryCode": "AT"}, {"countryCode": "AT"}],
            )
            self.assertEqual(
                [f["Thing_Datastream_id"] for f in features], ["45", "51", "59"]
            )
            self.assertEqual(
                [f["Thing_Datastream_selfLink"][-16:] for f in features],
                ["/Datastreams(45)", "/Datastreams(51)", "/Datastreams(59)"],
            )
            self.assertEqual(
                [f["Thing_Datastream_name"] for f in features],
                ["Datastream 45", "Datastream 51", "Datastream 59"],
            )
            self.assertEqual(
                [f["Thing_Datastream_description"] for f in features],
                [
                    "Description datastream 45",
                    "Description datastream 51",
                    "Description datastream 59",
                ],
            )
            self.assertEqual(
                [f["Thing_Datastream_properties"] for f in features],
                [{"owner": "someone"}, {"owner": "someone"}, {"owner": "someone"}],
            )

            self.assertEqual(
                [f.geometry().asWkt() for f in features],
                [
                    "Polygon ((100 0, 101 0, 101 1, 100 1, 100 0))",
                    "Polygon ((102 0, 103 0, 103 1, 102 1, 102 0))",
                    "Polygon ((103 0, 104 0, 104 1, 103 1, 103 0))",
                ],
            )

    def testDecodeUri(self):
        """
        Test decoding a SensorThings uri
        """
        uri = "url='https://sometest.com/api' type=MultiPointZ authcfg='abc' pageSize='20' entity='Locations'"
        parts = QgsProviderRegistry.instance().decodeUri("sensorthings", uri)
        self.assertEqual(
            parts,
            {
                "url": "https://sometest.com/api",
                "entity": "Location",
                "geometryType": "multipoint",
                "authcfg": "abc",
                "pageSize": 20,
            },
        )
        # should be forgiving to entity vs entity set strings
        uri = (
            "url='https://sometest.com/api' type=PointZ authcfg='abc' entity='Location'"
        )
        parts = QgsProviderRegistry.instance().decodeUri("sensorthings", uri)
        self.assertEqual(
            parts,
            {
                "url": "https://sometest.com/api",
                "entity": "Location",
                "geometryType": "point",
                "authcfg": "abc",
            },
        )

        uri = "url='https://sometest.com/api' type=MultiLineStringZ authcfg='abc' entity='Location'"
        parts = QgsProviderRegistry.instance().decodeUri("sensorthings", uri)
        self.assertEqual(
            parts,
            {
                "url": "https://sometest.com/api",
                "entity": "Location",
                "geometryType": "line",
                "authcfg": "abc",
            },
        )

        uri = "url='https://sometest.com/api' type=MultiPolygonZ authcfg='abc' entity='Location'"
        parts = QgsProviderRegistry.instance().decodeUri("sensorthings", uri)
        self.assertEqual(
            parts,
            {
                "url": "https://sometest.com/api",
                "entity": "Location",
                "geometryType": "polygon",
                "authcfg": "abc",
            },
        )

        uri = "url='https://sometest.com/api' bbox='1,2,3,4' type=MultiPolygonZ authcfg='abc' entity='Location'"
        parts = QgsProviderRegistry.instance().decodeUri("sensorthings", uri)
        self.assertEqual(
            parts,
            {
                "url": "https://sometest.com/api",
                "entity": "Location",
                "geometryType": "polygon",
                "authcfg": "abc",
                "bounds": QgsRectangle(1, 2, 3, 4),
            },
        )

        uri = "url='https://sometest.com/api' type=MultiPolygonZ authcfg='abc' entity='Location' sql=name eq 'test'"
        parts = QgsProviderRegistry.instance().decodeUri("sensorthings", uri)
        self.assertEqual(
            parts,
            {
                "url": "https://sometest.com/api",
                "entity": "Location",
                "geometryType": "polygon",
                "authcfg": "abc",
                "sql": "name eq 'test'",
            },
        )

        uri = "url='https://sometest.com/api' type=MultiPolygonZ authcfg='abc' featureLimit='50' entity='Location'"
        parts = QgsProviderRegistry.instance().decodeUri("sensorthings", uri)
        self.assertEqual(
            parts,
            {
                "url": "https://sometest.com/api",
                "entity": "Location",
                "geometryType": "polygon",
                "authcfg": "abc",
                "featureLimit": 50,
            },
        )

        uri = "url='https://sometest.com/api' type=MultiPolygonZ authcfg='abc' expandTo='Thing:orderby=description,asc:limit=5;Datastream:orderby=time,asc:limit=3' entity='Location'"
        parts = QgsProviderRegistry.instance().decodeUri("sensorthings", uri)
        self.assertEqual(
            parts,
            {
                "url": "https://sometest.com/api",
                "entity": "Location",
                "geometryType": "polygon",
                "authcfg": "abc",
                "expandTo": [
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.Thing, orderBy="description", limit=5
                    ),
                    QgsSensorThingsExpansionDefinition(
                        Qgis.SensorThingsEntity.Datastream, orderBy="time", limit=3
                    ),
                ],
            },
        )

    def testEncodeUri(self):
        """
        Test encoding a SensorThings uri
        """
        parts = {
            "url": "http://blah.com",
            "authcfg": "aaaaa",
            "entity": "locations",
            "geometryType": "multipoint",
            "pageSize": 20,
        }
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(
            uri,
            "authcfg=aaaaa type=MultiPointZ entity='Location' pageSize='20' url='http://blah.com'",
        )

        # should be forgiving to entity vs entity set strings
        parts = {
            "url": "http://blah.com",
            "authcfg": "aaaaa",
            "entity": "location",
            "geometryType": "point",
        }
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(
            uri, "authcfg=aaaaa type=PointZ entity='Location' url='http://blah.com'"
        )

        parts = {
            "url": "http://blah.com",
            "authcfg": "aaaaa",
            "entity": "location",
            "geometryType": "line",
        }
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(
            uri,
            "authcfg=aaaaa type=MultiLineStringZ entity='Location' url='http://blah.com'",
        )

        parts = {
            "url": "http://blah.com",
            "authcfg": "aaaaa",
            "entity": "location",
            "geometryType": "polygon",
        }
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(
            uri,
            "authcfg=aaaaa type=MultiPolygonZ entity='Location' url='http://blah.com'",
        )

        parts = {
            "url": "http://blah.com",
            "authcfg": "aaaaa",
            "entity": "location",
            "geometryType": "polygon",
            "bounds": QgsRectangle(1, 2, 3, 4),
        }
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(
            uri,
            "authcfg=aaaaa type=MultiPolygonZ bbox='1,2,3,4' entity='Location' url='http://blah.com'",
        )

        parts = {
            "url": "http://blah.com",
            "authcfg": "aaaaa",
            "entity": "location",
            "geometryType": "polygon",
            "sql": "name eq 'test'",
        }
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(
            uri,
            "authcfg=aaaaa type=MultiPolygonZ entity='Location' url='http://blah.com' sql=name eq 'test'",
        )

        parts = {
            "url": "http://blah.com",
            "authcfg": "aaaaa",
            "entity": "location",
            "geometryType": "polygon",
            "featureLimit": 50,
        }
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(
            uri,
            "authcfg=aaaaa type=MultiPolygonZ entity='Location' featureLimit='50' url='http://blah.com'",
        )

        parts = {
            "url": "http://blah.com",
            "authcfg": "aaaaa",
            "entity": "location",
            "geometryType": "polygon",
            "expandTo": [
                QgsSensorThingsExpansionDefinition(
                    Qgis.SensorThingsEntity.Thing, orderBy="description", limit=5
                ),
                QgsSensorThingsExpansionDefinition(
                    Qgis.SensorThingsEntity.Datastream, orderBy="time", limit=3
                ),
            ],
        }
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(
            uri,
            "authcfg=aaaaa type=MultiPolygonZ entity='Location' expandTo='Thing:orderby=description,asc:limit=5;Datastream:orderby=time,asc:limit=3' url='http://blah.com'",
        )


if __name__ == "__main__":
    unittest.main()
