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

from qgis.PyQt.QtCore import QVariant, QCoreApplication
from qgis.core import (
    Qgis,
    QgsProviderRegistry,
    QgsVectorLayer,
    QgsSettings,
    QgsSensorThingsUtils,
)
from qgis.testing import start_app, QgisTestCase


def sanitize(endpoint, x):
    if x.startswith("/query"):
        x = x[len("/query"):]
        endpoint = endpoint + "_query"

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
        endpoint = cls.basetestpath + "/fake_qgis_http_endpoint"

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        QgsSettings().clear()
        # shutil.rmtree(cls.basetestpath, True)
        cls.vl = (
            None  # so as to properly close the provider and remove any temporary file
        )
        super().tearDownClass()

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

    def test_invalid_layer(self):
        vl = QgsVectorLayer(
            f"url='http://fake.com/fake_qgis_http_endpoint'", "test", "sensorthings"
        )
        self.assertFalse(vl.isValid())
        self.assertEqual(
            vl.dataProvider().error().summary()[:32], "Connection failed: Error opening"
        )
        self.assertEqual(
            vl.dataProvider().error().summary()[-25:], "No such file or directory"
        )

    def test_layer_invalid_json(self):
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
                    sanitize(endpoint, "/Locations?$top=0&$count=true"),
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
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
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

            features = list(vl.getFeatures())
            self.assertEqual([f.id() for f in features], [0, 1, 2])
            self.assertEqual([f["id"] for f in features], ['1', '2', '3'])
            self.assertEqual([f["selfLink"][-10:] for f in features], ['/Things(1)', '/Things(2)', '/Things(3)'])
            self.assertEqual([f["name"] for f in features], ['Thing 1', 'Thing 2', 'Thing 3'])
            self.assertEqual([f["description"] for f in features], ['Desc 1', 'Desc 2', 'Desc 3'])
            # TODO!
            self.assertEqual([f["properties"] for f in features], [None, None, None])

    def test_location(self):
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
                sanitize(endpoint, "/Locations?$top=0&$count=true"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":30,"value":[]}""")

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ entity='Location'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.featureCount(), 30)
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

    def test_historical_location(self):
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
                f.write("""{"@iot.count":28,"value":[]}""")

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ entity='HistoricalLocation'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 28)
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

    def test_datastream(self):
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
                f.write("""{"@iot.count":27,"value":[]}""")

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ entity='Datastream'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 27)
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

    def test_sensor(self):
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
                f.write("""{"@iot.count":27,"value":[]}""")

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ entity='Sensor'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 27)
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

    def test_observed_property(self):
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
                f.write("""{"@iot.count":27,"value":[]}""")

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ entity='ObservedProperty'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 27)
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

    def test_observation(self):
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
                f.write("""{"@iot.count":27,"value":[]}""")

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ entity='Observation'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.NoGeometry)
            self.assertEqual(vl.featureCount(), 27)
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

    def test_feature_of_interest(self):
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
                sanitize(endpoint, "/FeaturesOfInterest?$top=0&$count=true"),
                "wt",
                encoding="utf8",
            ) as f:
                f.write("""{"@iot.count":25,"value":[]}""")

            vl = QgsVectorLayer(
                f"url='http://{endpoint}' type=PointZ entity='FeatureOfInterest'",
                "test",
                "sensorthings",
            )
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.storageType(), "OGC SensorThings API")
            self.assertEqual(vl.wkbType(), Qgis.WkbType.PointZ)
            self.assertEqual(vl.featureCount(), 25)
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


if __name__ == "__main__":
    unittest.main()
