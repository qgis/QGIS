"""QGIS Unit tests for Python utils

From build dir, run: ctest -R PyPythonUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Germán Carrillo'
__date__ = '31.8.2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import os

from qgis.testing import unittest, start_app
from qgis import utils

from utilities import unitTestDataPath


class TestPythonUtils(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        start_app()

    def test_update_available_plugins(self):
        utils.plugin_paths = [os.path.join(unitTestDataPath(), "test_plugin_path")]
        utils.updateAvailablePlugins(True)
        self.assertIn("PluginPathTest", utils.available_plugins)
        self.assertIn("dependent_plugin_1", utils.available_plugins)
        self.assertIn("dependent_plugin_2", utils.available_plugins)

        idx_independent_plugin = utils.available_plugins.index("PluginPathTest")
        idx_dependent_plugin_1 = utils.available_plugins.index("dependent_plugin_1")
        idx_dependent_plugin_2 = utils.available_plugins.index("dependent_plugin_2")

        self.assertGreater(idx_dependent_plugin_2, idx_dependent_plugin_1)
        self.assertGreater(idx_dependent_plugin_1, idx_independent_plugin)

    def test_sort_by_dependency(self):
        plugins = ["dependent_plugin_2", "dependent_plugin_1", "PluginPathTest"]
        plugin_name_map = {"Dependent plugin 2": "dependent_plugin_2", "Dependent plugin 1": "dependent_plugin_1", "plugin path test": "PluginPathTest"}

        utils.plugin_paths = [os.path.join(unitTestDataPath(), "test_plugin_path")]
        utils.updateAvailablePlugins()  # Required to have a proper plugins_metadata_parser
        sorted_plugins = utils._sortAvailablePlugins(plugins, plugin_name_map)

        expected_sorted_plugins = ["PluginPathTest", "dependent_plugin_1", "dependent_plugin_2"]
        self.assertEqual(sorted_plugins, expected_sorted_plugins)

    def test_sort_by_dependency_move_plugin(self):
        plugins = ["MSP", "P1", "LPA", "P2", "LA", "A", "MB", "P3", "LAA", "P4"]
        deps = {"A": ["MB", "MSP"], "LPA": ["A"], "LA": ["A"], "LAA": ["A"]}

        sorted_plugins = plugins.copy()
        visited = []

        for plugin in plugins:
            utils._move_plugin(plugin, deps, visited, sorted_plugins)

        expected_sorted_plugins = ["MSP", "P1", "P2", "MB", "A", "LA", "LPA", "P3", "LAA", "P4"]

        self.assertEqual(sorted_plugins, expected_sorted_plugins)


if __name__ == "__main__":
    unittest.main()
