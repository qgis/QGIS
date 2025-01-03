"""
Test the PyQgsSettingsRegistry classes

Run with: ctest -V -R PyQgsSettingsRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtWidgets import QComboBox, QSpinBox
from qgis.core import (
    QgsLocatorFilter,
    QgsSettings,
    QgsSettingsTree,
    QgsSettingsEntryEnumFlag,
    QgsSettingsEntryInteger,
)
from qgis.gui import (
    QgsGui,
    QgsSettingsEditorWidgetWrapper,
    QgsSettingsEnumEditorWidgetWrapper,
)
import unittest
from qgis.testing import start_app, QgisTestCase


start_app()

PLUGIN_NAME = "UnitTestSettingsRegistry"


class PyQgsSettingsRegistry(QgisTestCase):

    def setUp(self):
        self.settings_node = QgsSettingsTree.createPluginTreeNode(
            pluginName=PLUGIN_NAME
        )

    def tearDown(self):
        QgsSettingsTree.unregisterPluginTreeNode(PLUGIN_NAME)

    def test_settings_registry(self):
        int_setting = QgsSettingsEntryInteger("int_setting", self.settings_node, 77)
        registry = QgsGui.settingsEditorWidgetRegistry()

        editor = registry.createEditor(int_setting, [])
        self.assertIsInstance(editor, QSpinBox)

        wrapper = QgsSettingsEditorWidgetWrapper.fromWidget(editor)
        self.assertEqual(editor.value(), 77)

        editor.setValue(6)
        self.assertEqual(wrapper.variantValueFromWidget(), 6)
        wrapper.setSettingFromWidget()
        self.assertEqual(int_setting.value(), 6)

    def test_settings_registry_custom_enumflag_py(self):
        priority_setting = QgsSettingsEntryEnumFlag(
            "priority", self.settings_node, QgsLocatorFilter.Priority.High
        )
        registry = QgsGui.settingsEditorWidgetRegistry()
        registry.addWrapperForSetting(
            QgsSettingsEnumEditorWidgetWrapper(), priority_setting
        )

        editor = registry.createEditor(priority_setting, [])
        self.assertIsInstance(editor, QComboBox)
        wrapper = QgsSettingsEditorWidgetWrapper.fromWidget(editor)

        self.assertEqual(editor.currentData(), "High")

        editor.setCurrentIndex(editor.findData("Low"))

        self.assertEqual(wrapper.variantValueFromWidget(), "Low")

        wrapper.setSettingFromWidget()

        self.assertEqual(priority_setting.value(), QgsLocatorFilter.Priority.Low)
        self.assertEqual(QgsSettings().value(f"plugins/{PLUGIN_NAME}/priority"), "Low")


if __name__ == "__main__":
    unittest.main()
