"""QGIS Unit tests for QgsPointCloudDataProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "09/11/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.core import (
    QgsPointCloudLayer,
    QgsProviderRegistry,
    QgsStatisticalSummary,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudDataProvider(QgisTestCase):

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testStatistics(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        stats = layer.dataProvider().metadataStatistics()
        self.assertEqual(stats.statisticsOf("X").count, 253)
        self.assertEqual(stats.statisticsOf("X").minimum, 498062.0)
        self.assertEqual(stats.statisticsOf("X").maximum, 498067.39)
        self.assertAlmostEqual(stats.statisticsOf("X").mean, 498064.7342292491, 5)
        self.assertAlmostEqual(stats.statisticsOf("X").stDev, 1.5636647117681046, 5)
        with self.assertRaises(AttributeError):
            stats.statisticsOf("X").majority

        self.assertEqual(stats.statisticsOf("Xxxxx").count, 0)

        self.assertEqual(stats.statisticsOf("Intensity").count, 253)
        self.assertEqual(stats.statisticsOf("Intensity").minimum, 199)
        self.assertEqual(stats.statisticsOf("Intensity").maximum, 2086.0)
        self.assertAlmostEqual(
            stats.statisticsOf("Intensity").mean, 728.521739130435, 5
        )
        self.assertAlmostEqual(
            stats.statisticsOf("Intensity").stDev, 440.9652417017358, 5
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testMetadataClasses(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        stats = layer.dataProvider().metadataStatistics()
        self.assertEqual(stats.classesOf("X"), [])
        self.assertCountEqual(stats.classesOf("Classification"), [1, 2, 3, 5])

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testMetadataClassStatistics(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        stats = layer.dataProvider().metadataStatistics()

        self.assertEqual(stats.statisticsOf("X").singleClassCount(0), -1)
        self.assertEqual(stats.statisticsOf("Classification").singleClassCount(0), -1)

        self.assertEqual(stats.statisticsOf("Classification").singleClassCount(1), 1)
        self.assertEqual(stats.statisticsOf("Classification").singleClassCount(2), 160)
        self.assertEqual(stats.statisticsOf("Classification").singleClassCount(3), 89)
        self.assertEqual(stats.statisticsOf("Classification").singleClassCount(5), 3)

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testOriginalMetadataEpt(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertEqual(layer.dataProvider().originalMetadata()["major_version"], 1.0)
        self.assertEqual(layer.dataProvider().originalMetadata()["minor_version"], 2.0)
        self.assertEqual(
            layer.dataProvider().originalMetadata()["software_id"],
            "PDAL 2.1.0 (Releas)",
        )  # spellok
        self.assertEqual(
            layer.dataProvider().originalMetadata()["creation_year"], 2020.0
        )
        self.assertEqual(layer.dataProvider().originalMetadata()["creation_doy"], 309.0)

    @unittest.skipIf(
        "pdal" not in QgsProviderRegistry.instance().providerList(),
        "PDAL provider not available",
    )
    def testOriginalMetadataPdal(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/las/cloud.las", "test", "pdal"
        )
        self.assertEqual(layer.dataProvider().originalMetadata()["major_version"], 1.0)
        self.assertEqual(layer.dataProvider().originalMetadata()["minor_version"], 2.0)
        self.assertEqual(
            layer.dataProvider().originalMetadata()["software_id"],
            "PDAL 2.1.0 (Releas)",
        )  # spellok
        self.assertEqual(
            layer.dataProvider().originalMetadata()["creation_year"], 2020.0
        )
        self.assertEqual(layer.dataProvider().originalMetadata()["creation_doy"], 309.0)


if __name__ == "__main__":
    unittest.main()
