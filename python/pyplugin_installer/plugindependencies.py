# coding=utf-8
"""Parse plugin metadata for plugin_dependencies

.. note:: This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

"""

__author__ = 'elpaso@itopen.it'
__date__ = '2018-05-29'
__copyright__ = 'Copyright 2018, GISCE-TI S.L.'

from configparser import NoOptionError, NoSectionError
from .version_compare import compareVersions
from . import installer as plugin_installer
from qgis.utils import updateAvailablePlugins, metadataParser, get_plugin_deps


def __plugin_name_map(plugin_data_values):
    return {
        plugin['name']: plugin['id']
        for plugin in plugin_data_values
    }


def find_dependencies(plugin_id, plugin_data=None, plugin_deps=None, installed_plugins=None):
    """Finds the plugin dependencies and checks if they can be installed or upgraded

    :param plugin_id: plugin id
    :type plugin_id: str
    :param plugin_data: for testing only: dictionary of plugin data from the repo, defaults to None
    :param plugin_data: dict, optional
    :param plugin_deps: for testing only: dict of plugin id -> version_required, parsed from metadata value for "plugin_dependencies", defaults to None
    :param plugin_deps: dict, optional
    :param installed_plugins: for testing only: dict of plugin id -> version_installed
    :param installed_plugins: dict, optional
    :return: result dictionaries keyed by plugin name with: to_install, to_upgrade, not_found
    :rtype: tuple of dicts
    """

    to_install = {}
    to_upgrade = {}
    not_found = {}

    if plugin_deps is None or installed_plugins is None:
        updateAvailablePlugins()

    if plugin_deps is None:
        plugin_deps = get_plugin_deps(plugin_id)

    if installed_plugins is None:
        metadata_parser = metadataParser()
        installed_plugins = {metadata_parser[k].get('general', 'name'): metadata_parser[k].get('general', 'version') for k, v in metadata_parser.items()}

    if plugin_data is None:
        plugin_data = plugin_installer.plugins.all()

    plugins_map = __plugin_name_map(plugin_data.values())

    # Review all dependencies
    for name, version_required in plugin_deps.items():
        try:
            p_id = plugins_map[name]
        except KeyError:
            not_found.update({name: {
                'id': None,
                'version_installed': None,
                'version_required': None,
                'version_available': None,
                'use_stable_version': None,
                'action': None,
                'error': 'missing_id'
            }})
            continue

        affected_plugin = dict({
            "id": p_id,
            # "version_installed": installed_plugins.get(p_id, {}).get('installed_plugins', None),
            "version_installed": installed_plugins.get(name, None),
            "version_required": version_required,
            "version_available": plugin_data[p_id].get('version_available', None),
            "use_stable_version": True,  # Prefer stable by default
            "action": None,
        })
        version_available_stable = plugin_data[p_id].get('version_available_stable', None)
        version_available_experimental = plugin_data[p_id].get('version_available_experimental', None)

        if version_required is not None and version_required == version_available_stable:
            affected_plugin["version_available"] = version_available_stable
            affected_plugin["use_stable_version"] = True
        elif version_required is not None and version_required == version_available_experimental:
            affected_plugin["version_available"] = version_available_experimental
            affected_plugin["use_stable_version"] = False
        elif version_required is None:
            if version_available_stable:  # None if not found, "" if not offered
                # Prefer the stable version, if any
                affected_plugin["version_available"] = version_available_stable
                affected_plugin["use_stable_version"] = True
            else:  # The only available version is experimental
                affected_plugin["version_available"] = version_available_experimental
                affected_plugin["use_stable_version"] = False

        # Install is needed
        if name not in installed_plugins:
            affected_plugin['action'] = 'install'
            destination_list = to_install
        # Upgrade is needed
        elif version_required is not None and compareVersions(installed_plugins[name], version_required) == 2:
            affected_plugin['action'] = 'upgrade'
            destination_list = to_upgrade
        # TODO @elpaso: review installed but not activated
        # No action is needed
        else:
            continue

        if version_required == affected_plugin['version_available'] or version_required is None:
            destination_list.update({name: affected_plugin})
        else:
            affected_plugin['error'] = 'unavailable {}'.format(affected_plugin['action'])
            not_found.update({name: affected_plugin})

    return to_install, to_upgrade, not_found
