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

from qgis.PyQt.QtCore import QVariant, QCoreApplication, QDateTime, Qt, QDate, QTime
from qgis.core import (
    Qgis,
    QgsProviderRegistry,
    QgsVectorLayer,
    QgsSettings,
    QgsSensorThingsUtils,
    QgsFeatureRequest,
    QgsRectangle
)
from qgis.testing import start_app, QgisTestCase


def sanitize(endpoint, x):
    for prefix in ('/Locations',
                   '/HistoricalLocations',
                   '/Things',
                   '/FeaturesOfInterest',
                   '/MultiDatastreams'):
        if x.startswith(prefix):
            x = x[len(prefix):]
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
    ).replace(
        "\n", "_"
    ).replace(
        "$", "_"
    )


class TestPyQgsSensorThingsProvider(QgisTestCase):  # , ProviderTestCase):
    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super(TestPyQgsSensorThingsProvider, cls).setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("TestPyQgsSensorThingsProvider.com")
        QCoreApplication.setApplicationName("TestPyQgsSensorThingsProvider")
        QgsSettings().clear()
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
            QgsSensorThingsUtils.filterForWkbType(Qgis.SensorThingsEntity.Location, Qgis.WkbType.Point),
            "location/type eq 'Point' or location/geometry/type eq 'Point'"
        )
        self.assertEqual(
            QgsSensorThingsUtils.filterForWkbType(Qgis.SensorThingsEntity.Location, Qgis.WkbType.PointZ),
            "location/type eq 'Point' or location/geometry/type eq 'Point'"
        )
        self.assertEqual(
            QgsSensorThingsUtils.filterForWkbType(Qgis.SensorThingsEntity.FeatureOfInterest, Qgis.WkbType.Polygon),
            "feature/type eq 'Polygon' or feature/geometry/type eq 'Polygon'"
        )
        self.assertEqual(
            QgsSensorThingsUtils.filterForWkbType(Qgis.SensorThingsEntity.Location, Qgis.WkbType.LineString),
            "location/type eq 'LineString' or location/geometry/type eq 'LineString'"
        )
        self.assertEqual(
            QgsSensorThingsUtils.filterForWkbType(Qgis.SensorThingsEntity.MultiDatastream, Qgis.WkbType.Polygon),
            "observedArea/type eq 'Polygon' or observedArea/geometry/type eq 'Polygon'"
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
            QgsSensorThingsUtils.entitySetStringToEntity(" MultidataStreams "),
            Qgis.SensorThingsEntity.MultiDatastream,
        )

    def test_filter_for_extent(self):
        """
        Test constructing valid filter strings for features which intersect
        an extent
        """
        self.assertFalse(QgsSensorThingsUtils.filterForExtent('', QgsRectangle()))
        self.assertFalse(QgsSensorThingsUtils.filterForExtent('test', QgsRectangle()))
        self.assertFalse(QgsSensorThingsUtils.filterForExtent('', QgsRectangle(1, 2, 3, 4)))
        self.assertEqual(QgsSensorThingsUtils.filterForExtent('test', QgsRectangle(1, 2, 3, 4)),
                         "geo.intersects(test, geography'POLYGON((1 2, 3 2, 3 4, 1 4, 1 2))')")

    def test_combine_filters(self):
        """
        Test combining multiple filter strings into one
        """
        self.assertFalse(QgsSensorThingsUtils.combineFilters([]))
        self.assertFalse(QgsSensorThingsUtils.combineFilters(['']))
        self.assertEqual(QgsSensorThingsUtils.combineFilters(['', 'a eq 1']), 'a eq 1')
        self.assertEqual(QgsSensorThingsUtils.combineFilters(['a eq 1', 'b eq 2']), '(a eq 1) and (b eq 2)')
        self.assertEqual(QgsSensorThingsUtils.combineFilters(['a eq 1', '', 'b eq 2', 'c eq 3']),
                         '(a eq 1) and (b eq 2) and (c eq 3)')

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
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                    "wt",
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

    def test_thing(self):
        """
        Test a layer retrieving 'Thing' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/Things?$top=0&$count=true"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/Things?$top=2&$count=false"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/Things?$top=2&$skip=2"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
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

    def test_location(self):
        """
        Test a layer retrieving 'Location' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/Locations?$top=2&$count=false&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
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

    def test_location_formalism(self):
        """
        Test https://github.com/qgis/QGIS/issues/56732
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint,
                         "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint,
                         "/Locations?$top=2&$count=false&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint,
                             "/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
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
            self.assertIn("Entity Type</td><td>Location</td>",
                          vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"',
                          vl.htmlMetadata())

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
                [f["description"] for f in features],
                ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"},
                 {"owner": "owner 3"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (11.6 52.1)", "Point (12.6 53.1)",
                 "Point (13.6 55.1)"],
            )

            # all features fetched, accurate extent should be returned
            self.assertEqual(vl.extent(),
                             QgsRectangle(11.6, 52.1, 13.6, 55.1))

    def test_filter_rect(self):
        """
        Test retrieving features using feature requests with filter
        rectangles set
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/Locations?$top=2&$count=false&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/Locations?$top=2&$skip=2&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
                    )

                with open(
                    sanitize(endpoint,
                             "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((1 0, 10 0, 10 80, 1 80, 1 0))'))"),
                    "wt",
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
            }""".replace(
                            "endpoint", "http://" + endpoint
                        )
                    )

            with open(
                sanitize(endpoint,
                         "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((10 0, 20 0, 20 80, 10 80, 10 0))'))"),
                "wt",
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
            }""".replace(
                        "endpoint", "http://" + endpoint
                    )
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
            self.assertIn("Entity Type</td><td>Location</td>",
                          vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"',
                          vl.htmlMetadata())

            # test retrieving subset of features from a filter rect only
            request = QgsFeatureRequest()
            request.setFilterRect(
                QgsRectangle(1, 0, 10, 80)
            )

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
            self.assertEqual(
                [f["description"] for f in features],
                ["Desc 1", "Desc 3"]
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"},
                 {"owner": "owner 3"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (1.6 52.1)",
                 "Point (3.6 55.1)"],
            )

            # test retrieving a different subset with a different extent
            request = QgsFeatureRequest()
            request.setFilterRect(
                QgsRectangle(10, 0, 20, 80)
            )

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
            self.assertEqual(
                [f["description"] for f in features],
                ["Desc 2"]
            )
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
            request.setFilterRect(
                QgsRectangle(0, 0, 20, 80)
            )

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
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint,
                         "/Locations?$top=0&$count=true&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((1 0, 10 0, 10 80, 1 80, 1 0))'))"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":2,"value":[]}""")

            with open(
                sanitize(endpoint,
                         "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((1 0, 10 0, 10 80, 1 80, 1 0))'))"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint,
                         "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((1 0, 3 0, 3 50, 1 50, 1 0))'))"),
                "wt",
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
            }""".replace(
                        "endpoint", "http://" + endpoint
                    )
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
            self.assertIn("Entity Type</td><td>Location</td>",
                          vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"',
                          vl.htmlMetadata())

            # test retrieving a subset of the features from the layer,
            # using a filter rect which only covers a part of the hardcoded
            # provider's extent
            request = QgsFeatureRequest()
            request.setFilterRect(
                QgsRectangle(1, 0, 3, 50)
            )

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
            self.assertEqual(
                [f["description"] for f in features],
                ["Desc 1"]
            )
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
            self.assertEqual(
                [f["description"] for f in features],
                ["Desc 1", "Desc 3"]
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"},
                 {"owner": "owner 3"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (1.6 52.1)",
                 "Point (3.6 55.1)"],
            )

            # should have accurate layer extent now
            self.assertEqual(vl.extent(), QgsRectangle(1.62337299999999995, 52.13201699999999761, 3.62337299999999995,
                                                       55.13201699999999761))

    def test_subset_string(self):
        """
        Test a layer with a hardcoded user-defined filter string
        at the provider level
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint,
                         "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":2,"value":[]}""")

            with open(
                sanitize(endpoint,
                         "/Locations?$top=0&$count=true&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (name eq 'Location 1')"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":1,"value":[]}""")

            with open(
                sanitize(endpoint,
                         "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((1 0, 3 0, 3 50, 1 50, 1 0))')) and (name eq 'Location 1')"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint,
                         "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((1 0, 3 0, 3 50, 1 50, 1 0))'))"),
                "wt",
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
            }""".replace(
                        "endpoint", "http://" + endpoint
                    )
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
            self.assertEqual(vl.source(), f" type=PointZ entity='Location' pageSize='2' url='http://{endpoint}' sql=name eq 'Location 1'")
            self.assertEqual(vl.featureCount(), 1)

            self.assertEqual(vl.crs().authid(), "EPSG:4326")
            self.assertIn("Entity Type</td><td>Location</td>",
                          vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/Locations"',
                          vl.htmlMetadata())

            # test retrieving a subset of features, using a request which
            # must be combined with the layer's subset filter
            request = QgsFeatureRequest()
            request.setFilterRect(
                QgsRectangle(1, 0, 3, 50)
            )

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
            self.assertEqual(
                [f["description"] for f in features],
                ["Desc 1"]
            )
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
            self.assertEqual(
                [f["description"] for f in features],
                ["Desc 1"]
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ["Point (1.6 52.1)"],
            )

            # should have accurate layer extent now
            self.assertEqual(vl.extent(), QgsRectangle(1.62337299999999995,
                                                       52.13201699999999761,
                                                       1.62337299999999995,
                                                       52.13201699999999761))

    def test_feature_limit(self):
        """
        Test a layer with a hardcoded maximum number of features to retrieve
        from the service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint,
                         "/Locations?$top=0&$count=true&$filter=location/type eq 'Point' or location/geometry/type eq 'Point'"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint,
                         "/Locations?$top=0&$count=true&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (name eq 'Location 1')"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":1,"value":[]}""")

            with open(
                sanitize(endpoint,
                         "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((1 0, 3 0, 3 50, 1 50, 1 0))'))"),
                "wt",
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
            }""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint,
                         "/Locations?$top=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((0 0, 100 0, 100 150, 0 150, 0 0))'))"),
                "wt",
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
  "@iot.nextLink": "endpoint/Locations?$top=2&$skip=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((0 0, 100 0, 100 150, 0 150, 0 0))'))"
}
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            # Note -- top param here should be replaced by "top=1", NOT be the "top=2" parameter from the previous page's iot.nextLink url!
            with open(
                sanitize(endpoint,
                         "/Locations?$top=1&$skip=2&$count=false&$filter=(location/type eq 'Point' or location/geometry/type eq 'Point') and (geo.intersects(location, geography'POLYGON((0 0, 100 0, 100 150, 0 150, 0 0))'))"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
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
            request.setFilterRect(
                QgsRectangle(1, 0, 3, 50)
            )

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
            self.assertEqual(
                [f["description"] for f in features],
                ["Desc 1"]
            )
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
            request.setFilterRect(
                QgsRectangle(0, 0, 100, 150)
            )
            features = list(vl.getFeatures(request))
            self.assertEqual([f["id"] for f in features], ['1', '2', '3'])
            self.assertEqual(
                [f["selfLink"][-13:] for f in features],
                ["/Locations(1)", "/Locations(2)", "/Locations(3)"],
            )

            # should have accurate layer extent now
            self.assertEqual(vl.extent(), QgsRectangle(1.62337299999999995,
                                                       52,
                                                       82,
                                                       53))

    def test_historical_location(self):
        """
        Test a layer retrieving 'Historical Location' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/HistoricalLocations?$top=0&$count=true"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/HistoricalLocations?$top=2&$count=false"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/HistoricalLocations?$top=2&$skip=2"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
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
                    QDateTime(QDate(2020, 3, 20), QTime(16, 35, 23, 384), Qt.TimeSpec(1)),
                    QDateTime(QDate(2021, 3, 20), QTime(16, 35, 23, 384), Qt.TimeSpec(1)),
                    QDateTime(QDate(2022, 3, 20), QTime(16, 35, 23, 384), Qt.TimeSpec(1)),
                ],
            )

    def test_datastream(self):
        """
        Test a layer retrieving 'Datastream' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/Datastreams?$top=0&$count=true"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/Datastreams?$top=2&$count=false"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/Datastreams?$top=2&$skip=2"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
                    )

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' pageSize=2 type=PointZ entity='Datastream'",
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

    def test_sensor(self):
        """
        Test a layer retrieving 'Sensor' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/Sensors?$top=0&$count=true"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/Sensors?$top=2&$count=false"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/Sensors?$top=2&$skip=2"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
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

    def test_observed_property(self):
        """
        Test a layer retrieving 'Observed Property' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/ObservedProperties?$top=0&$count=true"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/ObservedProperties?$top=2&$count=false"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/ObservedProperties?$top=2&$skip=2"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
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
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/Observations?$top=0&$count=true"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/Observations?$top=2&$count=false"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/Observations?$top=2&$skip=2"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
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

    def test_feature_of_interest(self):
        """
        Test a layer retrieving 'Features of Interest' entities from a service
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/FeaturesOfInterest?$top=0&$count=true&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/FeaturesOfInterest?$top=2&$count=false&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/FeaturesOfInterest?$top=2&$skip=2&$filter=feature/type eq 'Point' or feature/geometry/type eq 'Point'"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
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
                ["/FeaturesOfInterest(1)", "/FeaturesOfInterest(2)", "/FeaturesOfInterest(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ['SAM.09.LAA.822.7.1', 'SAM.09.LOB.823.7.1', 'SAM.09.LOB.824.1.1'],
            )
            self.assertEqual(
                [f["description"] for f in features],
                ['Air quality sample SAM.09.LAA.822.7.1', None, 'Air quality sample SAM.09.LOB.824.1.1']
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{'localId': 'SAM.09.LAA.822.7.1',
                  'metadata': 'http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample',
                  'namespace': 'AT.0008.20.AQ', 'owner': 'http://luft.umweltbundesamt.at'},
                 {'localId': 'SAM.09.LOB.823.7.1',
                  'metadata': 'http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample',
                  'namespace': 'AT.0008.20.AQ', 'owner': 'http://luft.umweltbundesamt.at'},
                 {'localId': 'SAM.09.LOB.824.1.1',
                  'metadata': 'http://luft.umweltbundesamt.at/inspire/wfs?service=WFS&version=2.0.0&request=GetFeature&typeName=aqd:AQD_Sample',
                  'namespace': 'AT.0008.20.AQ', 'owner': 'http://luft.umweltbundesamt.at'}],
            )

            self.assertEqual(
                [f.geometry().asWkt(1) for f in features],
                ['Point (16.4 48.2)', 'Point (16.5 48.2)', 'Point (16.5 48.2)'],
            )

    def test_multidatastream_no_geometry(self):
        """
        Test a layer retrieving 'MultiDatastream' entities from a service without geometry
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/MultiDatastreams?$top=0&$count=true"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/MultiDatastreams?$top=2&$count=false"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/MultiDatastreams?$top=2&$skip=2"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
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
            self.assertIn("Entity Type</td><td>MultiDatastream</td>",
                          vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/MultiDatastreams"',
                          vl.htmlMetadata())

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
                ["/MultiDatastreams(1)", "/MultiDatastreams(2)", "/MultiDatastreams(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["MultiDatastream 1", "MultiDatastream 2", "MultiDatastream 3"],
            )
            self.assertEqual(
                [f["description"] for f in features],
                ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["unitOfMeasurements"] for f in features],
                [
                    [{
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    }],
                    [{
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    }],
                    [{
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    }],
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
                    ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
                    ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
                    ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 0, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 0, 0, 0),
                              Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2018, 1, 12), QTime(4, 0, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2019, 1, 12), QTime(4, 0, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2021, 1, 12), QTime(4, 0, 0, 0),
                              Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 30, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 30, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 30, 0, 0),
                              Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 31, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 31, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 31, 0, 0),
                              Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"},
                 {"owner": "owner 3"}],
            )

    def test_multidatastream_polygons(self):
        """
        Test a layer retrieving 'MultiDatastream' entities from a service using polygons
        """
        with tempfile.TemporaryDirectory() as temp_dir:
            base_path = temp_dir.replace("\\", "/")
            endpoint = base_path + "/fake_qgis_http_endpoint"
            with open(sanitize(endpoint, ""), "wt", encoding="utf8") as f:
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
}""".replace(
                        "endpoint", "http://" + endpoint
                    )
                )

            with open(
                sanitize(endpoint, "/MultiDatastreams?$top=0&$count=true&$filter=observedArea/type eq 'Polygon' or observedArea/geometry/type eq 'Polygon'"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":3,"value":[]}""")

            with open(
                sanitize(endpoint, "/MultiDatastreams?$top=2&$count=false&$filter=observedArea/type eq 'Polygon' or observedArea/geometry/type eq 'Polygon'"),
                "wt",
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
                """.replace(
                        "endpoint", "http://" + endpoint
                    )
                )

                with open(
                    sanitize(endpoint, "/MultiDatastreams?$top=2&$skip=2&$filter=observedArea/type eq 'Polygon' or observedArea/geometry/type eq 'Polygon'"),
                    "wt",
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
                            """.replace(
                            "endpoint", "http://" + endpoint
                        )
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
            self.assertEqual(vl.crs().authid(), 'EPSG:4326')
            self.assertIn("Entity Type</td><td>MultiDatastream</td>",
                          vl.htmlMetadata())
            self.assertIn(f'href="http://{endpoint}/MultiDatastreams"',
                          vl.htmlMetadata())

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
                ["/MultiDatastreams(1)", "/MultiDatastreams(2)", "/MultiDatastreams(3)"],
            )
            self.assertEqual(
                [f["name"] for f in features],
                ["MultiDatastream 1", "MultiDatastream 2", "MultiDatastream 3"],
            )
            self.assertEqual(
                [f["description"] for f in features],
                ["Desc 1", "Desc 2", "Desc 3"]
            )
            self.assertEqual(
                [f["unitOfMeasurements"] for f in features],
                [
                    [{
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    }],
                    [{
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    }],
                    [{
                        "definition": "http://dd.eionet.europa.eu/vocabulary/uom/concentration/ug.m-3",
                        "name": "ug.m-3",
                        "symbol": "ug.m-3",
                    }],
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
                    ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
                    ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
                    ["http://www.opengis.net/def/observationType/OGC-OM/2.0/OM_Measurement"],
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 0, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 0, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 0, 0, 0),
                              Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["phenomenonTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2018, 1, 12), QTime(4, 0, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2019, 1, 12), QTime(4, 0, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2021, 1, 12), QTime(4, 0, 0, 0),
                              Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeStart"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 30, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 30, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 30, 0, 0),
                              Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["resultTimeEnd"] for f in features],
                [
                    QDateTime(QDate(2017, 12, 31), QTime(23, 31, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2018, 12, 31), QTime(23, 31, 0, 0),
                              Qt.TimeSpec(1)),
                    QDateTime(QDate(2020, 12, 31), QTime(23, 31, 0, 0),
                              Qt.TimeSpec(1)),
                ],
            )
            self.assertEqual(
                [f["properties"] for f in features],
                [{"owner": "owner 1"}, {"owner": "owner 2"},
                 {"owner": "owner 3"}],
            )
            self.assertEqual(
                [f.geometry().asWkt() for f in features],
                ['Polygon ((100 0, 101 0, 101 1, 100 1, 100 0))',
                 'Polygon ((102 0, 103 0, 103 1, 102 1, 102 0))',
                 'Polygon ((103 0, 104 0, 104 1, 103 1, 103 0))'],
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
                "bounds": QgsRectangle(1, 2, 3, 4)
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
                "sql": "name eq 'test'"
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
                "featureLimit": 50
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
            "bounds": QgsRectangle(1, 2, 3, 4)
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
            "sql": "name eq 'test'"
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
            "featureLimit": 50
        }
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(
            uri,
            "authcfg=aaaaa type=MultiPolygonZ entity='Location' featureLimit='50' url='http://blah.com'",
        )


if __name__ == "__main__":
    unittest.main()
