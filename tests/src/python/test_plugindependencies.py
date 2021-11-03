# coding=utf-8
"""QGIS Plugin dependencies test

.. note:: This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2018-09-19'
__copyright__ = 'Copyright 2018, GISCE-TI S.L.'

import uuid
import os
import re
import json
import unittest
from qgis.PyQt.QtCore import QCoreApplication
from pyplugin_installer.plugindependencies import find_dependencies

from qgis.testing import (
    start_app,
    unittest,
)

from utilities import unitTestDataPath

TESTDATA_PATH = unitTestDataPath()


class PluginDependenciesTest(unittest.TestCase):
    """Test plugin dependencies"""

    @classmethod
    def setUpClass(cls):
        """Runs at start."""

        QCoreApplication.setOrganizationName("QGIS")
        QCoreApplication.setOrganizationDomain("qgis.org")
        QCoreApplication.setApplicationName("QGIS-TEST-%s" % uuid.uuid1())
        qgis_app = start_app()

        # Installed plugins
        cls.installed_plugins = {
            'MetaSearch': '0.3.5',
            'QuickWKT': '3.1',
            'db_manager': '0.1.20',
            'firstaid': '2.1.1',
            'InaSAFE': '5.0.0',
            'ipyconsole': '1.8',
            'plugin_reloader': '0.7.4',
            'processing': '2.12.99',
            'qgis-geocoding': '2.18',
            'qgisce': '0.9',
            'redistrict': '0.1'
        }

        data_path = os.path.join(TESTDATA_PATH, 'plugindependencies_data.json')
        with open(data_path) as f:
            cls.plugin_data = json.loads(f.read())

    def setUp(self):
        """Runs before each test."""
        pass

    def tearDown(self):
        """Runs after each test."""
        pass

    def test_find_dependencies(self):

        to_install, to_upgrade, not_found = find_dependencies('qgisce',
                                                              self.plugin_data,
                                                              plugin_deps={'InaSAFE': None},
                                                              installed_plugins=self.installed_plugins)
        self.assertEqual(to_install, {})
        self.assertEqual(to_upgrade, {})
        self.assertEqual(not_found, {})

        to_install, to_upgrade, not_found = find_dependencies('qgisce',
                                                              self.plugin_data,
                                                              plugin_deps={'InaSAFE': '110.1'},
                                                              installed_plugins=self.installed_plugins)
        self.assertEqual(to_install, {})
        self.assertEqual(to_upgrade, {})
        self.assertEqual(not_found['InaSAFE']['version_installed'], '5.0.0')

        # QuickWkt is installed, version is not specified: ignore
        installed_plugins = self.installed_plugins
        installed_plugins['QuickWKT'] = '2.1'
        to_install, to_upgrade, not_found = find_dependencies('qgisce',
                                                              self.plugin_data,
                                                              plugin_deps={'QuickMapServices': '0.19.10.1',
                                                                           'QuickWKT': None},
                                                              installed_plugins=self.installed_plugins)
        self.assertEqual(to_install['QuickMapServices']['version_required'], '0.19.10.1')
        self.assertEqual(to_install['QuickMapServices']['version_available'], '0.19.10.1')
        self.assertEqual(to_install['QuickMapServices']['use_stable_version'], True)
        self.assertEqual(to_upgrade, {})
        self.assertEqual(not_found, {})

        # QuickWkt is installed, version requires upgrade and it's in the repo: upgrade
        to_install, to_upgrade, not_found = find_dependencies('qgisce',
                                                              self.plugin_data,
                                                              plugin_deps={'QuickWKT': '3.1'},
                                                              installed_plugins=installed_plugins)
        self.assertEqual(to_install, {})
        self.assertEqual(to_upgrade['QuickWKT']['version_required'], '3.1')
        self.assertEqual(to_upgrade['QuickWKT']['version_available'], '3.1')
        self.assertEqual(to_upgrade['QuickWKT']['use_stable_version'], True)
        self.assertEqual(not_found, {})

        # QuickWkt is installed, version requires upgrade and it's NOT in the repo: not found
        to_install, to_upgrade, not_found = find_dependencies('qgisce',
                                                              self.plugin_data,
                                                              plugin_deps={'QuickWKT': '300.11234'},
                                                              installed_plugins=installed_plugins)
        self.assertEqual(to_install, {})
        self.assertEqual(to_upgrade, {})
        self.assertEqual(not_found['QuickWKT']['version_required'], '300.11234')
        self.assertEqual(not_found['QuickWKT']['version_installed'], '2.1')
        self.assertEqual(not_found['QuickWKT']['version_available'], '3.1')

        # Installed version is > than required: ignore (no downgrade is currently possible)
        installed_plugins['QuickWKT'] = '300.1'
        to_install, to_upgrade, not_found = find_dependencies('qgisce',
                                                              self.plugin_data,
                                                              plugin_deps={'QuickWKT': '1.2'},
                                                              installed_plugins=installed_plugins)
        self.assertEqual(to_install, {})
        self.assertEqual(to_upgrade, {})
        self.assertEqual(not_found, {})

        # A plugin offers both stable and experimental versions. A dependent plugin requires the experimental one.
        to_install, to_upgrade, not_found = find_dependencies('LADM-COL-Add-on-Ambiente',
                                                              self.plugin_data,
                                                              plugin_deps={'Asistente LADM-COL': '3.2.0-beta-1'},
                                                              installed_plugins=self.installed_plugins)
        self.assertEqual(to_install['Asistente LADM-COL']['version_required'], '3.2.0-beta-1')
        self.assertEqual(to_install['Asistente LADM-COL']['version_available'], '3.2.0-beta-1')
        self.assertEqual(to_install['Asistente LADM-COL']['use_stable_version'], False)
        self.assertEqual(to_upgrade, {})
        self.assertEqual(not_found, {})

        # A plugin offers both stable and experimental versions. A dependent plugin requires the stable one.
        to_install, to_upgrade, not_found = find_dependencies('LADM-COL-Add-on-Ambiente',
                                                              self.plugin_data,
                                                              plugin_deps={'Asistente LADM-COL': '3.1.9'},
                                                              installed_plugins=self.installed_plugins)
        self.assertEqual(to_install['Asistente LADM-COL']['version_required'], '3.1.9')
        self.assertEqual(to_install['Asistente LADM-COL']['version_available'], '3.1.9')
        self.assertEqual(to_install['Asistente LADM-COL']['use_stable_version'], True)
        self.assertEqual(to_upgrade, {})
        self.assertEqual(not_found, {})

        # A plugin offers both stable and experimental versions. If no version is required, choose the stable one.
        to_install, to_upgrade, not_found = find_dependencies('LADM-COL-Add-on-Ambiente',
                                                              self.plugin_data,
                                                              plugin_deps={'Asistente LADM-COL': None},
                                                              installed_plugins=self.installed_plugins)
        self.assertEqual(to_install['Asistente LADM-COL']['version_required'], None)
        self.assertEqual(to_install['Asistente LADM-COL']['version_available'], '3.1.9')
        self.assertEqual(to_install['Asistente LADM-COL']['use_stable_version'], True)
        self.assertEqual(to_upgrade, {})
        self.assertEqual(not_found, {})

        # A plugin only offers experimental version. If the experimental version is required, give it to him.
        to_install, to_upgrade, not_found = find_dependencies('dependent-on-unique_values_viewer',
                                                              self.plugin_data,
                                                              plugin_deps={'UniqueValuesViewer': '0.2'},
                                                              installed_plugins=self.installed_plugins)
        self.assertEqual(to_install['UniqueValuesViewer']['version_required'], '0.2')
        self.assertEqual(to_install['UniqueValuesViewer']['version_available'], '0.2')
        self.assertEqual(to_install['UniqueValuesViewer']['use_stable_version'], False)
        self.assertEqual(to_upgrade, {})
        self.assertEqual(not_found, {})

        # A plugin only offers experimental version. If no version is required, choose the experimental one.
        to_install, to_upgrade, not_found = find_dependencies('dependent-on-unique_values_viewer',
                                                              self.plugin_data,
                                                              plugin_deps={'UniqueValuesViewer': None},
                                                              installed_plugins=self.installed_plugins)
        self.assertEqual(to_install['UniqueValuesViewer']['version_required'], None)
        self.assertEqual(to_install['UniqueValuesViewer']['version_available'], '0.2')
        self.assertEqual(to_install['UniqueValuesViewer']['use_stable_version'], False)
        self.assertEqual(to_upgrade, {})
        self.assertEqual(not_found, {})


def pluginSuite():
    return unittest.makeSuite(PluginDependenciesTest, 'test')


if __name__ == "__main__":
    suite = unittest.makeSuite(PluginDependenciesTest)
    runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
