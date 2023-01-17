# -*- coding: utf-8 -*-
"""
Test the QgsSettingsEntry classes

Run with: ctest -V -R PyQgsSettingsEntry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.core import QgsSettingsException, QgsSettings, QgsSettingsTreeElement, QgsSettingsEntryString, QgsSettingsEntryEnumFlag, QgsUnitTypes
from qgis.testing import start_app, unittest


__author__ = 'Denis Rouzaud'
__date__ = '19/12/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'


start_app()


class TestQgsSettingsEntry(unittest.TestCase):

    def setUp(self):
        self.pluginName = "UnitTest"

    def tearDown(self):
        QgsSettings.unregisterPluginTreeElement(self.pluginName)

    def test_constructor(self):
        with self.assertRaises(TypeError):
            QgsSettingsTreeElement()

        root = QgsSettings.createPluginTreeElement(self.pluginName)
        self.assertEqual(root.type(), QgsSettingsTreeElement.Type.Standard)
        pluginsElement = root.parent()
        self.assertEqual(pluginsElement.type(), QgsSettingsTreeElement.Type.Standard)
        self.assertEqual(pluginsElement.parent().type(), QgsSettingsTreeElement.Type.Root)
        self.assertEqual(pluginsElement.parent().parent(), None)

    def test_parent(self):
        root = QgsSettings.createPluginTreeElement(self.pluginName)
        self.assertEqual(root.type(), QgsSettingsTreeElement.Type.Standard)

        l1 = root.createChildElement("test-parent-level-1")
        self.assertEqual(l1.type(), QgsSettingsTreeElement.Type.Standard)
        self.assertEqual(l1.key(), "test-parent-level-1")
        self.assertEqual(l1.completeKey(), f"/plugins/{self.pluginName}/test-parent-level-1/")
        self.assertEqual(l1.parent(), root)
        self.assertEqual(root.childrenElements(), [l1])
        self.assertEqual(root.childrenSettings(), [])

        l1a = l1.createChildElement("level-a")
        self.assertEqual(l1a.type(), QgsSettingsTreeElement.Type.Standard)
        self.assertEqual(l1a.key(), "level-a")
        self.assertEqual(l1a.completeKey(), f"/plugins/{self.pluginName}/test-parent-level-1/level-a/")
        self.assertEqual(l1a.parent(), l1)
        self.assertEqual(l1.childrenElements(), [l1a])
        l1b = l1.createChildElement("level-b")
        self.assertEqual(l1.childrenElements(), [l1a, l1b])

    def test_setting(self):
        root = QgsSettings.createPluginTreeElement(self.pluginName)
        setting = QgsSettingsEntryString("mysetting", root)

        self.assertEqual(setting.parent(), root)
        self.assertEqual(setting.key(), f"/plugins/{self.pluginName}/mysetting")

        self.assertEqual(root.childrenSettings(), [setting])
        self.assertEqual(root.childrenElements(), [])

    def test_named_list(self):
        proot = QgsSettings.createPluginTreeElement(self.pluginName)
        l1 = proot.createChildElement("level-1")
        self.assertEqual(l1.namedElementsCount(), 0)
        nl = l1.createNamedListElement("my_list")
        self.assertEqual(nl.key(), "my_list")
        self.assertEqual(nl.completeKey(), f"/plugins/{self.pluginName}/level-1/my_list/items/%1/")
        self.assertEqual(nl.namedElementsCount(), 1)
        self.assertEqual(nl.childrenElements(), [])
        self.assertEqual(nl.childrenSettings(), [])

        # nesting lists
        nl2 = nl.createNamedListElement("my_nested_list", QgsSettingsTreeElement.Option.NamedListSelectedItemSetting)
        self.assertEqual(nl2.key(), "my_nested_list")
        self.assertEqual(nl2.completeKey(), f"/plugins/{self.pluginName}/level-1/my_list/items/%1/my_nested_list/items/%2/")
        self.assertEqual(nl2.namedElementsCount(), 2)
        self.assertEqual(nl2.childrenElements(), [])
        self.assertEqual(len(nl2.childrenSettings()), 0)  # the setting for the current selection
        self.assertEqual(nl2.selectedItemSetting().definitionKey(), f"/plugins/{self.pluginName}/level-1/my_list/items/%1/my_nested_list/selected")
        selected_key = f"/plugins/{self.pluginName}/level-1/my_list/items/item1/my_nested_list/selected"
        self.assertEqual(QgsSettings().value(selected_key), None)
        nl2.setSelectedItem("xxx", ["item1"])
        self.assertEqual(QgsSettings().value(selected_key), "xxx")

        # list with settings
        setting = QgsSettingsEntryString("mysetting-inlist", nl2)
        self.assertEqual(setting.definitionKey(), f"/plugins/{self.pluginName}/level-1/my_list/items/%1/my_nested_list/items/%2/mysetting-inlist")
        self.assertEqual(setting.key(['item1', 'item2']), f"/plugins/{self.pluginName}/level-1/my_list/items/item1/my_nested_list/items/item2/mysetting-inlist")
        self.assertEqual(nl2.childrenElements(), [])
        self.assertEqual(len(nl2.childrenSettings()), 1)
        self.assertEqual(nl2.childrenSettings()[0], setting)
        setting.setValue("xxx", ["item1", "item2"])
        self.assertEqual(QgsSettings().value(setting.key(['item1', 'item2'])), "xxx")
        with self.assertRaises(QgsSettingsException):
            nl2.deleteItem("item2")
        nl2.deleteItem("item2", ["item1"])
        self.assertEqual(QgsSettings().value(setting.key(['item1', 'item2'])), None)

    def test_registration(self):
        proot = QgsSettings.createPluginTreeElement(self.pluginName)
        self.assertEqual(len(proot.childrenElements()), 0)
        l1 = proot.createChildElement("level-1")
        self.assertEqual(len(proot.childrenElements()), 1)
        QgsSettings.unregisterPluginTreeElement(self.pluginName)

        # with several levels + settings
        proot = QgsSettings.createPluginTreeElement(self.pluginName)
        l1 = proot.createChildElement("level-1")
        s1 = QgsSettingsEntryString("my-setting-1", l1)
        l2 = l1.createChildElement("level-2")
        s2 = QgsSettingsEntryString("my-setting-2", l2)
        l2.unregisterChildSetting(s2)
        QgsSettings.unregisterPluginTreeElement(self.pluginName)

    def test_duplicated_key(self):
        proot = QgsSettings.createPluginTreeElement(self.pluginName)
        proot.createChildElement("duplicate-key")
        with self.assertRaises(QgsSettingsException):
            QgsSettingsEntryString("duplicate-key", proot)

    def test_python_implementation(self):
        proot = QgsSettings.createPluginTreeElement(self.pluginName)
        self.setting = QgsSettingsEntryEnumFlag("python-implemented-setting", proot, QgsUnitTypes.LayoutMeters)
        self.assertEqual(type(self.setting), QgsSettingsEntryEnumFlag)
        self.assertEqual(type(proot.childSetting("python-implemented-setting")), QgsSettingsEntryEnumFlag)


if __name__ == '__main__':
    unittest.main()
