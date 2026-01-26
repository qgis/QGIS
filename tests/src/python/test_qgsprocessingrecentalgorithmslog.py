"""QGIS Unit tests for QgsProcessingRecentAlgorithmLog.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2018-07"
__copyright__ = "Copyright 2018, The QGIS Project"

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsSettings
from qgis.gui import QgsGui, QgsProcessingRecentAlgorithmLog
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsProcessingRecentAlgorithmLog(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(
            "QGIS_TestPyQgsNewGeoPackageLayerDialog.com"
        )
        QCoreApplication.setApplicationName("QGIS_TestPyQgsNewGeoPackageLayerDialog")
        QgsSettings().clear()

    def test_log(self):
        log = QgsProcessingRecentAlgorithmLog()
        self.assertFalse(log.recentAlgorithmIds())
        spy = QSignalSpy(log.changed)

        log.push("test")
        self.assertEqual(log.recentAlgorithmIds(), ["test"])
        self.assertEqual(len(spy), 1)
        log.push("test")
        self.assertEqual(log.recentAlgorithmIds(), ["test"])
        self.assertEqual(len(spy), 1)

        log.push("test2")
        self.assertEqual(log.recentAlgorithmIds(), ["test2", "test"])
        self.assertEqual(len(spy), 2)

        log.push("test")
        self.assertEqual(log.recentAlgorithmIds(), ["test", "test2"])
        self.assertEqual(len(spy), 3)

        log.push("test3")
        self.assertEqual(log.recentAlgorithmIds(), ["test3", "test", "test2"])
        self.assertEqual(len(spy), 4)

        log.push("test4")
        self.assertEqual(log.recentAlgorithmIds(), ["test4", "test3", "test", "test2"])
        self.assertEqual(len(spy), 5)

        log.push("test5")
        self.assertEqual(
            log.recentAlgorithmIds(), ["test5", "test4", "test3", "test", "test2"]
        )
        self.assertEqual(len(spy), 6)

        log.push("test6")
        self.assertEqual(
            log.recentAlgorithmIds(), ["test6", "test5", "test4", "test3", "test"]
        )
        self.assertEqual(len(spy), 7)

        log.push("test3")
        self.assertEqual(
            log.recentAlgorithmIds(), ["test3", "test6", "test5", "test4", "test"]
        )
        self.assertEqual(len(spy), 8)

        log.push("test3")
        self.assertEqual(
            log.recentAlgorithmIds(), ["test3", "test6", "test5", "test4", "test"]
        )
        self.assertEqual(len(spy), 8)

        # test that log has been saved to QgsSettings
        log2 = QgsProcessingRecentAlgorithmLog()
        self.assertEqual(
            log2.recentAlgorithmIds(), ["test3", "test6", "test5", "test4", "test"]
        )

    def test_gui_instance(self):
        self.assertIsNotNone(QgsGui.instance().processingRecentAlgorithmLog())


if __name__ == "__main__":
    unittest.main()
