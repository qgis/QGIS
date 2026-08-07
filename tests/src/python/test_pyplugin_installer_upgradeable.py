"""QGIS Plugin upgrade test

.. note:: This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

"""

__author__ = "s.lopez@brgm.fr"
__date__ = "2025-07-16"
__copyright__ = "2026 BRGM"

import unittest

from qgis.core import (
    QgsSettingsEntryBool,
    QgsSettingsEntryStringList,
    QgsSettingsEntryVariant,
    QgsSettingsTree,
)
from qgis.testing import QgisTestCase, start_app

start_app()

# The Plugins class uses QgsSettings / QgsSettingsTree we must start qgis app first (preivous line)
# and prevent linter to move the following import (# noqa below)
from pyplugin_installer.installer_data import Plugins  # noqa: E402


def _ensure_plugin_manager_settings():
    try:
        node = QgsSettingsTree.node("plugin-manager")
    except Exception:
        node = None
    if node is None:
        node = QgsSettingsTree.treeRoot().createChildNode("plugin-manager")

    existing = {s.name() for s in node.childrenSettings()}
    entries = []
    for name, cls, default in [
        ("automatically-check-for-updates", QgsSettingsEntryBool, True),
        ("allow-experimental", QgsSettingsEntryBool, False),
        ("allow-deprecated", QgsSettingsEntryBool, False),
        ("check-on-start-last-date", QgsSettingsEntryVariant, None),
        ("seen-plugins", QgsSettingsEntryStringList, []),
    ]:
        if name not in existing:
            entries.append(cls(name, node, default))
    return node, entries


# Python must not release these references so that they are available during the test
_PM_NODE, _PM_ENTRIES = _ensure_plugin_manager_settings()


def _make_plugin(name, repo="Repo A", download_url=None, **kwargs):
    """Build plugin information as provided by distant repository."""
    plugin = {
        "id": name,
        "name": "42",
        "name": name.title(),
        "version_available": "1.1",
        "version_available_stable": "1.1",
        "version_available_experimental": "",
        "description": "desc",
        "about": "",
        "author_name": "",
        "homepage": "",
        "download_url": download_url or f"https://example.org/{name}.zip",
        "download_url_stable": download_url or f"https://example.org/{name}.zip",
        "download_url_experimental": "",
        "category": "",
        "tags": "",
        "changelog": "",
        "author_email": "",
        "tracker": "",
        "code_repository": "",
        "downloads": "0",
        "average_vote": "0",
        "rating_votes": "0",
        "create_date": "",
        "update_date": "",
        "create_date_stable": "",
        "update_date_stable": "",
        "create_date_experimental": "",
        "update_date_experimental": "",
        "icon": "",
        "experimental": False,
        "deprecated": False,
        "trusted": True,
        "filename": f"{name}.zip",
        "installed": False,
        "available": True,
        "status": "not installed",
        "status_exp": "not installed",
        "error": "",
        "error_details": "",
        "version_installed": "",
        "zip_repository": repo,
        "library": "",
        "readonly": False,
        "plugin_dependencies": "",
    }
    plugin.update(**kwargs)
    return plugin


class TestPlugins(QgisTestCase):
    def setUp(self):
        """Run before each test."""
        self.plugins = Plugins()

    def tearDown(self):
        """Run after each test."""
        pass

    def test_rebuild_marks_upgradeable(self):
        plugin = _make_plugin(
            "foo",
            installed=True,
            version_installed="1.0",
            version_available="",
            version_available_stable="",
            download_url="/local/path",
            status="orphan",
            status_exp="orphan",
        )
        self.plugins.localCache[plugin["id"]] = plugin
        self.plugins.addFromRepository(
            _make_plugin("foo", version_available="2.0", version_available_stable="2.0")
        )
        self.plugins.addFromRepository(
            _make_plugin("foo", version_available="1.0", version_available_stable="1.0")
        )

        self.plugins.rebuild()

        self.assertIn("foo", self.plugins.all())
        result = self.plugins.all()["foo"]
        self.assertTrue(result["installed"])
        self.assertTrue(result["available"])
        self.assertEqual(result["status"], "upgradeable")


if __name__ == "__main__":
    unittest.main()
