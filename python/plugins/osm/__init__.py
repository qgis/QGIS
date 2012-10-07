# -*- coding: utf-8 -*-

"""
***************************************************************************
    __init__.py
    ---------------------
    Date                 : July 2009
    Copyright            : (C) 2009 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Martin Dobias'
__date__ = 'July 2009'
__copyright__ = '(C) 2009, Martin Dobias'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

"""@package __init__

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


def category():
    """Function returns category of this plugin.

    @return category of this plugin.
    """

    return "Web"


def version():
    """Function returns version of this plugin.

    @return version of this plugin
    """
    from OsmPlugin import versionNumber
    return "Version "+versionNumber()


def qgisMinimumVersion():
    """Function returns information on what minimum version
    of Quantum GIS this plugin works with.

    @return minimum supported version of QGIS
    """

    return "1.0.0"

def icon():
    import resources_rc
    return ":/plugins/osm_plugin/images/osm_load.png"


def classFactory(iface):
    """Function returns OSM Plugin instance.

    @return instance of OSM Plugin
    """

    from OsmPlugin import OsmPlugin
    # return object of our plugin with reference to QGIS interface as the only argument
    return OsmPlugin(iface)

