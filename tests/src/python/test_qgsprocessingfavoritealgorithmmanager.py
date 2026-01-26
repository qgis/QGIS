"""QGIS Unit tests for QgsProcessingFavoriteAlgorithmManager.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alexander Bruy"
__date__ = "2024-02"
__copyright__ = "Copyright 2024, The QGIS Project"

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsSettings
from qgis.gui import QgsGui, QgsProcessingFavoriteAlgorithmManager
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsProcessingFavoriteAlgorithmManager(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        QgsSettings().clear()

    def test_log(self):
        log = QgsProcessingFavoriteAlgorithmManager()
        self.assertFalse(log.favoriteAlgorithmIds())
        spy = QSignalSpy(log.changed)

        log.add("test")
        self.assertEqual(log.favoriteAlgorithmIds(), ["test"])
        self.assertEqual(len(spy), 1)
        log.add("test")
        self.assertEqual(log.favoriteAlgorithmIds(), ["test"])
        self.assertEqual(len(spy), 1)

        log.add("test2")
        self.assertEqual(log.favoriteAlgorithmIds(), ["test", "test2"])
        self.assertEqual(len(spy), 2)

        log.remove("test")
        self.assertEqual(log.favoriteAlgorithmIds(), ["test2"])
        self.assertEqual(len(spy), 3)

        log.add("test")
        self.assertEqual(log.favoriteAlgorithmIds(), ["test2", "test"])
        self.assertEqual(len(spy), 4)

        log.add("test3")
        self.assertEqual(log.favoriteAlgorithmIds(), ["test2", "test", "test3"])
        self.assertEqual(len(spy), 5)

        log.add("test4")
        self.assertEqual(
            log.favoriteAlgorithmIds(), ["test2", "test", "test3", "test4"]
        )
        self.assertEqual(len(spy), 6)

        log.add("test5")
        self.assertEqual(
            log.favoriteAlgorithmIds(), ["test2", "test", "test3", "test4", "test5"]
        )
        self.assertEqual(len(spy), 7)

        log.add("test6")
        self.assertEqual(
            log.favoriteAlgorithmIds(),
            ["test2", "test", "test3", "test4", "test5", "test6"],
        )
        self.assertEqual(len(spy), 8)

        log.add("test7")
        self.assertEqual(
            log.favoriteAlgorithmIds(),
            ["test2", "test", "test3", "test4", "test5", "test6", "test7"],
        )
        self.assertEqual(len(spy), 9)

        log.add("test3")
        self.assertEqual(
            log.favoriteAlgorithmIds(),
            ["test2", "test", "test3", "test4", "test5", "test6", "test7"],
        )
        self.assertEqual(len(spy), 9)

        # test that log has been saved to QgsSettings
        log2 = QgsProcessingFavoriteAlgorithmManager()
        self.assertEqual(
            log2.favoriteAlgorithmIds(),
            ["test2", "test", "test3", "test4", "test5", "test6", "test7"],
        )

        log2.clear()
        self.assertEqual(log2.favoriteAlgorithmIds(), [])

    def test_gui_instance(self):
        self.assertIsNotNone(QgsGui.instance().processingFavoriteAlgorithmManager())


if __name__ == "__main__":
    unittest.main()
