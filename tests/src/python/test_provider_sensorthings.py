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

from qgis.PyQt.QtCore import QCoreApplication
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
                        "endpoint", endpoint
                    )
                )

            vl = QgsVectorLayer(f"url='http://{endpoint}'", "test", "sensorthings")
            self.assertTrue(vl.isValid())
            self.assertEqual(vl.storageType(), "OGC SensorThings API")

    def testDecodeUri(self):
        """
        Test decoding a SensorThings uri
        """
        uri = "url='https://sometest.com/api' authcfg='abc' entity='Locations'"
        parts = QgsProviderRegistry.instance().decodeUri("sensorthings", uri)
        self.assertEqual(
            parts,
            {"url": "https://sometest.com/api", "entity": "Location", "authcfg": "abc"},
        )
        # should be forgiving to entity vs entity set strings
        uri = "url='https://sometest.com/api' authcfg='abc' entity='Location'"
        parts = QgsProviderRegistry.instance().decodeUri("sensorthings", uri)
        self.assertEqual(
            parts,
            {"url": "https://sometest.com/api", "entity": "Location", "authcfg": "abc"},
        )

    def testEncodeUri(self):
        """
        Test encoding a SensorThings uri
        """
        parts = {"url": "http://blah.com", "authcfg": "aaaaa", "entity": "locations"}
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(uri, "authcfg=aaaaa entity='Location' url='http://blah.com'")

        # should be forgiving to entity vs entity set strings
        parts = {"url": "http://blah.com", "authcfg": "aaaaa", "entity": "location"}
        uri = QgsProviderRegistry.instance().encodeUri("sensorthings", parts)
        self.assertEqual(uri, "authcfg=aaaaa entity='Location' url='http://blah.com'")


if __name__ == "__main__":
    unittest.main()
