"""
Test the QgsSettings class

Run with: ctest -V -R PyQgsSettings

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile
from pathlib import Path

from qgis.PyQt.QtCore import QSettings, QVariant
from qgis.core import Qgis, QgsMapLayerProxyModel, QgsSettings, QgsTolerance, NULL
import unittest
from qgis.testing import start_app, QgisTestCase

__author__ = "Alessandro Pasotti"
__date__ = "02/02/2017"
__copyright__ = "Copyright 2017, The QGIS Project"


start_app()


class TestQgsSettings(QgisTestCase):

    cnt = 0

    def setUp(self):
        self.cnt += 1
        h, path = tempfile.mkstemp(".ini")
        Path(path).touch()
        assert QgsSettings.setGlobalSettingsPath(path)
        self.settings = QgsSettings("testqgissettings", f"testqgissettings{self.cnt}")
        self.globalsettings = QSettings(
            self.settings.globalSettingsPath(), QSettings.Format.IniFormat
        )
        self.globalsettings.sync()
        assert os.path.exists(self.globalsettings.fileName())

    def tearDown(self):
        settings_file = self.settings.fileName()
        settings_default_file = self.settings.globalSettingsPath()
        del self.settings
        try:
            os.unlink(settings_file)
        except:
            pass
        try:
            os.unlink(settings_default_file)
        except:
            pass

    def addToDefaults(self, key, value):
        self.globalsettings.setValue(key, value)
        self.globalsettings.sync()

    def addArrayToDefaults(self, prefix, key, values):
        defaults = QSettings(
            self.settings.globalSettingsPath(), QSettings.Format.IniFormat
        )  # NOQA
        self.globalsettings.beginWriteArray(prefix)
        i = 0
        for v in values:
            self.globalsettings.setArrayIndex(i)
            self.globalsettings.setValue(key, v)
            i += 1
        self.globalsettings.endArray()
        self.globalsettings.sync()

    def addGroupToDefaults(self, prefix, kvp):
        defaults = QSettings(
            self.settings.globalSettingsPath(), QSettings.Format.IniFormat
        )  # NOQA
        self.globalsettings.beginGroup(prefix)
        for k, v in kvp.items():
            self.globalsettings.setValue(k, v)
        self.globalsettings.endGroup()
        self.globalsettings.sync()

    def test_basic_functionality(self):
        self.assertEqual(
            self.settings.value("testqgissettings/doesnotexists", "notexist"),
            "notexist",
        )
        self.settings.setValue("testqgissettings/name", "qgisrocks")
        self.settings.sync()
        self.assertEqual(self.settings.value("testqgissettings/name"), "qgisrocks")

    def test_defaults(self):
        self.assertIsNone(self.settings.value("testqgissettings/name"))
        self.addToDefaults("testqgissettings/name", "qgisrocks")
        self.assertEqual(self.settings.value("testqgissettings/name"), "qgisrocks")

    def test_allkeys(self):
        self.assertEqual(self.settings.allKeys(), [])
        self.addToDefaults("testqgissettings/name", "qgisrocks")
        self.addToDefaults("testqgissettings/name2", "qgisrocks2")
        self.settings.setValue("nepoti/eman", "osaple")

        self.assertEqual(3, len(self.settings.allKeys()))
        self.assertIn("testqgissettings/name", self.settings.allKeys())
        self.assertIn("nepoti/eman", self.settings.allKeys())
        self.assertEqual("qgisrocks", self.settings.value("testqgissettings/name"))
        self.assertEqual("qgisrocks2", self.settings.value("testqgissettings/name2"))
        self.assertEqual(
            "qgisrocks", self.globalsettings.value("testqgissettings/name")
        )
        self.assertEqual("osaple", self.settings.value("nepoti/eman"))
        self.assertEqual(3, len(self.settings.allKeys()))
        self.assertEqual(2, len(self.globalsettings.allKeys()))

    def test_precedence_simple(self):
        self.assertEqual(self.settings.allKeys(), [])
        self.addToDefaults("testqgissettings/names/name1", "qgisrocks1")
        self.settings.setValue("testqgissettings/names/name1", "qgisrocks-1")

        self.assertEqual(
            self.settings.value("testqgissettings/names/name1"), "qgisrocks-1"
        )

    def test_precedence_group(self):
        """Test if user can override a group value"""
        self.assertEqual(self.settings.allKeys(), [])
        self.addGroupToDefaults(
            "connections-xyz",
            {
                "OSM": "http://a.tile.openstreetmap.org/{z}/{x}/{y}.png",
                "OSM-b": "http://b.tile.openstreetmap.org/{z}/{x}/{y}.png",
            },
        )
        self.settings.beginGroup("connections-xyz")
        self.assertEqual(
            self.settings.value("OSM"),
            "http://a.tile.openstreetmap.org/{z}/{x}/{y}.png",
        )
        self.assertEqual(
            self.settings.value("OSM-b"),
            "http://b.tile.openstreetmap.org/{z}/{x}/{y}.png",
        )
        self.settings.endGroup()

        # Override edit
        self.settings.beginGroup("connections-xyz")
        self.settings.setValue("OSM", "http://c.tile.openstreetmap.org/{z}/{x}/{y}.png")
        self.settings.endGroup()

        # Check it again!
        self.settings.beginGroup("connections-xyz")
        self.assertEqual(
            self.settings.value("OSM"),
            "http://c.tile.openstreetmap.org/{z}/{x}/{y}.png",
        )
        self.assertEqual(
            self.settings.value("OSM-b"),
            "http://b.tile.openstreetmap.org/{z}/{x}/{y}.png",
        )
        self.settings.endGroup()

        # Override remove: the global value will be resumed!!!
        self.settings.beginGroup("connections-xyz")
        self.settings.remove("OSM")
        self.settings.endGroup()

        # Check it again!
        self.settings.beginGroup("connections-xyz")
        self.assertEqual(
            self.settings.value("OSM"),
            "http://a.tile.openstreetmap.org/{z}/{x}/{y}.png",
        )
        self.assertEqual(
            self.settings.value("OSM-b"),
            "http://b.tile.openstreetmap.org/{z}/{x}/{y}.png",
        )
        self.settings.endGroup()

        # Override remove: store a blank!
        self.settings.beginGroup("connections-xyz")
        self.settings.setValue("OSM", "")
        self.settings.endGroup()

        # Check it again!
        self.settings.beginGroup("connections-xyz")
        self.assertEqual(self.settings.value("OSM"), "")
        self.assertEqual(
            self.settings.value("OSM-b"),
            "http://b.tile.openstreetmap.org/{z}/{x}/{y}.png",
        )
        self.settings.endGroup()

        # Override remove: store a None: will resume the global setting!
        self.settings.beginGroup("connections-xyz")
        self.settings.setValue("OSM", None)
        self.settings.endGroup()

        # Check it again!
        self.settings.beginGroup("connections-xyz")
        self.assertEqual(
            self.settings.value("OSM"),
            "http://a.tile.openstreetmap.org/{z}/{x}/{y}.png",
        )
        self.assertEqual(
            self.settings.value("OSM-b"),
            "http://b.tile.openstreetmap.org/{z}/{x}/{y}.png",
        )
        self.settings.endGroup()

    def test_uft8(self):
        self.assertEqual(self.settings.allKeys(), [])
        self.addToDefaults("testqgissettings/names/namèé↓1", "qgisrocks↓1")
        self.assertEqual(
            self.settings.value("testqgissettings/names/namèé↓1"), "qgisrocks↓1"
        )

        self.settings.setValue("testqgissettings/names/namèé↓2", "qgisrocks↓2")
        self.assertEqual(
            self.settings.value("testqgissettings/names/namèé↓2"), "qgisrocks↓2"
        )
        self.settings.setValue("testqgissettings/names/namèé↓1", "qgisrocks↓-1")
        self.assertEqual(
            self.settings.value("testqgissettings/names/namèé↓1"), "qgisrocks↓-1"
        )

    def test_groups(self):
        self.assertEqual(self.settings.allKeys(), [])
        self.addToDefaults("testqgissettings/names/name1", "qgisrocks1")
        self.addToDefaults("testqgissettings/names/name2", "qgisrocks2")
        self.addToDefaults("testqgissettings/names/name3", "qgisrocks3")
        self.addToDefaults("testqgissettings/name", "qgisrocks")

        self.settings.beginGroup("testqgissettings")
        self.assertEqual(self.settings.group(), "testqgissettings")
        self.assertEqual(["names"], self.settings.childGroups())

        self.settings.setValue("surnames/name1", "qgisrocks-1")
        self.assertEqual(["surnames", "names"], self.settings.childGroups())

        self.settings.setValue("names/name1", "qgisrocks-1")
        self.assertEqual("qgisrocks-1", self.settings.value("names/name1"))
        self.settings.endGroup()
        self.assertEqual(self.settings.group(), "")
        self.settings.beginGroup("testqgissettings/names")
        self.assertEqual(self.settings.group(), "testqgissettings/names")
        self.settings.setValue("name4", "qgisrocks-4")
        keys = sorted(self.settings.childKeys())
        self.assertEqual(keys, ["name1", "name2", "name3", "name4"])
        self.settings.endGroup()
        self.assertEqual(self.settings.group(), "")
        self.assertEqual(
            "qgisrocks-1", self.settings.value("testqgissettings/names/name1")
        )
        self.assertEqual(
            "qgisrocks-4", self.settings.value("testqgissettings/names/name4")
        )

    def test_global_groups(self):
        self.assertEqual(self.settings.allKeys(), [])
        self.assertEqual(self.globalsettings.allKeys(), [])

        self.addToDefaults("testqgissettings/foo/first", "qgis")
        self.addToDefaults("testqgissettings/foo/last", "rocks")

        self.settings.beginGroup("testqgissettings")
        self.assertEqual(self.settings.group(), "testqgissettings")
        self.assertEqual(["foo"], self.settings.childGroups())
        self.assertEqual(["foo"], self.settings.globalChildGroups())
        self.settings.endGroup()
        self.assertEqual(self.settings.group(), "")

        self.settings.setValue("testqgissettings/bar/first", "qgis")
        self.settings.setValue("testqgissettings/bar/last", "rocks")

        self.settings.beginGroup("testqgissettings")
        self.assertEqual(sorted(["bar", "foo"]), sorted(self.settings.childGroups()))
        self.assertEqual(["foo"], self.settings.childGroups(Qgis.SettingsOrigin.Global))
        self.assertEqual(["bar"], self.settings.childGroups(Qgis.SettingsOrigin.Local))
        self.settings.endGroup()

        self.globalsettings.remove("testqgissettings/foo")

        self.settings.beginGroup("testqgissettings")
        self.assertEqual(["bar"], self.settings.childGroups())
        self.assertEqual([], self.settings.globalChildGroups())
        self.settings.endGroup()

    def test_origin(self):
        self.assertEqual(self.settings.allKeys(), [])
        self.assertEqual(self.globalsettings.allKeys(), [])

        self.addToDefaults("testqgissettings/global/setting", "qgis")
        self.settings.setValue("testqgissettings/local/setting", "rocks")

        self.assertEqual(
            self.settings.origin("testqgissettings/global/setting"),
            Qgis.SettingsOrigin.Global,
        )
        self.assertEqual(
            self.settings.origin("testqgissettings/local/setting"),
            Qgis.SettingsOrigin.Local,
        )
        self.assertEqual(self.settings.origin("undefined-key"), Qgis.SettingsOrigin.Any)

        self.settings.setValue("testqgissettings/global/setting", "rocks")
        self.assertEqual(
            self.settings.origin("testqgissettings/global/setting"),
            Qgis.SettingsOrigin.Global,
        )

    def test_group_section(self):
        # Test group by using Section
        self.settings.beginGroup("firstgroup", section=QgsSettings.Section.Core)
        self.assertEqual(self.settings.group(), "core/firstgroup")
        self.assertEqual([], self.settings.childGroups())
        self.settings.setValue("key", "value")
        self.settings.setValue("key2/subkey1", "subvalue1")
        self.settings.setValue("key2/subkey2", "subvalue2")
        self.settings.setValue("key3", "value3")

        self.assertEqual(
            ["key", "key2/subkey1", "key2/subkey2", "key3"], self.settings.allKeys()
        )
        self.assertEqual(["key", "key3"], self.settings.childKeys())
        self.assertEqual(["key2"], self.settings.childGroups())
        self.settings.endGroup()
        self.assertEqual(self.settings.group(), "")
        # Set value by writing the group manually
        self.settings.setValue(
            "firstgroup/key4", "value4", section=QgsSettings.Section.Core
        )
        # Checking the value that have been set
        self.assertEqual(
            self.settings.value("firstgroup/key", section=QgsSettings.Section.Core),
            "value",
        )
        self.assertEqual(
            self.settings.value(
                "firstgroup/key2/subkey1", section=QgsSettings.Section.Core
            ),
            "subvalue1",
        )
        self.assertEqual(
            self.settings.value(
                "firstgroup/key2/subkey2", section=QgsSettings.Section.Core
            ),
            "subvalue2",
        )
        self.assertEqual(
            self.settings.value("firstgroup/key3", section=QgsSettings.Section.Core),
            "value3",
        )
        self.assertEqual(
            self.settings.value("firstgroup/key4", section=QgsSettings.Section.Core),
            "value4",
        )
        # Clean up firstgroup
        self.settings.remove("firstgroup", section=QgsSettings.Section.Core)

    def test_array(self):
        self.assertEqual(self.settings.allKeys(), [])
        self.addArrayToDefaults(
            "testqgissettings", "key", ["qgisrocks1", "qgisrocks2", "qgisrocks3"]
        )
        self.assertEqual(
            self.settings.allKeys(),
            [
                "testqgissettings/1/key",
                "testqgissettings/2/key",
                "testqgissettings/3/key",
                "testqgissettings/size",
            ],
        )
        self.assertEqual(
            self.globalsettings.allKeys(),
            [
                "testqgissettings/1/key",
                "testqgissettings/2/key",
                "testqgissettings/3/key",
                "testqgissettings/size",
            ],
        )

        self.assertEqual(3, self.globalsettings.beginReadArray("testqgissettings"))
        self.globalsettings.endArray()
        self.assertEqual(3, self.settings.beginReadArray("testqgissettings"))

        values = []
        for i in range(3):
            self.settings.setArrayIndex(i)
            values.append(self.settings.value("key"))

        self.assertEqual(values, ["qgisrocks1", "qgisrocks2", "qgisrocks3"])

    def test_array_overrides(self):
        """Test if an array completely shadows the global one"""
        self.assertEqual(self.settings.allKeys(), [])
        self.addArrayToDefaults(
            "testqgissettings", "key", ["qgisrocks1", "qgisrocks2", "qgisrocks3"]
        )
        self.assertEqual(
            self.settings.allKeys(),
            [
                "testqgissettings/1/key",
                "testqgissettings/2/key",
                "testqgissettings/3/key",
                "testqgissettings/size",
            ],
        )
        self.assertEqual(
            self.globalsettings.allKeys(),
            [
                "testqgissettings/1/key",
                "testqgissettings/2/key",
                "testqgissettings/3/key",
                "testqgissettings/size",
            ],
        )

        self.assertEqual(3, self.globalsettings.beginReadArray("testqgissettings"))
        self.globalsettings.endArray()
        self.assertEqual(3, self.settings.beginReadArray("testqgissettings"))

        # Now override!
        self.settings.beginWriteArray("testqgissettings")
        self.settings.setArrayIndex(0)
        self.settings.setValue("key", "myqgisrocksmore1")
        self.settings.setArrayIndex(1)
        self.settings.setValue("key", "myqgisrocksmore2")
        self.settings.endArray()

        # Check it!
        self.assertEqual(2, self.settings.beginReadArray("testqgissettings"))

        values = []
        for i in range(2):
            self.settings.setArrayIndex(i)
            values.append(self.settings.value("key"))

        self.assertEqual(values, ["myqgisrocksmore1", "myqgisrocksmore2"])

    def test_section_getters_setters(self):
        self.assertEqual(self.settings.allKeys(), [])

        self.settings.setValue("key1", "core1", section=QgsSettings.Section.Core)
        self.settings.setValue("key2", "core2", section=QgsSettings.Section.Core)

        self.settings.setValue("key1", "server1", section=QgsSettings.Section.Server)
        self.settings.setValue("key2", "server2", section=QgsSettings.Section.Server)

        self.settings.setValue("key1", "gui1", section=QgsSettings.Section.Gui)
        self.settings.setValue("key2", "gui2", QgsSettings.Section.Gui)

        self.settings.setValue("key1", "plugins1", section=QgsSettings.Section.Plugins)
        self.settings.setValue("key2", "plugins2", section=QgsSettings.Section.Plugins)

        self.settings.setValue("key1", "misc1", section=QgsSettings.Section.Misc)
        self.settings.setValue("key2", "misc2", section=QgsSettings.Section.Misc)

        self.settings.setValue("key1", "auth1", section=QgsSettings.Section.Auth)
        self.settings.setValue("key2", "auth2", section=QgsSettings.Section.Auth)

        self.settings.setValue("key1", "app1", section=QgsSettings.Section.App)
        self.settings.setValue("key2", "app2", section=QgsSettings.Section.App)

        self.settings.setValue(
            "key1", "provider1", section=QgsSettings.Section.Providers
        )
        self.settings.setValue(
            "key2", "provider2", section=QgsSettings.Section.Providers
        )

        # This is an overwrite of previous setting and it is intentional
        self.settings.setValue("key1", "auth1", section=QgsSettings.Section.Auth)
        self.settings.setValue("key2", "auth2", section=QgsSettings.Section.Auth)

        # Test that the values are namespaced
        self.assertEqual(self.settings.value("core/key1"), "core1")
        self.assertEqual(self.settings.value("core/key2"), "core2")

        self.assertEqual(self.settings.value("server/key1"), "server1")
        self.assertEqual(self.settings.value("server/key2"), "server2")

        self.assertEqual(self.settings.value("gui/key1"), "gui1")
        self.assertEqual(self.settings.value("gui/key2"), "gui2")

        self.assertEqual(self.settings.value("plugins/key1"), "plugins1")
        self.assertEqual(self.settings.value("plugins/key2"), "plugins2")

        self.assertEqual(self.settings.value("misc/key1"), "misc1")
        self.assertEqual(self.settings.value("misc/key2"), "misc2")

        # Test getters
        self.assertEqual(
            self.settings.value("key1", None, section=QgsSettings.Section.Core), "core1"
        )
        self.assertEqual(
            self.settings.value("key2", None, section=QgsSettings.Section.Core), "core2"
        )

        self.assertEqual(
            self.settings.value("key1", None, section=QgsSettings.Section.Server),
            "server1",
        )
        self.assertEqual(
            self.settings.value("key2", None, section=QgsSettings.Section.Server),
            "server2",
        )

        self.assertEqual(
            self.settings.value("key1", None, section=QgsSettings.Section.Gui), "gui1"
        )
        self.assertEqual(
            self.settings.value("key2", None, section=QgsSettings.Section.Gui), "gui2"
        )

        self.assertEqual(
            self.settings.value("key1", None, section=QgsSettings.Section.Plugins),
            "plugins1",
        )
        self.assertEqual(
            self.settings.value("key2", None, section=QgsSettings.Section.Plugins),
            "plugins2",
        )

        self.assertEqual(
            self.settings.value("key1", None, section=QgsSettings.Section.Misc), "misc1"
        )
        self.assertEqual(
            self.settings.value("key2", None, section=QgsSettings.Section.Misc), "misc2"
        )

        self.assertEqual(
            self.settings.value("key1", None, section=QgsSettings.Section.Auth), "auth1"
        )
        self.assertEqual(
            self.settings.value("key2", None, section=QgsSettings.Section.Auth), "auth2"
        )

        self.assertEqual(
            self.settings.value("key1", None, section=QgsSettings.Section.App), "app1"
        )
        self.assertEqual(
            self.settings.value("key2", None, section=QgsSettings.Section.App), "app2"
        )

        self.assertEqual(
            self.settings.value("key1", None, section=QgsSettings.Section.Providers),
            "provider1",
        )
        self.assertEqual(
            self.settings.value("key2", None, section=QgsSettings.Section.Providers),
            "provider2",
        )

        # Test default values on Section getter
        self.assertEqual(
            self.settings.value(
                "key_not_exist", "misc_not_exist", section=QgsSettings.Section.Misc
            ),
            "misc_not_exist",
        )

    def test_contains(self):
        self.assertEqual(self.settings.allKeys(), [])
        self.addToDefaults("testqgissettings/name", "qgisrocks1")
        self.addToDefaults("testqgissettings/name2", "qgisrocks2")

        self.assertTrue(self.settings.contains("testqgissettings/name"))
        self.assertTrue(self.settings.contains("testqgissettings/name2"))

        self.settings.setValue("testqgissettings/name3", "qgisrocks3")
        self.assertTrue(self.settings.contains("testqgissettings/name3"))

    def test_remove(self):
        self.settings.setValue("testQgisSettings/temp", True)
        self.assertEqual(self.settings.value("testQgisSettings/temp"), True)
        self.settings.remove("testQgisSettings/temp")
        self.assertEqual(self.settings.value("testqQgisSettings/temp"), None)

        # Test remove by using Section
        self.settings.setValue(
            "testQgisSettings/tempSection", True, section=QgsSettings.Section.Core
        )
        self.assertEqual(
            self.settings.value(
                "testQgisSettings/tempSection", section=QgsSettings.Section.Core
            ),
            True,
        )
        self.settings.remove("testQgisSettings/temp", section=QgsSettings.Section.Core)
        self.assertEqual(
            self.settings.value(
                "testqQgisSettings/temp", section=QgsSettings.Section.Core
            ),
            None,
        )

    def test_enumValue(self):
        self.settings.setValue("enum", "Layer")
        self.assertEqual(
            self.settings.enumValue("enum", Qgis.MapToolUnit.Pixels),
            Qgis.MapToolUnit.Layer,
        )
        self.settings.setValue("enum", "dummy_setting")
        self.assertEqual(
            self.settings.enumValue("enum", Qgis.MapToolUnit.Pixels),
            Qgis.MapToolUnit.Pixels,
        )
        self.assertEqual(
            type(self.settings.enumValue("enum", Qgis.MapToolUnit.Pixels)),
            QgsTolerance.UnitType,
        )

    def test_setEnumValue(self):
        self.settings.setValue("enum", "Layer")
        self.assertEqual(
            self.settings.enumValue("enum", Qgis.MapToolUnit.Pixels),
            Qgis.MapToolUnit.Layer,
        )
        self.settings.setEnumValue("enum", Qgis.MapToolUnit.Pixels)
        self.assertEqual(
            self.settings.enumValue("enum", Qgis.MapToolUnit.Pixels),
            Qgis.MapToolUnit.Pixels,
        )

    def test_flagValue(self):
        pointAndLine = Qgis.LayerFilters(
            Qgis.LayerFilter.PointLayer | Qgis.LayerFilter.LineLayer
        )
        pointAndPolygon = Qgis.LayerFilters(
            Qgis.LayerFilter.PointLayer | Qgis.LayerFilter.PolygonLayer
        )

        self.settings.setValue("flag", "PointLayer|PolygonLayer")
        self.assertEqual(self.settings.flagValue("flag", pointAndLine), pointAndPolygon)
        self.settings.setValue("flag", "dummy_setting")
        self.assertEqual(self.settings.flagValue("flag", pointAndLine), pointAndLine)
        self.assertEqual(
            type(self.settings.flagValue("enum", pointAndLine)), Qgis.LayerFilters
        )

    def test_overwriteDefaultValues(self):
        """Test that unchanged values are not stored"""
        self.globalsettings.setValue("a_value_with_default", "a value")
        self.globalsettings.setValue("an_invalid_value", NULL)

        self.assertEqual(self.settings.value("a_value_with_default"), "a value")
        self.assertEqual(self.settings.value("an_invalid_value"), NULL)

        # Now, set them with the same current value
        self.settings.setValue("a_value_with_default", "a value")
        self.settings.setValue("an_invalid_value", NULL)

        # Check
        pure_settings = QSettings(self.settings.fileName(), QSettings.Format.IniFormat)
        self.assertNotIn("a_value_with_default", pure_settings.allKeys())
        self.assertNotIn("an_invalid_value", pure_settings.allKeys())

        # Set a changed value
        self.settings.setValue("a_value_with_default", "a new value")
        self.settings.setValue("an_invalid_value", "valid value")

        # Check
        self.assertIn("a_value_with_default", pure_settings.allKeys())
        self.assertIn("an_invalid_value", pure_settings.allKeys())

        self.assertEqual(self.settings.value("a_value_with_default"), "a new value")
        self.assertEqual(self.settings.value("an_invalid_value"), "valid value")

        # Re-set to original values
        self.settings.setValue("a_value_with_default", "a value")
        self.settings.setValue("an_invalid_value", NULL)

        self.assertEqual(self.settings.value("a_value_with_default"), "a value")
        self.assertEqual(self.settings.value("an_invalid_value"), NULL)

        # Check if they are gone
        pure_settings = QSettings(self.settings.fileName(), QSettings.Format.IniFormat)
        self.assertIn("a_value_with_default", pure_settings.allKeys())
        self.assertIn("an_invalid_value", pure_settings.allKeys())


if __name__ == "__main__":
    unittest.main()
