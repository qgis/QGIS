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
from qgis.utils import updateAvailablePlugins, plugins_metadata_parser


def __plugin_name_map(plugin_data_values):
    return {
        plugin['name']: plugin['id']
        for plugin in plugin_data_values
    }


def __get_plugin_deps(plugin_id):
    result = {}
    updateAvailablePlugins()
    try:
        global plugins_metadata_parser
        parser = plugins_metadata_parser[plugin_id]
        plugin_deps = parser.get('general', 'plugin_dependencies')
    except (NoOptionError, NoSectionError, KeyError):
        return result

    for dep in plugin_deps.split(','):
        if dep.find('==') > 0:
            name, version_required = dep.split('==')
        else:
            name = dep
            version_required = None
        result[name] = version_required
    return result


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

    if plugin_deps is None:
        plugin_deps = __get_plugin_deps(plugin_id)

    if installed_plugins is None:
        global plugins_metadata_parser
        updateAvailablePlugins()
        metadata_parser = plugins_metadata_parser
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
            "action": None,
        })

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

        if affected_plugin['version_required'] == affected_plugin['version_available'] or affected_plugin['version_required'] is None:
            destination_list.update({name: affected_plugin})
        else:
            affected_plugin['error'] = 'unavailable {}'.format(affected_plugin['action'])
            not_found.update({name: affected_plugin})

    return to_install, to_upgrade, not_found
