"""QGIS Unit tests for Python utils

From build dir, run: ctest -R PyPythonUtils -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Germán Carrillo"
__date__ = "31.8.2021"
__copyright__ = "Copyright 2021, The QGIS Project"

import os
import subprocess
import sys
import tempfile
import unittest
from unittest import mock

from qgis import utils
from qgis.testing import QgisTestCase, start_app
from utilities import unitTestDataPath


class TestPythonUtils(QgisTestCase):
    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        start_app()

    def test_python_executable(self):
        exe = utils.python_executable()
        self.assertTrue(exe)
        self.assertTrue(os.path.isfile(exe))
        self.assertTrue(os.path.basename(exe).lower().startswith("python"))

        # the interpreter must run, and be the very one QGIS embeds
        version = subprocess.check_output(
            [exe, "-c", "import sys; print(sys.version_info[0], sys.version_info[1])"],
            text=True,
            timeout=60,
        )
        self.assertEqual(
            version.split(), [str(sys.version_info[0]), str(sys.version_info[1])]
        )

    def test_python_executable_embedded_layouts(self):
        """sys.executable is the host application when QGIS embeds Python"""

        def touch(*parts):
            path = os.path.join(*parts)
            os.makedirs(os.path.dirname(path), exist_ok=True)
            with open(path, "w"):
                pass
            os.chmod(path, 0o755)
            return path

        with tempfile.TemporaryDirectory() as root:
            # macOS application bundle
            host = touch(root, "QGIS.app", "Contents", "MacOS", "QGIS")
            touch(root, "QGIS.app", "Contents", "MacOS", "python3.12")
            wrapper = touch(root, "QGIS.app", "Contents", "MacOS", "python")
            prefix = os.path.join(root, "QGIS.app", "Contents", "Frameworks")
            with mock.patch.multiple(
                sys,
                platform="darwin",
                executable=host,
                prefix=prefix,
                base_prefix=prefix,
                exec_prefix=prefix,
            ):
                self.assertEqual(utils.python_executable(), wrapper)

            # the bundled interpreter run through the wrapper, e.g. by ctest
            with mock.patch.multiple(
                sys,
                platform="darwin",
                executable=os.path.join(
                    root, "QGIS.app", "Contents", "MacOS", "python3.12"
                ),
                prefix=prefix,
                base_prefix=prefix,
                exec_prefix=prefix,
            ):
                self.assertEqual(utils.python_executable(), wrapper)

            # an interpreter outside the bundle is left alone, whatever sits
            # next to it
            python = touch(root, "opt", "python", "bin", "python3.12")
            touch(root, "opt", "python", "bin", "python")
            prefix = os.path.join(root, "opt", "python")
            with mock.patch.multiple(
                sys,
                platform="darwin",
                executable=python,
                prefix=prefix,
                base_prefix=prefix,
                exec_prefix=prefix,
            ):
                self.assertEqual(utils.python_executable(), python)

            # OSGeo4W and the Windows standalone installer
            host = touch(root, "OSGeo4W", "bin", "qgis-bin.exe")
            python = touch(root, "OSGeo4W", "apps", "Python312", "python.exe")
            prefix = os.path.dirname(python)
            with mock.patch.multiple(
                sys,
                platform="win32",
                executable=host,
                prefix=prefix,
                base_prefix=prefix,
                exec_prefix=prefix,
            ):
                self.assertEqual(utils.python_executable(), python)

            # nothing to repair when sys.executable already is an interpreter
            python = touch(root, "usr", "bin", "python3")
            with mock.patch.multiple(sys, platform="linux", executable=python):
                self.assertEqual(utils.python_executable(), python)

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
        plugin_name_map = {
            "Dependent plugin 2": "dependent_plugin_2",
            "Dependent plugin 1": "dependent_plugin_1",
            "plugin path test": "PluginPathTest",
        }

        utils.plugin_paths = [os.path.join(unitTestDataPath(), "test_plugin_path")]
        utils.updateAvailablePlugins()  # Required to have a proper plugins_metadata_parser
        sorted_plugins = utils._sortAvailablePlugins(plugins, plugin_name_map)

        expected_sorted_plugins = [
            "PluginPathTest",
            "dependent_plugin_1",
            "dependent_plugin_2",
        ]
        self.assertEqual(sorted_plugins, expected_sorted_plugins)

    def test_sort_by_dependency_move_plugin(self):
        plugins = ["MSP", "P1", "LPA", "P2", "LA", "A", "MB", "P3", "LAA", "P4"]
        deps = {"A": ["MB", "MSP"], "LPA": ["A"], "LA": ["A"], "LAA": ["A"]}

        sorted_plugins = plugins.copy()
        visited = []

        for plugin in plugins:
            utils._move_plugin(plugin, deps, visited, sorted_plugins)

        expected_sorted_plugins = [
            "MSP",
            "P1",
            "P2",
            "MB",
            "A",
            "LA",
            "LPA",
            "P3",
            "LAA",
            "P4",
        ]

        self.assertEqual(sorted_plugins, expected_sorted_plugins)


if __name__ == "__main__":
    unittest.main()
