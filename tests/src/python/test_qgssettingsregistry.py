"""
Test the PyQgsSettingsRegistry classes

Run with: ctest -V -R PyQgsSettingsRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import (
    QgsApplication,
    QgsSettingsEntryInteger,
    QgsSettingsRegistry,
)
import unittest
from qgis.testing import start_app, QgisTestCase

__author__ = "Damiano Lombardi"
__date__ = "18/04/2021"
__copyright__ = "Copyright 2021, The QGIS Project"


start_app()


class PyQgsSettingsRegistry(QgisTestCase):

    def setUp(self):
        self.pluginName = "UnitTestSettingsRegistry"

    def test_settings_registry(self):

        settingsEntryKey = "settingsRegistry/integerValue"
        settingsEntry = QgsSettingsEntryInteger(settingsEntryKey, self.pluginName, 123)

        settingsRegistry = QgsSettingsRegistry()
        settingsRegistry.addSettingsEntry(settingsEntry)

        # check get settings entry
        self.assertEqual(
            settingsRegistry.settingsEntry(settingsEntry.key(), False), settingsEntry
        )

        # add registry to core registry
        QgsApplication.settingsRegistryCore().addSubRegistry(settingsRegistry)

        self.assertEqual(
            QgsApplication.settingsRegistryCore().settingsEntry(
                settingsEntry.key(), True
            ),
            settingsEntry,
        )


if __name__ == "__main__":
    unittest.main()
