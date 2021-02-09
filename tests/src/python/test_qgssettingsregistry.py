# -*- coding: utf-8 -*-
"""
Test the QgsSettingsRegistry class

Run with: ctest -V -R PyQgsSettingsRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile
from qgis.core import QgsSettingsRegistry, QgsSettings, QgsTolerance, QgsMapLayerProxyModel
from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QSettings, QVariant
from pathlib import Path

__author__ = 'Damiano Lombardi'
__date__ = '08/02/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'


start_app()


class TestQgsSettingsRegistry(unittest.TestCase):

    cnt = 0

    def setUp(self):
        self.cnt += 1
        self.settingsRegistry = QgsSettingsRegistry(QgsSettings.NoSection)

    def tearDown(self):
        del(self.settingsRegistry)

    def test_basic_functionality(self):
        self.settingsRegistry.registerSettings('testqgissettingsregistry/basicsetting', 42, 'Basic setting')
        self.assertEqual(self.settingsRegistry.isRegistered('testqgissettingsregistry/basicsetting'), True)
        self.assertEqual(self.settingsRegistry.isRegistered('testqgissettingsregistry/notexisting'), False)
        self.settingsRegistry.unregister('testqgissettingsregistry/basicsetting')
        self.assertEqual(self.settingsRegistry.isRegistered('testqgissettingsregistry/basicsetting'), False)

    def test_defaults(self):
        self.settingsRegistry.registerSettings('testqgissettingsregistry/defaultsettingsinteger', 42, 'Default setting integer')
        self.assertEqual(self.settingsRegistry.defaultValue('testqgissettingsregistry/defaultsettingsinteger'), 42)
        self.settingsRegistry.unregister('testqgissettingsregistry/defaultsettingsinteger')
        self.settingsRegistry.registerSettings('testqgissettingsregistry/defaultsettingsfloat', 3.14, 'Default setting float')
        self.assertEqual(self.settingsRegistry.defaultValue('testqgissettingsregistry/defaultsettingsfloat'), 3.14)
        self.settingsRegistry.unregister('testqgissettingsregistry/defaultsettingsfloat')
        self.settingsRegistry.registerSettings('testqgissettingsregistry/defaultsettingsstring', 'qgisrocks', 'Default setting string')
        self.assertEqual(self.settingsRegistry.defaultValue('testqgissettingsregistry/defaultsettingsstring'), 'qgisrocks')
        self.settingsRegistry.unregister('testqgissettingsregistry/defaultsettingsstring')

    def test_uft8(self):
        self.settingsRegistry.registerSettings('testqgissettings/names/namèé↓1', 'qgisrocks↓1')
        self.assertEqual(self.settingsRegistry.value('testqgissettings/names/namèé↓1'), 'qgisrocks↓1')
        self.assertEqual(self.settingsRegistry.defaultValue('testqgissettings/names/namèé↓1'), 'qgisrocks↓1')
        self.settingsRegistry.setValue('testqgissettings/names/namèé↓1', 'qgisrocks↓2')
        self.assertEqual(self.settingsRegistry.value('testqgissettings/names/namèé↓1'), 'qgisrocks↓2')
        self.settingsRegistry.setValue('testqgissettings/names/namèé↓1', 'qgisrocks↓-1')
        self.assertEqual(self.settingsRegistry.value('testqgissettings/names/namèé↓1'), 'qgisrocks↓-1')
        self.settingsRegistry.unregister('testqgissettings/names/namèé↓1')

    def test_section_getters_setters(self):

        settingsRegistryCore = QgsSettingsRegistry(QgsSettings.Core)
        settingsRegistryCore.registerSettings('key1')
        settingsRegistryCore.registerSettings('key2')
        settingsRegistryCore.setValue('key1', 'core1')
        settingsRegistryCore.setValue('key2', 'core2')

        settingsRegistryServer = QgsSettingsRegistry(QgsSettings.Server)
        settingsRegistryServer.registerSettings('key1')
        settingsRegistryServer.registerSettings('key2')
        settingsRegistryServer.setValue('key1', 'server1')
        settingsRegistryServer.setValue('key2', 'server2')

        settingsRegistryGui = QgsSettingsRegistry(QgsSettings.Gui)
        settingsRegistryGui.registerSettings('key1')
        settingsRegistryGui.registerSettings('key2')
        settingsRegistryGui.setValue('key1', 'gui1')
        settingsRegistryGui.setValue('key2', 'gui2')

        settingsRegistryPlugins = QgsSettingsRegistry(QgsSettings.Plugins)
        settingsRegistryPlugins.registerSettings('key1')
        settingsRegistryPlugins.registerSettings('key2')
        settingsRegistryPlugins.setValue('key1', 'plugins1')
        settingsRegistryPlugins.setValue('key2', 'plugins2')

        settingsRegistryMisc = QgsSettingsRegistry(QgsSettings.Misc)
        settingsRegistryMisc.registerSettings('key1')
        settingsRegistryMisc.registerSettings('key2')
        settingsRegistryMisc.setValue('key1', 'misc1')
        settingsRegistryMisc.setValue('key2', 'misc2')

        settingsRegistryAuth = QgsSettingsRegistry(QgsSettings.Auth)
        settingsRegistryAuth.registerSettings('key1')
        settingsRegistryAuth.registerSettings('key2')
        settingsRegistryAuth.setValue('key1', 'auth1')
        settingsRegistryAuth.setValue('key2', 'auth2')

        settingsRegistryApp = QgsSettingsRegistry(QgsSettings.App)
        settingsRegistryApp.registerSettings('key1')
        settingsRegistryApp.registerSettings('key2')
        settingsRegistryApp.setValue('key1', 'app1')
        settingsRegistryApp.setValue('key2', 'app2')

        settingsRegistryProviders = QgsSettingsRegistry(QgsSettings.Providers)
        settingsRegistryProviders.registerSettings('key1')
        settingsRegistryProviders.registerSettings('key2')
        settingsRegistryProviders.setValue('key1', 'provider1')
        settingsRegistryProviders.setValue('key2', 'provider2')

        # This is an overwrite of previous setting and it is intentional
        settingsRegistryAuth = QgsSettingsRegistry(QgsSettings.Auth)
        settingsRegistryAuth.registerSettings('key1')
        settingsRegistryAuth.registerSettings('key2')
        settingsRegistryAuth.setValue('key1', 'auth1')
        settingsRegistryAuth.setValue('key2', 'auth2')

        # Test that the values are namespaced
        settings = QgsSettings()

        self.assertEqual(settings.value('core/key1'), 'core1')
        self.assertEqual(settings.value('core/key2'), 'core2')

        self.assertEqual(settings.value('server/key1'), 'server1')
        self.assertEqual(settings.value('server/key2'), 'server2')

        self.assertEqual(settings.value('gui/key1'), 'gui1')
        self.assertEqual(settings.value('gui/key2'), 'gui2')

        self.assertEqual(settings.value('plugins/key1'), 'plugins1')
        self.assertEqual(settings.value('plugins/key2'), 'plugins2')

        self.assertEqual(settings.value('misc/key1'), 'misc1')
        self.assertEqual(settings.value('misc/key2'), 'misc2')

        # Test getters
        self.assertEqual(settingsRegistryCore.value('key1'), 'core1')
        self.assertEqual(settingsRegistryCore.value('key2'), 'core2')

        self.assertEqual(settingsRegistryServer.value('key1'), 'server1')
        self.assertEqual(settingsRegistryServer.value('key2'), 'server2')

        self.assertEqual(settingsRegistryGui.value('key1'), 'gui1')
        self.assertEqual(settingsRegistryGui.value('key2'), 'gui2')

        self.assertEqual(settingsRegistryPlugins.value('key1'), 'plugins1')
        self.assertEqual(settingsRegistryPlugins.value('key2'), 'plugins2')

        self.assertEqual(settingsRegistryMisc.value('key1'), 'misc1')
        self.assertEqual(settingsRegistryMisc.value('key2'), 'misc2')

        self.assertEqual(settingsRegistryAuth.value('key1'), 'auth1')
        self.assertEqual(settingsRegistryAuth.value('key2'), 'auth2')

        self.assertEqual(settingsRegistryApp.value('key1'), 'app1')
        self.assertEqual(settingsRegistryApp.value('key2'), 'app2')

        self.assertEqual(settingsRegistryProviders.value('key1'), 'provider1')
        self.assertEqual(settingsRegistryProviders.value('key2'), 'provider2')

        # Test value on non registered settings
        self.assertEqual(self.settingsRegistry.value('key_not_exist'), None)

#    def test_enumValue(self):
#        self.settings.setValue('enum', 'LayerUnits')
#        self.assertEqual(self.settings.enumValue('enum', QgsTolerance.Pixels), QgsTolerance.LayerUnits)
#        self.settings.setValue('enum', 'dummy_setting')
#        self.assertEqual(self.settings.enumValue('enum', QgsTolerance.Pixels), QgsTolerance.Pixels)
#        self.assertEqual(type(self.settings.enumValue('enum', QgsTolerance.Pixels)), QgsTolerance.UnitType)

#    def test_setEnumValue(self):
#        self.settings.setValue('enum', 'LayerUnits')
#        self.assertEqual(self.settings.enumValue('enum', QgsTolerance.Pixels), QgsTolerance.LayerUnits)
#        self.settings.setEnumValue('enum', QgsTolerance.Pixels)
#        self.assertEqual(self.settings.enumValue('enum', QgsTolerance.Pixels), QgsTolerance.Pixels)

#    def test_flagValue(self):
#        pointAndLine = QgsMapLayerProxyModel.Filters(QgsMapLayerProxyModel.PointLayer | QgsMapLayerProxyModel.LineLayer)
#        pointAndPolygon = QgsMapLayerProxyModel.Filters(QgsMapLayerProxyModel.PointLayer | QgsMapLayerProxyModel.PolygonLayer)

#        self.settings.setValue('flag', 'PointLayer|PolygonLayer')
#        self.assertEqual(self.settings.flagValue('flag', pointAndLine), pointAndPolygon)
#        self.settings.setValue('flag', 'dummy_setting')
#        self.assertEqual(self.settings.flagValue('flag', pointAndLine), pointAndLine)
#        self.assertEqual(type(self.settings.flagValue('enum', pointAndLine)), QgsMapLayerProxyModel.Filters)


if __name__ == '__main__':
    unittest.main()
