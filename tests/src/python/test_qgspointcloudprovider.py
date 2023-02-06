# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPointCloudDataProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import QDir
from qgis.core import (
    QgsProviderRegistry,
    QgsPointCloudLayer,
    QgsStatisticalSummary
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudDataProvider(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.report = "<h1>Python QgsPointCloudDataProvider Tests</h1>\n"

    @classmethod
    def tearDownClass(cls):
        report_file_path = "%s/qgistest.html" % QDir.tempPath()
        with open(report_file_path, 'a') as report_file:
            report_file.write(cls.report)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testStatistics(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        self.assertEqual(layer.dataProvider().metadataStatistic('X', QgsStatisticalSummary.Count), 253)
        self.assertEqual(layer.dataProvider().metadataStatistic('X', QgsStatisticalSummary.Min), 498062.0)
        self.assertEqual(layer.dataProvider().metadataStatistic('X', QgsStatisticalSummary.Max), 498067.39)
        self.assertAlmostEqual(layer.dataProvider().metadataStatistic('X', QgsStatisticalSummary.Range), 5.39000000001397, 5)
        self.assertAlmostEqual(layer.dataProvider().metadataStatistic('X', QgsStatisticalSummary.Mean), 498064.7342292491, 5)
        self.assertAlmostEqual(layer.dataProvider().metadataStatistic('X', QgsStatisticalSummary.StDev),
                               1.5636647117681046, 5)
        with self.assertRaises(ValueError):
            layer.dataProvider().metadataStatistic('X', QgsStatisticalSummary.Majority)

        with self.assertRaises(ValueError):
            layer.dataProvider().metadataStatistic('Xxxxx', QgsStatisticalSummary.Count)

        self.assertEqual(layer.dataProvider().metadataStatistic('Intensity', QgsStatisticalSummary.Count), 253)
        self.assertEqual(layer.dataProvider().metadataStatistic('Intensity', QgsStatisticalSummary.Min), 199)
        self.assertEqual(layer.dataProvider().metadataStatistic('Intensity', QgsStatisticalSummary.Max), 2086.0)
        self.assertAlmostEqual(layer.dataProvider().metadataStatistic('Intensity', QgsStatisticalSummary.Range), 1887.0, 5)
        self.assertAlmostEqual(layer.dataProvider().metadataStatistic('Intensity', QgsStatisticalSummary.Mean), 728.521739130435, 5)
        self.assertAlmostEqual(layer.dataProvider().metadataStatistic('Intensity', QgsStatisticalSummary.StDev),
                               440.9652417017358, 5)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testMetadataClasses(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        self.assertEqual(layer.dataProvider().metadataClasses('X'), [])
        self.assertCountEqual(layer.dataProvider().metadataClasses('Classification'), [1, 2, 3, 5])

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testMetadataClassStatistics(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())

        with self.assertRaises(ValueError):
            self.assertEqual(layer.dataProvider().metadataClassStatistic('X', 0, QgsStatisticalSummary.Count), [])

        with self.assertRaises(ValueError):
            self.assertEqual(layer.dataProvider().metadataClassStatistic('Classification', 0, QgsStatisticalSummary.Count), [])

        with self.assertRaises(ValueError):
            self.assertEqual(layer.dataProvider().metadataClassStatistic('Classification', 1, QgsStatisticalSummary.Sum), [])

        self.assertEqual(layer.dataProvider().metadataClassStatistic('Classification', 1, QgsStatisticalSummary.Count), 1)
        self.assertEqual(layer.dataProvider().metadataClassStatistic('Classification', 2, QgsStatisticalSummary.Count),
                         160)
        self.assertEqual(layer.dataProvider().metadataClassStatistic('Classification', 3, QgsStatisticalSummary.Count),
                         89)
        self.assertEqual(layer.dataProvider().metadataClassStatistic('Classification', 5, QgsStatisticalSummary.Count),
                         3)

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testOriginalMetadataEpt(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertEqual(layer.dataProvider().originalMetadata()['major_version'], 1.0)
        self.assertEqual(layer.dataProvider().originalMetadata()['minor_version'], 2.0)
        self.assertEqual(layer.dataProvider().originalMetadata()['software_id'], 'PDAL 2.1.0 (Releas)')  # spellok
        self.assertEqual(layer.dataProvider().originalMetadata()['creation_year'], 2020.0)
        self.assertEqual(layer.dataProvider().originalMetadata()['creation_doy'], 309.0)

    @unittest.skipIf('pdal' not in QgsProviderRegistry.instance().providerList(), 'PDAL provider not available')
    def testOriginalMetadataPdal(self):
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/las/cloud.las', 'test', 'pdal')
        self.assertEqual(layer.dataProvider().originalMetadata()['major_version'], 1.0)
        self.assertEqual(layer.dataProvider().originalMetadata()['minor_version'], 2.0)
        self.assertEqual(layer.dataProvider().originalMetadata()['software_id'], 'PDAL 2.1.0 (Releas)')  # spellok
        self.assertEqual(layer.dataProvider().originalMetadata()['creation_year'], 2020.0)
        self.assertEqual(layer.dataProvider().originalMetadata()['creation_doy'], 309.0)


if __name__ == '__main__':
    unittest.main()
