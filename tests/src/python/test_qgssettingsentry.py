# -*- coding: utf-8 -*-
"""
Test the QgsSettingsEntry classes

Run with: ctest -V -R PyQgsSettingsEntry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile
from qgis.core import QgsSettings, QgsSettingsEntryBase, QgsSettingsEntryVariant, QgsSettingsEntryString, QgsSettingsEntryStringList, QgsSettingsEntryBool, QgsSettingsEntryInteger, QgsSettingsEntryDouble, QgsTolerance, QgsMapLayerProxyModel
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QSettings, QVariant
from pathlib import Path

__author__ = 'Damiano Lombardi'
__date__ = '02/04/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'


start_app()


class TestQgsSettingsEntry(unittest.TestCase):

    cnt = 0

    def setUp(self):
        self.pluginName = "UnitTest"

    def tearDown(self):
        pass

    def test_settings_entry_base(self):
        settingsKey = "settingsEntryBase/variantValue"
        settingsKeyComplete = self.pluginName + "/" + settingsKey

        # Make sure settings does not exists
        QgsSettings().remove(settingsKeyComplete, QgsSettings.Plugins)

        defaultValue = 42
        description = "Variant value for basic functionality test"
        settingsEntryVariant = QgsSettingsEntryVariant(settingsKey, self.pluginName, defaultValue, description)

        # Check key
        self.assertEqual(settingsEntryVariant.key(), settingsKeyComplete)

        # Passing dynamicKeyPart to a non dynamic settings has no effect
        self.assertEqual(settingsEntryVariant.key("gugus"), settingsKeyComplete)

        self.assertEqual(settingsEntryVariant.hasDynamicKey(), False)

        # At this point settings should still not exists in underlyng QSettings as it was still not written (setValue)
        self.assertEqual(settingsEntryVariant.exists(), False)
        settingsEntryVariant.setValue(43)
        self.assertEqual(settingsEntryVariant.exists(), True)
        settingsEntryVariant.remove()
        self.assertEqual(settingsEntryVariant.exists(), False)

        # Section
        self.assertEqual(settingsEntryVariant.section(), QgsSettings.Plugins)

        # DefaultValue
        self.assertEqual(settingsEntryVariant.defaultValueAsVariant(), defaultValue)

        # Set/Get value
        # as settings still does not exists return default value
        self.assertEqual(settingsEntryVariant.valueAsVariant(), defaultValue)
        settingsEntryVariant.setValue(43)
        # Verify setValue using QgsSettings
        self.assertEqual(QgsSettings().value(settingsKeyComplete, defaultValue, section=QgsSettings.Plugins), 43)
        self.assertEqual(settingsEntryVariant.valueAsVariant(), 43)

        # Settings type
        self.assertEqual(settingsEntryVariant.settingsType(), QgsSettingsEntryBase.Variant)

        # Description
        self.assertEqual(settingsEntryVariant.description(), description)

    def test_settings_entry_base_dynamic_key(self):
        settingsKeyDynamic = "settingsEntryBase/%/variantValue"
        dynamicKeyPart1 = "first"
        dynamicKeyPart2 = "second"
        settingsKeyComplete1 = self.pluginName + "/" + settingsKeyDynamic.replace("%", dynamicKeyPart1)
        settingsKeyComplete2 = self.pluginName + "/" + settingsKeyDynamic.replace("%", dynamicKeyPart2)

        # Make sure settings does not exists
        QgsSettings().remove(settingsKeyComplete1, QgsSettings.Plugins)
        QgsSettings().remove(settingsKeyComplete2, QgsSettings.Plugins)

        defaultValue = 42
        settingsEntryVariantDynamic = QgsSettingsEntryVariant(settingsKeyDynamic, self.pluginName, defaultValue, "Variant value for dynamic key functionality test")

        # Check key
        self.assertEqual(settingsEntryVariantDynamic.key(dynamicKeyPart1), settingsKeyComplete1)
        self.assertEqual(settingsEntryVariantDynamic.key(dynamicKeyPart2), settingsKeyComplete2)

        # Get set values
        settingsEntryVariantDynamic.setValue(43, dynamicKeyPart1)
        settingsEntryVariantDynamic.setValue(44, dynamicKeyPart2)
        self.assertEqual(QgsSettings().value(settingsKeyComplete1, defaultValue, section=QgsSettings.Plugins), 43)
        self.assertEqual(QgsSettings().value(settingsKeyComplete2, defaultValue, section=QgsSettings.Plugins), 44)
        self.assertEqual(settingsEntryVariantDynamic.value(dynamicKeyPart1), 43)
        self.assertEqual(settingsEntryVariantDynamic.value(dynamicKeyPart2), 44)

    def test_settings_entry_variant(self):
        settingsKey = "settingsEntryVariant/variantValue"
        settingsKeyComplete = self.pluginName + "/" + settingsKey

        # Make sure settings does not exists
        QgsSettings().remove(settingsKeyComplete, QgsSettings.Plugins)

        defaultValue = 42
        description = "Variant value functionality test"
        settingsEntryVariant = QgsSettingsEntryVariant(settingsKey, self.pluginName, defaultValue, description)

        # Set/Get value
        # as settings still does not exists return default value
        self.assertEqual(settingsEntryVariant.valueAsVariant(), defaultValue)
        settingsEntryVariant.setValue("abc")
        # Verify setValue using QgsSettings
        self.assertEqual(QgsSettings().value(settingsKeyComplete, defaultValue, section=QgsSettings.Plugins), "abc")
        self.assertEqual(settingsEntryVariant.valueAsVariant(), "abc")

        # Settings type
        self.assertEqual(settingsEntryVariant.settingsType(), QgsSettingsEntryBase.Variant)

    def test_settings_entry_string(self):
        settingsKey = "settingsEntryString/stringValue"
        settingsKeyComplete = self.pluginName + "/" + settingsKey

        # Make sure settings does not exists
        QgsSettings().remove(settingsKeyComplete, QgsSettings.Plugins)

        defaultValue = "abc"
        description = "String value functionality test"
        settingsEntryString = QgsSettingsEntryString(settingsKey, self.pluginName, defaultValue, description)

        # Set/Get value
        # as settings still does not exists return default value
        self.assertEqual(settingsEntryString.valueAsVariant(), defaultValue)
        settingsEntryString.setValue("xyz")
        # Verify setValue using QgsSettings
        self.assertEqual(QgsSettings().value(settingsKeyComplete, defaultValue, section=QgsSettings.Plugins), "xyz")
        self.assertEqual(settingsEntryString.valueAsVariant(), "xyz")

        # Settings type
        self.assertEqual(settingsEntryString.settingsType(), QgsSettingsEntryBase.String)

    def test_settings_entry_stringlist(self):
        settingsKey = "settingsEntryStringList/stringListValue"
        settingsKeyComplete = self.pluginName + "/" + settingsKey

        # Make sure settings does not exists
        QgsSettings().remove(settingsKeyComplete, QgsSettings.Plugins)

        defaultValue = ["abc", "def"]
        description = "String list value functionality test"
        settingsEntryStringList = QgsSettingsEntryStringList(settingsKey, self.pluginName, defaultValue, description)

        # Set/Get value
        # as settings still does not exists return default value
        self.assertEqual(settingsEntryStringList.valueAsVariant(), defaultValue)
        settingsEntryStringList.setValue(["uvw", "xyz"])
        # Verify setValue using QgsSettings
        self.assertEqual(QgsSettings().value(settingsKeyComplete, defaultValue, section=QgsSettings.Plugins), ["uvw", "xyz"])
        self.assertEqual(settingsEntryStringList.valueAsVariant(), ["uvw", "xyz"])

        # Settings type
        self.assertEqual(settingsEntryStringList.settingsType(), QgsSettingsEntryBase.StringList)

    def test_settings_entry_bool(self):
        settingsKey = "settingsEntryBool/boolValue"
        settingsKeyComplete = self.pluginName + "/" + settingsKey

        # Make sure settings does not exists
        QgsSettings().remove(settingsKeyComplete, QgsSettings.Plugins)

        defaultValue = False
        description = "Bool value functionality test"
        settingsEntryBool = QgsSettingsEntryBool(settingsKey, self.pluginName, defaultValue, description)

        # Set/Get value
        # as settings still does not exists return default value
        self.assertEqual(settingsEntryBool.valueAsVariant(), defaultValue)
        settingsEntryBool.setValue(True)
        # Verify setValue using QgsSettings
        self.assertEqual(QgsSettings().value(settingsKeyComplete, defaultValue, section=QgsSettings.Plugins), True)
        self.assertEqual(settingsEntryBool.valueAsVariant(), True)

        # Settings type
        self.assertEqual(settingsEntryBool.settingsType(), QgsSettingsEntryBase.Bool)

    def test_settings_entry_integer(self):
        settingsKey = "settingsEntryInteger/integerValue"
        settingsKeyComplete = self.pluginName + "/" + settingsKey

        # Make sure settings does not exists
        QgsSettings().remove(settingsKeyComplete, QgsSettings.Plugins)

        defaultValue = 42
        description = "Integer value functionality test"
        settingsEntryInteger = QgsSettingsEntryInteger(settingsKey, self.pluginName, defaultValue, description)

        # Set/Get value
        # as settings still does not exists return default value
        self.assertEqual(settingsEntryInteger.valueAsVariant(), defaultValue)
        settingsEntryInteger.setValue(43)
        # Verify setValue using QgsSettings
        self.assertEqual(QgsSettings().value(settingsKeyComplete, defaultValue, section=QgsSettings.Plugins), 43)
        self.assertEqual(settingsEntryInteger.valueAsVariant(), 43)

        # Settings type
        self.assertEqual(settingsEntryInteger.settingsType(), QgsSettingsEntryBase.Integer)

    def test_settings_entry_double(self):
        settingsKey = "settingsEntryDouble/doubleValue"
        settingsKeyComplete = self.pluginName + "/" + settingsKey

        # Make sure settings does not exists
        QgsSettings().remove(settingsKeyComplete, QgsSettings.Plugins)

        defaultValue = 3.14
        description = "Double value functionality test"
        settingsEntryDouble = QgsSettingsEntryDouble(settingsKey, self.pluginName, defaultValue, description)

        # Set/Get value
        # as settings still does not exists return default value
        self.assertEqual(settingsEntryDouble.valueAsVariant(), defaultValue)
        settingsEntryDouble.setValue(1.618)
        # Verify setValue using QgsSettings
        self.assertEqual(QgsSettings().value(settingsKeyComplete, defaultValue, section=QgsSettings.Plugins), 1.618)
        self.assertEqual(settingsEntryDouble.valueAsVariant(), 1.618)

        # Settings type
        self.assertEqual(settingsEntryDouble.settingsType(), QgsSettingsEntryBase.Double)

    def test_settings_entry_enum(self):
        # Todo implement QgsSettingsEntryEnum for python
        pass

    def test_settings_entry_flag(self):
        # Todo implement QgsSettingsEntryFlag for python
        pass


if __name__ == '__main__':
    unittest.main()
