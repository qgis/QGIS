"""@package __init__
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

This is the main module of OpenStreetMap plugin for Quantum GIS.
It initializes the plugin, making it known to QGIS.

OSM Plugin is viewer and editor for OpenStreetMap data.
"""


def name():
    """Function returns name of this plugin.

    @return name of this plugin ~ OpenStreetMap plugin
    """

    return "OpenStreetMap plugin"


def description():
    """Function returns brief description of this plugin.

    @return brief description of this plugin.
    """

    return "Viewer and editor for OpenStreetMap data"


def version():
    """Function returns version of this plugin.

    @return version of this plugin
    """

    return "Version 0.5"


def qgisMinimumVersion():
    """Function returns information on what minimum version
    of Quantum GIS this plugin works with.

    @return minimum supported version of QGIS
    """

    return "1.0.0"


def classFactory(iface):
    """Function returns OSM Plugin instance.

    @return instance of OSM Plugin
    """

    # load TestPlugin class from file testplug.py
    from OsmPlugin import OsmPlugin
    # return object of our plugin with reference to QGIS interface as the only argument
    return OsmPlugin(iface) 

